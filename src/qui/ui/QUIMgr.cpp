#include "stdafx.h"
#include "QUIMgr.h"
#include "AppHelper.h"
#include "QView.h"
#include "QSkinUI.h"
#include "QUIFunction.h"
#include "WndHelper.h"

using namespace htmlayout;

HTMLayoutResourceType QUIGetResourceType(__in const CStdString& uri)
{
    int i = uri.ReverseFind(L'.');
    if (-1 == i)
        return HLRT_DATA_IMAGE;
    
    CStdString ext = uri.Mid(i+1);
    if (!ext.CompareNoCase(L"css"))
        return HLRT_DATA_STYLE;
    else if (!ext.CompareNoCase(L"cur"))
        return HLRT_DATA_CURSOR;
    else if (!ext.CompareNoCase(L"htm") || !ext.CompareNoCase(L"html"))
        return HLRT_DATA_HTML;

    return HLRT_DATA_IMAGE;
}

/** 查询特殊控件的指针
 *	return:
 *      控件指针值，需要自己根据需要进行类型转换
 *	params:
 *		-[in]
 *          heCtl       控件包裹dom值
 *          eWidgetType 控件类型
**/
LPVOID QUIQuerySpecialCtrl(__in HELEMENT ctl, __in UINT widget_type)
{
    QView *view = QUIViewFromHELEMENT(ctl);
    if (NULL != view)
    {
        return view->QuerySpecialCtrl(ctl, (ENUM_WIDGET_TYPE)widget_type);
    }
    return NULL;
}

BOOL QUISplitResPath(__in LPCWSTR szURI, __out CStdString& sIndicator, __out CStdString& sPath)
{
    LPCWSTR pInd = wcschr(szURI, L':');
    if (nullptr != pInd)
    {
        // 后面滴
        sPath = pInd + 1;
        // 前面滴
        sIndicator = szURI;
        sIndicator = sIndicator.Left(pInd - szURI + 1);

        return TRUE;
    }
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////
std::vector<QView*> QUIMgr::m_lstWnd;
std::map<HELEMENT, HWND> QUIMgr::m_mapHwndDOM;
_Skinbag* QUIMgr::m_pCurSkin = nullptr;
QUIFunction* QUIMgr::m_pFunction = nullptr;

BOOL QUIMgr::Startup( __in const wchar_t* ui_zip/* = L"UI.zip"*/ )
{
    if (IsStartup())
        return TRUE;

    if ( !res_.LoadFromZip(ui_zip) )
        return FALSE;

    // 加载master CSS
    const wchar_t* css_master = L"common/master.css";
    QBuffer* p = res_.GetResource(css_master);
    if ((nullptr == p)
        || !HTMLayoutSetMasterCSS(p->GetBuffer(),p->GetBufferLen()))
    {
        return FALSE;
    }
    // 以后不会再用到master css了。删除之
    res_.ReleaseResource(css_master);
    return TRUE;
}

void QUIMgr::Shutdown() 
{
    // 关闭所有窗口
    QView *p = nullptr;
    int n = 0;
    while ( !m_lstWnd.empty() )
    {
        n = m_lstWnd.size();
        p = m_lstWnd.back();    // 从后面开始
        if ((nullptr == p) || !(p->IsWindow()))
        {
            m_lstWnd.pop_back();
            continue;
        }

        // 向所有的popup窗口发关闭消息
        if ( p->GetStyle() & WS_POPUP )
        {
            p->SendMessage(WM_CLOSE);
        }

        // 没变化啊！
        if (m_lstWnd.size() == n)
        {
            break;
        }
    }

}

QUIMgr::ENUM_UI_RESPATH QUIMgr::DetectDataPathType( __in const CStdString& sPath )
{
    if (sPath.IsEmpty())
        return UI_RESPATH_ERROR;

    int idx = sPath.Find(L':',0);
    if ( -1 == idx)
        return UI_RESPATH_UNKOWN;

    if (StrCmpNIW(sPath, QUI_UIPAK_ABSINDICATOR, idx) == 0)
		return UI_RESPATH_PKGABS;
    else if (StrCmpNIW(sPath, QUI_UIPAK_RELATEINDICATOR, idx) == 0)
		return UI_RESPATH_PKGRELATE;

	return UI_RESPATH_UNKOWN;
}

QUIMgr::ENUM_UI_RESPATH QUIMgr::ParseDataPath(__inout CStdString& sPath,__in HELEMENT hPrincipal)
{
	ENUM_UI_RESPATH nType = DetectDataPathType(sPath);
	if ((UI_RESPATH_PKGRELATE == nType) && (hPrincipal != NULL) )
	{ // 资源存在于资源包内，路径相对于当前窗口加载的html路径
		QView *pView = QUIMgr::QWindowFromHELEMENT(hPrincipal);
		if (NULL != pView)
		{
			ConvertToAbsPackagePath(pView->GetHtmlPath(),sPath);
            QUITrimPrefix(sPath);
		}
	}
    else if (UI_RESPATH_PKGABS == nType)
    {
        QUITrimPrefix(sPath);
        sPath.Replace(L'\\',L'/');
    }
	return nType;
}

QUIMgr::ENUM_UI_RESPATH QUIMgr::ParseDataPath(__inout CStdString& sPath,__in QView *pView)
{
	ENUM_UI_RESPATH nType = DetectDataPathType(sPath);
	if ((UI_RESPATH_PKGRELATE == nType) && (pView != NULL) )
	{ // 资源存在于资源包内，路径相对于当前窗口加载的html路径
		ConvertToAbsPackagePath(pView->GetHtmlPath(),sPath);
        QUITrimPrefix(sPath);
	}
    else if (UI_RESPATH_PKGABS == nType)
    {
        QUITrimPrefix(sPath);
        sPath.Replace(L'\\',L'/');
    }
	return nType;
}

// qrel:a.txt + qabs:html/b.html = qabs:html/a.txt
// qrel:a.txt + qabs:b.html = qabs:a.txt;
// 使用<include src="">的标签src必须使用qabs引用资源的全路径
BOOL QUIMgr::ConvertToAbsPackagePath( const CStdString& sAbsPath,__inout CStdString& sRelatePath )
{
    ENUM_UI_RESPATH eType = DetectDataPathType(sRelatePath);
    if (UI_RESPATH_PKGABS == eType)
        return TRUE;
    if (   (UI_RESPATH_PKGRELATE != eType)  // 要转换的路径不是包内相对路径
        || (UI_RESPATH_PKGABS != DetectDataPathType(sAbsPath))) // 相对的路径不是绝对路径
    {
        ATLASSERT(FALSE);
        return FALSE;
    }

    // 去掉最后一级（文件级别）
    CStdString sTempAbs = sAbsPath;
    sTempAbs.Replace(L'\\',L'/');
    sTempAbs = sTempAbs.Left(sTempAbs.find_last_of(L"/:") + 1);
    // 去掉前缀（qrel:）
    sRelatePath = sRelatePath.Mid(sRelatePath.Find(L':') + 1);
    sRelatePath.Replace(L'\\',L'/');
    do 
    {
        // 解析相对目录符号（../）
        int i = sRelatePath.Find(L"../");
        int j = sTempAbs.find_last_of(L"/:");
        if ((-1 != i) && (-1 == j)) 
            return FALSE;
        if ((-1 == i) || (-1 == j))
            break;
        sRelatePath = sRelatePath.Mid(3);	// 去掉 ../
        sTempAbs = sTempAbs.Left(j + 1);	// 向上一级，留下 ‘/’ 或者 ':'
    } while ( true );
    sRelatePath = sTempAbs + sRelatePath;
    return TRUE;
}

BOOL QUIMgr::_LoadData(LPCWSTR szResPath, LPCWSTR szOriginPath ,
    ENUM_UI_RESPATH nType, __out QBuffer* &pBuf)
{
    pBuf = nullptr;
    switch (nType)
    {
    case UI_RESPATH_PKGRELATE:
    case UI_RESPATH_PKGABS:
        {
            pBuf = res_.GetResource(szResPath);
            break;
        }
    case UI_RESPATH_UNKOWN:
        {
            if (nullptr != m_pFunction)
            {
                m_pFunction->LoadResData(szOriginPath, NULL, pBuf);
            }
            break;
        }
    }

#ifdef _DEBUG
    if (nullptr == pBuf)
    {
        //TRACE(L"资源加载失败：%s\n",(LPCWSTR)szOriginPath);
        return FALSE;
    }
#endif

    // FALSE: 数据需要交给底层处理
    return NULL != pBuf;
}

BOOL QUIMgr::LoadData(LPCWSTR szResPath,__in QView* pView,__out QBuffer* &pBuf)
{
	CStdString sPath = szResPath;
	ENUM_UI_RESPATH nType = ParseDataPath(sPath,pView);
    
    return _LoadData(sPath, szResPath, nType, pBuf);
}

BOOL QUIMgr::LoadData(LPCWSTR szResPath,__in HELEMENT hPrincipal,__out QBuffer* &pBuf)
{
    CStdString sPath = szResPath;
    ENUM_UI_RESPATH nType = ParseDataPath(sPath,hPrincipal);

    return _LoadData(sPath, szResPath, nType, pBuf);
}

LRESULT QUIMgr::OnHtmlayoutLoadData( __inout LPNMHL_LOAD_DATA pns)
{
	QBuffer* pBuf = nullptr;
	if (!LoadData(pns->uri,pns->principal,pBuf))
	{
		return LOAD_DISCARD;
	}
	// call #SciterDataReady() function instead of filling these fields. 
	// This allows you to free your outData buffer immediately.
	// 		pns->outData = m_bufData.GetBuffer(0);
	// 		pns->outDataSize = m_bufData.GetBufferLen();
//	if (HTMLayoutDataReady(pns->hdr.hwndFrom,pns->uri,pBuf->GetBuffer(),pBuf->GetBufferLen()))
//	return LOAD_DISCARD;
  
    pns->outData = pBuf->GetBuffer();
    pns->outDataSize = pBuf->GetBufferLen();

    return LOAD_OK;
}

void QUIMgr::OnWindowCreate( __in QView *pView )
{
    m_lstWnd.push_back(pView);

//     if (nullptr != m_pCurSkin)
//     {
//         pView->ApplySkinbag(m_pCurSkin);
//     }
}

void QUIMgr::OnWindowDestroy( QView* p )
{
	// 	if (NULL == p)
	// 		return;
	// 	HWND hWnd = p->GetSafeHwnd();
	// 	QCtrlDraw::EndDynamicDraw(hWnd);

    auto i = std::find(m_lstWnd.begin(), m_lstWnd.end(), p);
    if (m_lstWnd.end() != i)
    {
        m_lstWnd.erase(i);
    }
}

QView* QUIMgr::QWindowFromHWND( __in HWND hWnd )
{
    if (NULL != hWnd)
    {
        auto iEnd = m_lstWnd.end();
        for (auto i = m_lstWnd.begin(); i != iEnd; ++i)
        {
            if ((*i)->GetSafeHwnd() == hWnd)
            {
                return *i;
            }
        }
    }
	return nullptr;
}

QView* QUIMgr::QWindowFromHELEMENT( __in HELEMENT he )
{
	dom::element el(he);
	if (el.is_valid())
	{
		return QWindowFromHWND(el.get_element_hwnd(true));
	}
	return NULL;
}

HWND QUIMgr::HwndOfWidgetDOM( __in HELEMENT he )
{
    ATLASSERT(NULL != he);

    auto i = m_mapHwndDOM.find(he);
    if (i != m_mapHwndDOM.end())
    {
        return i->second;
    }

#ifdef _DEBUG
    json::astring htm = ECtrl(he).get_html(1);
#endif
    
    return NULL;
}

HELEMENT QUIMgr::DOMOfWidgetHwnd( __in HWND hWnd )
{
    auto iEnd = m_mapHwndDOM.end();
    for (auto i = m_mapHwndDOM.begin(); i != iEnd; ++i)
    {
        if (i->second == hWnd)
        {
            return i->first;
        }
    }
    return NULL;
}

BOOL QUIMgr::OnWidgetWindowCreate( __in HELEMENT hWidget, __in HWND hWnd )
{
#ifdef _DEBUG
    QView* vt =  QWindowFromHWND(hWnd);
    json::astring htmt = ECtrl(hWidget).get_html(1);

    HELEMENT hOWdgt = DOMOfWidgetHwnd(hWnd);
    if (NULL != hOWdgt)
    {
        json::astring htm = ECtrl(hOWdgt).get_html(1);

        QView *v1 = QWindowFromHWND(HwndOfWidgetDOM(hOWdgt));

        ATLASSERT(FALSE);
        return FALSE;
    }

    HWND hOldWnd = HwndOfWidgetDOM(hWidget);
    if (NULL != hOldWnd)
    {
        QView *v2 = QWindowFromHWND(hOldWnd);

        ATLASSERT(FALSE);
        return FALSE;
    }
#else
    if ((NULL != DOMOfWidgetHwnd(hWnd)) || (NULL != HwndOfWidgetDOM(hWidget)))
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
#endif

    m_mapHwndDOM[hWidget] = hWnd;

    return TRUE;
}

void QUIMgr::OnWidgetWindowDestroy( __in HELEMENT hWidget, __in HWND hWnd )
{
    m_mapHwndDOM.erase(hWidget);

    auto iEnd = m_mapHwndDOM.end();
    for (auto i = m_mapHwndDOM.begin(); i != iEnd; ++i)
    {
        if (i->second == hWnd)
        {
            m_mapHwndDOM.erase(i);
            break;
        }
    }
}

BOOL QUIMgr::LoadSkinbag(__in _Skinbag* pBag)
{
    if (nullptr != m_pFunction)
    {
        return m_pFunction->LoadSkinbag(pBag);
    }
    return FALSE;
}

BOOL QUIMgr::SkinbagComplete(__inout _Skinbag* p)
{
    if (nullptr != m_pFunction)
    {
        return m_pFunction->SkinbagComplete(p);
    }
    return FALSE;
}

BOOL QUIMgr::PrepareSkinbag(__in _Skinbag* pBag)
{
    if (nullptr != m_pFunction)
    {
        return m_pFunction->PrepareSkinbag(pBag);
    }
    return FALSE;
}

BOOL QUIMgr::ApplyStylebag( __in _Skinbag* pBag )
{
    if (nullptr != pBag)
    {
        // 确保数据包是完整的，
        if (!pBag->IsComplete())
        {
            ATLASSERT(FALSE);
            return FALSE;
        }

        if (nullptr != m_pFunction)
        {
            // 确保被加载到了资源管理器
            if (!m_pFunction->LoadSkinbag(pBag))
            {
                return FALSE;
            }
        }

        auto iEnd = m_lstWnd.end();
        for (auto i = m_lstWnd.begin();  i != iEnd; ++i)
        {
            if (nullptr != m_pFunction)
            {
                if (m_pFunction->OnApplySkinbag(*i, pBag))
                {
                    (*i)->ApplySkinbag(pBag);

                    m_pFunction->ApplySkinbagComplete(*i, pBag);
                }
            }
            else
            {
                (*i)->ApplySkinbag(pBag);
            }
        }
    }

    // 记录当前使用的包
    m_pCurSkin = pBag;
    return TRUE;
}

void QUIMgr::ApplyOpacity(__in int nAlpha) 
{
    // 除了skindlg，其它窗口都透明
    QSkinUI *pSkinDlg = QSkinUI::GetInstance();

    QView *p;
    auto iEnd = m_lstWnd.end();
    for (auto i = m_lstWnd.begin();  i != iEnd; ++i)
    {
        p = *i;

        if (nullptr != m_pFunction)
        {
            if (m_pFunction->OnApplyOpacity(p, nAlpha))
            {
                quibase::TransparentWindow(p->GetSafeHwnd(), nAlpha);
            }
        }
        else if ((p != pSkinDlg) && (p->GetStyle() & WS_POPUP))
        {
            quibase::TransparentWindow(p->GetSafeHwnd(), nAlpha);
        }
    }
}

struct _EnumRes
{
    LPCWSTR     uri_;
    BOOL        found_;
};

BOOL CALLBACK OnEnumRes(LPCWSTR resourceUri, LPCSTR resourceType, 
    LPCBYTE imageData, DWORD imageDataSize, LPVOID prm)
{
    _EnumRes *p = reinterpret_cast<_EnumRes*>(prm);
    if (wcsicmp(resourceUri, p->uri_) == 0)
    {
        p->found_ = TRUE;
        // 停止枚举
        return FALSE;
    }
    return TRUE;
}

void QUIMgr::UpdateResDataWith(LPCWSTR uri, LPBYTE pData, int nDataLen, BOOL bIfExist)
{
    auto *vs = QUIMgr::GetInstance()->GetViews();
    auto ie_view = vs->end();
    for (auto j = vs->begin(); j != ie_view; ++j) 
    {
        // 是否能更新？
        if ((nullptr != m_pFunction) && (!m_pFunction->OnUpdateResData(*j, uri)))
            continue;

        if (bIfExist)
        {
            _EnumRes er;
            er.found_ = FALSE;
            er.uri_ = uri;
//            HTMLayoutEnumResourcesEx((*j)->GetSafeHwnd(),OnEnumRes, &er);
//             if (!er.found_)
//             {
//                 continue;
//             }
        }

        // 更新之
        ATLVERIFY(HTMLayoutDataReadyAsync((*j)->GetSafeHwnd(), uri, 
            pData, nDataLen, QUIGetResourceType(uri)));
    }
}

BOOL QUIMgr::_UIData::DecodeResource( __in const CStdString &sResName, __inout QBuffer* pData )
{
#ifndef _DEBUG  // release
    return ende_.DecryptData(*pData);
#endif
    return TRUE;
}

BOOL QUIMgr::_UIData::OnLoadFromZip( CXUnzip *zip )
{
#ifndef _DEBUG
    // 为方便调试，只有在Release版本下才进行UI数据加密
    QBuffer buf;
    if (!zip->UnzipToBuffer(L"CODE", buf))
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
    return ende_.SetCodeData(buf);
#endif
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// BOOL QHtmlMgr::Startup()
// {
// 	if (IsStartup())
// 		return TRUE;
// 
// 	if (NULL == QUIGetConfig())
// 		return FALSE;
// 	m_bStarted = m_cIni.Load(QUIGetConfig()->GetLanguage());
// 	return m_bStarted;
// }
// 
// QString QHtmlMgr::GetLaugString( LPCWSTR szSec,LPCWSTR szKey )
// {
// 	if (IsStartup())
// 	{
// 		return m_cIni.GetKeyValue(szSec,szKey);
// 	}
// 	return L"";
// }
// 
// BOOL QHtmlMgr::ParseVar(__in const QString& str,
// 	__out QString& sSection,__out QString &sKey)
// {
// 	if (!IsStartup())
// 		return FALSE;
// 
// 	int idx = str.Find(L'.');
// 	if (-1 == idx)
// 		return FALSE;
// 	sSection = str.Left(idx);
// 	sKey = str.Mid(idx+1);
// 	return TRUE;
// }
// 
// BOOL QHtmlMgr::TranslateHtmlLanguageVar( __inout QString& sHtml )
// {
// 	if (!IsStartup())
// 		return FALSE;
// 
// 	// trans_lang_var 翻译指示器
// 	if (sHtml.Left(20).CompareNoCase(L"<html trans_lang_var") != 0)
// 		return TRUE;
// 
// 	// 正则表达式太慢啦！
// 	// 咱还是一点点的解析吧
// 	QString sVar ,sRet,sSec,sKey;
// 	int idxb = 0 ,idxe = 0;
// 	while ((idxb = sHtml.Find(CHAR_HTMLVAR_SEP,0)) != -1)
// 	{
// 		sRet += sHtml.Left(idxb);
// 		sHtml = sHtml.Mid(idxb);
// 		idxe = sHtml.Find(CHAR_HTMLVAR_SEP,1);
// 		if (-1 == idxe)
// 			break;
// 		sVar = sHtml.Mid(1,idxe-1);
// 		if (!ParseVar(sVar,sSec,sKey))
// 		{
// 			ASSERT(FALSE);
// 			sRet += CHAR_HTMLVAR_SEP;
// 			sRet += sVar;
// 			sRet += CHAR_HTMLVAR_SEP;
// 			sHtml = sHtml.Mid(idxe + 1);
// 			continue;
// 		}
// 		sRet += GetLaugString(sSec,sKey);
// 		sHtml = sHtml.Mid(idxe + 1);
// 	}
// 	if (!sRet.IsEmpty())
// 	{ // 如果sRet为空的话，那就是根本没有字符串需要解析出来
// 		sRet += sHtml;
// 		sHtml = sRet;
// 	}
// 	return TRUE;
// }

//////////////////////////////////////////////////////////////////////////
BOOL QUIFunction::OnApplyOpacity(__in QView* pView, __in int nAlpha) 
{
    return QSkinUI::GetInstance() != pView;
}

BOOL QUIFunction::OnUpdateResData(__in QView* pView, __in LPCWSTR uri) 
{
    return QSkinUI::GetInstance() != pView;
}

//////////////////////////////////////////////////////////////////////////
QUIResource::~QUIResource() 
{
    ReleaseAll();
}

BOOL QUIResource::LoadZip(__in CXUnzip* zip)
{
    if ( !zip || !zip->IsOpen() || !OnLoadFromZip(zip))
        return FALSE;

    ZIPENTRYW ze;
    QBuffer *pData = NULL;
    //////////////////////////////////////////////////////////////////////////
    // 不要为了节省运行时内存，而等到实际需要数据的时候再加载数据
    // 因为那样的话，数据的分布将会非常零散（在内存中的布局）
    // 这将极可能大大拖慢查找速度
    // 参考：http://www.codeproject.com/Articles/340797/Number-crunching-Why-you-should-never-ever-EVER-us
    // 解压所有的数据到内存中
    for (int i = 0; i < zip->GetItemCount(); ++i)
    {
        if (!zip->GetItemInfo(i,&ze))
            continue;

        pData = new QBuffer;
        if ( !zip->UnzipToBuffer(i,*pData) 
            || !DecodeResource(ze.name, pData))  
        {
            delete pData;
            continue;
        }

        m_mapData[ze.name] = pData;
    }
    return TRUE;
}

// BOOL QUIResource::LoadFromZipBuffer(__in QBuffer& buf)
// {
//     if ( GetResourceCount() > 0)
//     {
//         ASSERT(FALSE);
//         return FALSE;
//     }
// 
//     BOOL bOK = FALSE;
//     CXUnzip theZip;
//     if ( theZip.Open(buf) )
//     {
//         bOK = LoadZip(&theZip);
//         theZip.Close();
//     }
//     return bOK;
// }
// 
// BOOL QUIResource::LoadFromZip(__in LPCWSTR szZip) 
// {
//     if ( GetResourceCount() > 0)
//     {
//         ASSERT(FALSE);
//         return FALSE;
//     }
// 
//     BOOL bOK = FALSE;
//     CXUnzip theZip;
//     if ( theZip.Open(szZip) )
//     {
//         bOK = LoadZip(&theZip);
//         theZip.Close();
//     }
//     return bOK;
// }

BOOL QUIResource::AddedLoadFromZip(__in LPCWSTR szZip) 
{
    CXUnzip theZip;
    if ( !theZip.Open(szZip) )
        return FALSE;

    ZIPENTRYW ze;
    QBuffer *pData = NULL;
    for (int i = 0; i < theZip.GetItemCount(); ++i)
    {
        if (!theZip.GetItemInfo(i,&ze))
            continue;

        pData = new QBuffer;
        if ( !theZip.UnzipToBuffer(i,*pData) 
            || !DecodeResource(ze.name, pData))  
        {
            delete pData;
            continue;
        }

        auto j = _Find(ze.name);
        if (_End() != j)
        {
            delete j->second;
            j->second = pData;
        }
        else
        {
            m_mapData[ze.name] = pData;
        }
    }
    return TRUE;
}

QBuffer* QUIResource::GetResource(__in LPCWSTR szResName) 
{
    auto itr = m_mapData.find(szResName);
    if (m_mapData.end() != itr)
    {
        return itr->second;
    }
    return nullptr;
}

void QUIResource::ReleaseResource(__in LPCWSTR szResName, __in LPCWSTR szResIndicator) 
{
    auto ir = m_mapData.find(szResName);
    if (m_mapData.end() != ir)
    {
        // HL会有此资源的缓存。销毁之！
        OnReleaseResource(szResName, szResIndicator);

        delete ir->second;
        m_mapData.erase(ir);
    }
}

void QUIResource::ReleaseAll(__in LPCWSTR szResIndicator) 
{
    // 内存资源
    // HL会有此资源的缓存。销毁之！
    MapStr2DataItr ie_res = m_mapData.end();
    for (MapStr2DataItr i = m_mapData.begin(); i != ie_res; ++i)
    {
        OnReleaseResource(szResIndicator, i->first);

        delete i->second;
    }
    m_mapData.clear();
}

BOOL QUIResource::AddResource(__in LPCWSTR szResName, __in QBuffer* pRes) 
{
    ATLASSERT(nullptr != pRes);
    ATLASSERT(nullptr != szResName);

    if ( !IsExist(szResName) )
    {
        m_mapData[szResName] = pRes;
        return TRUE;
    }
    return FALSE;
}

BOOL QUIResource::UpdateResourceName(__in LPCWSTR szResName, __in LPCWSTR szNewName) 
{
    if (IsExist(szNewName))
        return FALSE;

    auto i = _Find(szResName);
    if (_End() != i)
    {
        auto p = i->second;
        m_mapData.erase(i);
        m_mapData[szNewName] = p;
        return TRUE;
    }
    return FALSE;
}

BOOL QUIResource::UpdateResource(__in LPCWSTR szResName, __in QBuffer* pNewRes) 
{
    auto i = _Find(szResName);
    if (_End() != i)
    {
        delete i->second;
        i->second = pNewRes;
        return TRUE;
    }
    return FALSE;
}

void QUIResource::OnReleaseResource(__in const CStdString& szResName, 
    __in const CStdString& szResIndicator)
{
    if (!szResIndicator.IsEmpty() && !szResName.IsEmpty())
    {
        QUIMgr::GetInstance()->UpdateResDataWithNULL(szResIndicator + szResName);
    }
}
