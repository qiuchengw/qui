#include "stdafx.h"
#include "QView.h"
#include "QBuffer.h"
#include "QUIMgr.h"
#include "QApp.h"
#include "AppHelper.h"
#include "BackHelper.h"
#include "QSkinUI.h"

#ifdef UIBASE_SUPPORT_WEBBROWSER
#   include "browser/WebBrowser.h"
#endif

using namespace htmlayout;

// 特殊控件（创建、销毁、查询）管理器
class _SpecialWidgets
{
    friend class QView;

    typedef struct _SPECIAL_WIDGET 
    {
        _SPECIAL_WIDGET(HELEMENT h,LPVOID p,ENUM_WIDGET_TYPE e)
            :heCtl(h),pCtl(p),eType(e){}
        HELEMENT            heCtl;
        LPVOID              pCtl;
        ENUM_WIDGET_TYPE    eType;
    }*LP_SPECIAL_WIDGET;
    typedef std::vector<LP_SPECIAL_WIDGET> ListWidgets;
    typedef ListWidgets::iterator ListWidgetsItr;

protected:
    ~_SpecialWidgets()
    {
        ATLASSERT(widgets_.size() == 0);   // 控件应该已经全部销毁了
    }

    /** 处理特殊控件的创建
        *	return:
        *      创建后的窗口控件，NULL 未能成功窗口
        *	params:
        *		-[in]
        *          heWrapper        控件的包裹元素
        *          hWndParent       控件所在父窗口
        *          eWidgetType      控件类型
        *      -[out]
        *          bHandled         创建是否被处理，TRUE 被处理
    **/
    HWND HandleCreation(__in HELEMENT heWrapper, __in HWND hWndParent, 
                __in ENUM_WIDGET_TYPE eWidgetType , __out BOOL &bHandled)
    {
        bHandled = TRUE;

        switch (eWidgetType)
        {
            // 网络浏览器控件
#ifdef UIBASE_SUPPORT_WEBBROWSER
        case WIDGET_TYPE_WEBBROWSER:
            {   
                return RegisterWebbrowser(heWrapper, hWndParent);
            }
#endif

            // flash控件
#ifdef UIBASE_SUPPORT_FLASH
        case WIDGET_TYPE_FLASH:
            {
                return RegisterFlashPlayer(heWrapper, hWndParent);
            }
#endif

            // 带dock支持的视图
        case WIDGET_TYPE_DOCKVIEW:
            {
                return RegisterDockView(heWrapper, hWndParent);
            }

            // shell 控件
#ifdef UIBASE_SUPPORT_SHELLCTRL     
        case WIDGET_TYPE_SHELLTREE:
        case WIDGET_TYPE_SHELLLIST:
        case WIDGET_TYPE_SHELLCOMBOBOX:
            {
                return RegisterShellCtrl(heWrapper, hWndParent, eWidgetType);
            }
#endif

            // 未知类型控件
        case WIDGET_TYPE_UNKNOWN:
        default:
            {
                // 未知类型的处理不了
                bHandled = FALSE;
                break;
            }
        }
        // 未被处理的控件类型返回NULL
        return NULL;
    }
        
    /** 处理特殊控件的销毁
        *	return:
        *      销毁事件是否被处理，TRUE 被处理
        *	params:
        *		-[in]
        *          heWrapper        控件的包裹元素
        *          eWidgetType      控件类型
    **/
    BOOL HandleDestrucion(__in HELEMENT heWrapper, __in ENUM_WIDGET_TYPE eWidgetType )
    {
        ListWidgetsItr itr = FindWidgetRecord(heWrapper, eWidgetType);
        if (widgets_.end() == itr)
            return FALSE;

        BOOL bHandled = TRUE;
        switch (eWidgetType)
        {
#ifdef UIBASE_SUPPORT_FLASH
        case WIDGET_TYPE_FLASH:
            {
                delete reinterpret_cast<CFlashWnd*>((*itr)->pCtl);
                break;
            }
#endif

#ifdef UIBASE_SUPPORT_WEBBROWSER
        case WIDGET_TYPE_WEBBROWSER:
            {
                CWebBrowser* browser_ = reinterpret_cast<CWebBrowser*>((*itr)->pCtl);
                browser_->DestroyWindow();
                delete browser_;
                break;
            }
#endif

#ifdef UIBASE_SUPPORT_SHELLCTRL
        case WIDGET_TYPE_SHELLLIST:
            {
                delete reinterpret_cast<CShellListCtrl*>((*itr)->pCtl);
                break;
            }
        case WIDGET_TYPE_SHELLTREE:
            {
                delete reinterpret_cast<CShellTreeCtrl*>((*itr)->pCtl);
                break;
            }
        case WIDGET_TYPE_SHELLCOMBOBOX:
            {
                delete reinterpret_cast<CShellComboCtrl*>((*itr)->pCtl);
                break;
            }
#endif
        case WIDGET_TYPE_DOCKVIEW:
            {
                //                    delete reinterpret_cast<QDockView_*>((*itr)->pCtl);
                break;
            }
        case WIDGET_TYPE_UNKNOWN:
        default:
            {
                ATLASSERT(FALSE);
                bHandled = FALSE;
                break;
            }
        }
        if (bHandled)
        {
            delete *itr;
            widgets_.erase(itr);
        }
        return bHandled;
    }

    /** 查询特殊控件
        *	return:
        *      控件指针值，需要自己根据需要进行类型转换
        *	params:
        *		-[in]
        *          heCtl       控件包裹dom值
        *          eWidgetType 控件类型
    **/
    LPVOID QuerySpecialCtrl(__in HELEMENT heCtl, __in ENUM_WIDGET_TYPE eWidgetType)
    {
        ListWidgetsItr itr = FindWidgetRecord(heCtl, eWidgetType);
        if (widgets_.end() != itr)
        {
            return (*itr)->pCtl;
        }
        return NULL;
    }

protected:
#ifdef UIBASE_SUPPORT_WEBBROWSER
    /** 创建网络浏览器控件
        *	return:
        *      网络浏览器控件句柄， NULL 创建失败
        *	params:
        *		-[in]
        *          heWrapper       所在的dom控件
        *          hWndParent      父亲窗口
    **/
    HWND RegisterWebbrowser(__in HELEMENT heWrapper, __in HWND hWndParent)
    {
        CWebBrowser *pHost = new CWebBrowser;
        HWND hWndRet = pHost->Create( hWndParent );
        if ( !pHost->IsWindow() )
        {	// 窗口创建失败
            delete pHost;
            pHost = NULL;
            hWndRet = NULL;
        }
        else
        {	// 创建成功
            pHost->GotoURL(EWebBrowser(heWrapper).get_attribute("src"));
            // 保存下来
            AddWidgetRecord(heWrapper, pHost, WIDGET_TYPE_WEBBROWSER);
        }
        return hWndRet;
    }

#endif

#ifdef UIBASE_SUPPORT_FLASH
    /** 创建flash控件
        *	return:
        *      flash控件句柄， NULL 创建失败
        *	params:
        *		-[in]
        *          heWrapper       所在的dom控件
        *          hWndParent      父亲窗口
    **/
    HWND RegisterFlashPlayer(__in HELEMENT heWrapper, __in HWND hWndParent)
    {
        CFlashWnd* pFlash = new CFlashWnd();
        HWND hWndRet = pFlash->Create(hWndParent, QUIGetInstance());
        if (!IsWindow(hWndRet))
        {
            ATLASSERT(FALSE);
            delete pFlash;
            pFlash = NULL;
        }
        else
        {
            AddWidgetRecord(heWrapper, pFlash, WIDGET_TYPE_FLASH);
            pFlash->PlaySWF(EFlash(pcc->helement).get_attribute("src"));
        }
        return hWndRet;
    }

#endif

    /** 创建dock控件
        *	return:
        *      flash控件句柄， NULL 创建失败
        *	params:
        *		-[in]
        *          heWrapper       所在的dom控件
        *          hWndParent      父亲窗口
    **/
    HWND RegisterDockView(__in HELEMENT heWrapper, __in HWND hWndParent)
    {
//             QDockView_* pDock = new QDockView_();
//             HWND hWndRet = pDock->Create(hWndParent, QUIGetInstance());
//             if (!IsWindow(hWndRet))
//             {
//                 ASSERT(FALSE);
//                 delete pDock;
//                 pDock = NULL;
//             }
//             else
//             {
//                 AddWidgetRecord(heWrapper, pDock, WIDGET_TYPE_DOCKVIEW);
//             }
//             return hWndRet;
        return NULL;
    }

#ifdef UIBASE_SUPPORT_SHELLCTRL     
    /** 创建shell控件(list，tree，combobox)
        *	return:
        *      flash控件句柄， NULL 创建失败
        *	params:
        *		-[in]
        *          heWrapper       所在的dom控件
        *          hWndParent      父亲窗口
        *          eType           WIDGET_TYPE_SHELLCOMBOBOX
        *                          WIDGET_TYPE_SHELLLIST
        *                          WIDGET_TYPE_SHELLTREE
    **/
    HWND RegisterShellCtrl(__in HELEMENT heWrapper, __in HWND hWndParent, 
            __in ENUM_WIDGET_TYPE eType)
    {
        switch (eType)
        {
        case WIDGET_TYPE_SHELLTREE:
            {
                return CreateShellCtrl<CShellTreeCtrl>(
                    heWrapper, hWndParent, 
                    WIDGET_TYPE_SHELLTREE);
            }
        case WIDGET_TYPE_SHELLLIST:
            {
                return CreateShellCtrl<CShellListCtrl>(
                    heWrapper, hWndParent, 
                    WIDGET_TYPE_SHELLLIST);
            }
        case WIDGET_TYPE_SHELLCOMBOBOX:
            {
                return CreateShellCtrl<CShellComboCtrl>(
                    heWrapper, hWndParent, 
                    WIDGET_TYPE_SHELLCOMBOBOX);
            }
        default:
            {
                ATLASSERT(FALSE);
                break;
            }
        }
        return NULL;
    }

    template<typename TShellCtrl>
    HWND CreateShellCtrl(__in HELEMENT heCtl, __in HWND hWndParent, 
        __in ENUM_WIDGET_TYPE eType)
    {
        TShellCtrl *pCtrl = new TShellCtrl();
        HWND hWndRet = pCtrl->Create(hWndParent);
        if ( !pCtrl->IsWindow() )
        {
            delete pCtrl; 
            pCtrl = NULL;
        } 
        else 
        {
            LPCWSTR pszFolder = dom::element(heCtl).get_attribute("src");
            if (NULL == pszFolder)
                pCtrl->Populate();
            else
                pCtrl->Populate(pszFolder);
            AddWidgetRecord(heCtl, pCtrl, eType);
        }
        return hWndRet;
    }
#endif

protected:
    /** 记录特殊控件数据
        *	params:
        *		-[in]
        *          heCtl       控件的包裹dom值
        *          pCtl        控件指针
        *          eType       控件类型
    **/
    inline void AddWidgetRecord(__in HELEMENT heCtl,
                __in LPVOID pCtl,__in ENUM_WIDGET_TYPE eType)
    {
        ATLASSERT(dom::element(heCtl).is_valid());
        ATLASSERT(NULL != pCtl);
        ATLASSERT(WIDGET_TYPE_UNKNOWN != eType);

        widgets_.push_back(new _SPECIAL_WIDGET(heCtl, pCtl, eType));
    }

    inline ListWidgetsItr FindWidgetRecord(
        __in HELEMENT heCtl, __in ENUM_WIDGET_TYPE eType)
    {
        ListWidgetsItr iEnd = widgets_.end();
        for (ListWidgetsItr i = widgets_.begin(); i != iEnd; ++i)
        {
            if ( ((*i)->heCtl == heCtl) && ((*i)->eType == eType) )
            {
                return i;
            }
        }
        return iEnd;
    }

private:
    // 特殊的控件
    std::vector<LP_SPECIAL_WIDGET> widgets_;
};

//////////////////////////////////////////////////////////////////////////
QView::QView(LPCWSTR szResName):event_handler(HANDLE_ALL)
{
	m_sHtmlPath = szResName;
    he_wrapper_ = NULL;
    styles_ = 0;
    p_widgets_ = new _SpecialWidgets;
}

QView::~QView()
{
    if (nullptr != p_widgets_)
    {
        delete p_widgets_;
        p_widgets_ = nullptr;
    }

    if (!IsWindow())
    {
        m_hWnd = NULL;
    }
}

BOOL QView::OnLoadHtml()
{
	if ( !m_sHtmlPath.IsEmpty() )
	{
		// 从皮肤文件中加载界面数据
		QBuffer *pBuf;
		if (QUIMgr::GetInstance()->LoadData(m_sHtmlPath,this,pBuf))
		{
			return QView::LoadHtmlData(pBuf->GetBuffer(0),pBuf->GetBufferLen());
		}
	}
	return FALSE;
}

BOOL QView::LoadHtmlData( LPCBYTE pData,int nLen,LPCWSTR pBaseUrl/*=GetModulePath()+L"skin/"*/ )
{
	BOOL b = HTMLayoutLoadHtmlEx(GetSafeHwnd(), pData, nLen,
		(NULL == pBaseUrl)?(quibase::GetModulePath() + L"skin/"):pBaseUrl);
	return b;
}

BOOL QView::SetCss( LPCBYTE pData,int nDataLen,LPCWSTR pBaseUrl,LPCWSTR pMediaType/*=L"screen"*/ )
{
	return HTMLayoutSetCSS(GetSafeHwnd(),
		pData,nDataLen,
		(NULL == pBaseUrl) ? (quibase::GetModulePath() + L"skin/") : pBaseUrl,
		pMediaType);
}

BOOL QView::LoadFile( LPCWSTR pszFile )
{
	return HTMLayoutLoadFile(GetSafeHwnd(),pszFile);
}

inline ENUM_WIDGET_TYPE QView::DetectWidgetType( ECtrl ctlWidget )
{
    LPCWSTR pszType = ctlWidget.get_attribute("type");
    if (StrCmpIW(pszType, L"windowed") == 0)
        return WIDGET_TYPE_CUSTOM;
    else if (StrCmpIW(pszType, L"toolbar") == 0)
        return WIDGET_TYPE_TOOLBAR;     
    else if (StrCmpIW(pszType, L"dock-container") == 0)
        return WIDGET_TYPE_DOCKCONTAINER;
    else if (StrCmpIW(pszType, L"dockview") == 0)
        return WIDGET_TYPE_DOCKVIEW; 
    else if (StrCmpIW(pszType, L"WebBrowser") == 0)
        return WIDGET_TYPE_WEBBROWSER;
    else if (StrCmpIW(pszType, L"flash") == 0)
        return WIDGET_TYPE_FLASH;
    else if (StrCmpIW(pszType, L"shell-list") == 0)
        return WIDGET_TYPE_SHELLLIST;
    else if (StrCmpIW(pszType, L"shell-tree") == 0)
        return WIDGET_TYPE_SHELLTREE;
    else if (StrCmpIW(pszType, L"shell-combobox") == 0)
        return WIDGET_TYPE_SHELLCOMBOBOX;
    return WIDGET_TYPE_UNKNOWN;
}

LRESULT QView::CreateControl( HWND hWnd, LPNMHL_CREATE_CONTROL pcc )
{
    ENUM_WIDGET_TYPE eWidgetType = DetectWidgetType(pcc->helement);
    if ( WIDGET_TYPE_CUSTOM == eWidgetType )
    {
        QView *pControl = CustomControlCreate(pcc->helement);
        ATLASSERT(NULL != pControl );
        if ( NULL != pControl )
        {   
            ATLVERIFY(_DoCreateCustomCtrl(pcc, pControl));
        }
        return 0;
    }
//     else if (WIDGET_TYPE_DOCKVIEW == eWidgetType)
//     {
//         // 只有DockFrame才支持这个元素
//         ASSERT(FALSE);  
//         return 0;
//     }
    else if (WIDGET_TYPE_UNKNOWN != eWidgetType)
    {
        // 特殊的控件交给特殊控件管理类创建
        BOOL bHandled = FALSE;
        pcc->outControlHwnd = p_widgets_->HandleCreation(pcc->helement, 
                    pcc->inHwndParent, eWidgetType, bHandled);
        if (bHandled)
        {
            return 0;
        }
    }
	// 其他的直接让基类进行默认处理
	return BaseHost::CreateControl(hWnd,pcc);
}

BOOL QView::MakeTransparent(BOOL bTransparent/* = TRUE*/)
{
    CBackHelper(this/*, CBackHelper::BACK_TRANS_RENDERPARENT*/);

    return TRUE;

    using namespace htmlayout;
    using namespace htmlayout::dom;

    // 非子窗口不可以使用透明效果
    if ( ((GetStyle() & WS_CHILD) == 0) || (NULL == he_wrapper_) )
    {
        ATLASSERT(FALSE);
        return FALSE;
    }

    ECtrl eThis = GetBody();
    // html背景URI
    CStdString sBackImgURI;
    sBackImgURI.Format(L"%s_back_%p.png",quibase::GetModulePath(),GetSafeHwnd());
    if ( !bTransparent )
    {
        // 将背景图像删除掉
        char ch = 0;
        return HTMLayoutDataReadyAsync(GetSafeHwnd(), 
            L"file://" + sBackImgURI, (LPBYTE)&ch, 1, HLRT_DATA_IMAGE);
    }
//     else
//     {
//         // 写入透明标记
//         eThis.set_attribute("transparent",L"true");
//     }

    // 首先获取QView相对于Parent的位置
    //！！ <widget type="windowed" /> 不能使用padding属性
    // 需要QView完全的覆盖到widget上面
    ATLASSERT(element(he_wrapper_).get_style_attribute("padding") == NULL);
    WTL::CRect rcCont = element(he_wrapper_).get_location(ROOT_RELATIVE | PADDING_BOX);
    WTL::CRect rcClient = element(element(he_wrapper_).root()).get_location();

    // 找到上层的实体窗口句柄
    BOOL bRet = FALSE;
    image *pImgParent = image::create(rcCont.Width(), rcCont.Height());
    image *pImgThis = NULL;
    do 
    {
        // 先将父窗口整个区域的图像拷贝下来
        if (NULL == (image::create(rcClient.Width(),rcClient.Height())))
            break;
        if ( !HTMLayoutRender(GetParent().m_hWnd,pImgParent->handle(),rcCont) )
            break;
        // 将本窗口与父亲窗口重叠区域的背景保存到图像中
        if (NULL == (pImgThis = image::create(rcCont.Width(), rcCont.Height())))
            break;
        graphics(pImgThis).blit_image(pImgParent,0,0, rcCont.left, rcCont.top,
            rcCont.right, rcCont.bottom);

        // 成功 
        bRet = TRUE;
    } while (false);

    // 将重叠区域图像数据写到内存缓冲区中
    if ( bRet )
    {
        struct write_to_thisback : public htmlayout::writer
        {   
            QBuffer buf;
            virtual bool write( aux::bytes data )
            {
                return buf.Write(data.start,data.length);
            }
        } wtt;
        // 参数0，指示写入格式为png格式
        pImgThis->save(wtt, 0);
        // 使用透明效果
        // 现在改写背景style和背景的图像地址，原来的自定义的背景数据将丢失
        eThis.SetBkgndImage(sBackImgURI);
        eThis.SetBkgndRepeat(L"no-repeat");
        eThis.SetBkgndPosition(0,0,0,0);
        // 将数据更新到指定的URI中
        bRet = HTMLayoutDataReadyAsync(GetSafeHwnd(),L"file://" + sBackImgURI,
            wtt.buf.GetBuffer(0),wtt.buf.GetBufferLen(),HLRT_DATA_IMAGE);

        wtt.buf.FileWrite(L"F:/a.png");
        eThis.SetBkgndImage(L"F:/a.png");
    }

    // 释放资源
    if (NULL != pImgThis)
    {
        delete pImgThis;
    }
    if (NULL != pImgParent)
    {
        delete pImgParent;
    }

    return bRet;
}

LRESULT QView::OnDestroyControl( LPNMHL_DESTROY_CONTROL pnmhl )
{
    ENUM_WIDGET_TYPE eWidgetType = DetectWidgetType(pnmhl->helement);
    if ( (WIDGET_TYPE_UNKNOWN != eWidgetType) 
        && (WIDGET_TYPE_CUSTOM != eWidgetType) )
    {
        if (p_widgets_->HandleDestrucion(pnmhl->helement, eWidgetType))
        {
            // 已经处理销毁了，底层就不要管了
            pnmhl->inoutControlHwnd = NULL;
            return 0;
        }
    }

    // 底层去处理那些没有被特殊控件管理器处理的销毁事件
	return BaseHost::OnDestroyControl(pnmhl);
}

LRESULT QView::OnCreate( LPCREATESTRUCT lpcs )
{
	// 禁用内建的drag-n-drop,使用htmlayout提供的drag-n-drop
	DefWindowProc(WM_CREATE,WPARAM_DISCARD_BUILTIN_DD_SUPP0RT,(LPARAM)lpcs);

	QUIMgr::OnWindowCreate(this);

	// 事件路由连接
	SetCallback(); 

	// set this as a "zero level" event handler, 
	// all not handled HTML DOM events will go here
	SetEventHandler(this); 

	// 加载html皮肤
	if ( !OnLoadHtml() )
	{
        // 销毁掉
        QUIMgr::OnWindowDestroy(this);

        ATLASSERT(FALSE);

        // If the application returns –1, the window is destroyed 
        // and the CreateWindowEx or CreateWindow function 
        // returns a NULL handle.
		return -1;
	}
    
    // 查看都启用了什么特殊效果
    if (GetBody().IsHasAttribute("transparent"))
    {
        ModifyQEXStyle(WS_QEX_BACKTRANSPARENT, 0);
    }

	return 0;
}

void QView::OnDestroy()
{
	// 窗口即将销毁，做最后的清理工作
	QUIMgr::OnWindowDestroy(this);

    // 销毁事件处理器，不再处理任何HL事件 
    __DeinitEventEntry();

	// 一定要将bHandled设置为false，因为派生类可能也需要处理这个消息的
	// QFrame 是一定要处理这个消息的
	SetMsgHandled(FALSE);
}

void QView::OnSize( UINT nState,WTL::CSize sz )
{
    if ( HasQEXStyle(WS_QEX_BACKTRANSPARENT) )
    {
        MakeTransparent(TRUE);
    }
    SetMsgHandled(FALSE);
}

ECtrl QView::GetCtrl( const char* selector, ... )const
{
    using namespace htmlayout::dom;

    char buf[2049] = {0};
    va_list args; 
    va_start ( args, selector ); 
    vsnprintf( buf, 2048, selector, args ); 
    va_end ( args );
    
    element::find_first_callback ff;
    element(root_element(GetSafeHwnd())).select_elements( &ff, buf); 
    return ff.hfound;
}

LRESULT QView::OnHtmlNotify( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    ATLASSERT(uMsg == WM_NOTIFY);

    // Crack and call appropriate method
    LRESULT lResult;
    // here are all notifiactions
    switch(((NMHDR*)lParam)->code) 
    {
    case HLN_CONTROL_CREATED:
        {
            LPNMHL_CREATE_CONTROL p = reinterpret_cast<LPNMHL_CREATE_CONTROL>(lParam);
            ATLASSERT(nullptr != p);
            ATLASSERT(NULL != p->outControlHwnd);

            QUIMgr::OnWidgetWindowCreate(p->helement, p->outControlHwnd);

            break;  // 依然需要调用底层的
        }
    case HLN_DESTROY_CONTROL:
        {
            LPNMHL_DESTROY_CONTROL p = reinterpret_cast<LPNMHL_DESTROY_CONTROL>(lParam);
            ATLASSERT(nullptr != p);
            ATLASSERT(nullptr != p->inoutControlHwnd);

            QUIMgr::OnWidgetWindowDestroy(p->helement, p->inoutControlHwnd);

            break;
        }
    case HLN_LOAD_DATA:         
        {
            LPNMHL_LOAD_DATA pnmld = reinterpret_cast<LPNMHL_LOAD_DATA>(lParam);
            lResult = QUIMgr::GetInstance()->OnHtmlayoutLoadData(pnmld);
            if (LOAD_DISCARD != lResult)
                return lResult;
            return OnLoadData(pnmld);
        }
    case HLN_DOCUMENT_COMPLETE: 
        {
            // 在调用派生类的OnDocumentComplete之前初始化事件、mouse、key处理器
            __InitEventEntry();   
            // 反射事件处理器初始化
            __InitReflectEventEntry();
            // HTML加载完成，是否需要换肤呢？
            _Skinbag* pSkin = QUIMgr::GetInstance()->GetCurSkin();
            if (nullptr != pSkin)
            {
                ApplySkinbag(pSkin);
            }
            return OnDocumentComplete();
        }
    }
    return BaseHost::OnHtmlNotify(uMsg, wParam, lParam);
}

BOOL QView::_DoCreateCustomCtrl( __in LPNMHL_CREATE_CONTROL& pcc, __in QView* pCtl )
{
    // 如果用户返回的窗口还没有创建，则在这儿代理创建
    if (!pCtl->IsWindow() )
    {	
        // 读取子控件的html资源路径
        CStdString sAbsPath = ECtrl(pcc->helement).get_attribute("src");
        if (sAbsPath.IsEmpty())
        {
            sAbsPath = pCtl->GetHtmlPath();
        }
        QUIMgr::ConvertToAbsPackagePath(GetHtmlPath(), sAbsPath);
        // 如果是资源包中的相对路径，那么更正为资源包的绝对路径
        pCtl->SetPrivateData(sAbsPath,pcc->helement);
        
        // 创建实体窗口
        // WS_CLIPSIBLINGS|WS_CLIPCHILDREN 防止窗口闪烁
        pcc->outControlHwnd  = pCtl->Create(pcc->inHwndParent,NULL,NULL, 
            WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
    }
    else
    {
        pcc->outControlHwnd = pCtl->GetSafeHwnd();
    }
    return ::IsWindow(pcc->outControlHwnd) 
        && (pcc->inHwndParent == ::GetParent(pcc->outControlHwnd ));
}

void QView::SetElementsAttibute( __in LPCWSTR szSelector, 
    __in LPCSTR szAtti, __in LPCWSTR szVal )
{
    struct _forall_sel : public htmlayout::dom::callback
    {
        LPCSTR  atti_;
        LPCWSTR val_;
        
        _forall_sel(LPCSTR a, LPCWSTR v)
            :atti_(a), val_(v)
        {

        }

        virtual bool on_element(HELEMENT he)
        {
            ECtrl(he).set_attribute(atti_, val_);

            return false;
        }
    };

    GetRoot().select_elements(&_forall_sel(szAtti, szVal), szSelector);
}

BOOL QView::ApplySkinbag( __in _Skinbag* pBag )
{
    if (nullptr == pBag)
    {
        ATLASSERT(FALSE);
        return FALSE;
    }

    struct _forall_sel : public htmlayout::dom::callback
    {
        LPCSTR  atti_;
        LPCWSTR val_;
        LPCSTR xcall_;

        _forall_sel(LPCSTR a, LPCWSTR v, LPCSTR c)
            :atti_(a), val_(v), xcall_(c)
        {

        }

        virtual bool on_element(HELEMENT he)
        {
            ECtrl ctl(he);
            ctl.set_attribute(atti_, val_);

            if (strlen(xcall_))
            {
                ctl.xcall(xcall_);
            }
            return false;
        }
    };

    ECtrl cRoot = GetRoot();
    SkinItems* pAll = pBag->GetAll();
    for (auto i = pAll->begin(); i != pAll->end(); ++i)
    {
        _SkinItem *p = *i;
        cRoot.select_elements(&_forall_sel(p->atti_ , p->val_, p->xcall_),
            p->selector_);
    }

    // 强制刷新窗口
    HTMLayoutUpdateWindow(GetSafeHwnd());

    return TRUE;
}

inline htmlayout::dom::ECtrl QView::GetRoot() const
{ 
    return ECtrl::root_element(GetSafeHwnd()); 
}

inline htmlayout::dom::ECtrl QView::GetFocus() const
{ 
    return ECtrl::focus_element(GetSafeHwnd()); 
}

inline htmlayout::dom::ECtrl QView::GetBody() const
{
    return GetCtrl("body"); 
}

inline htmlayout::dom::ECtrl QView::CtrlFromPoint( POINT ptClient ) const
{ 
    return dom::element::find_element(GetSafeHwnd(),ptClient); 
}

inline htmlayout::dom::ECtrl QView::GetWrapperDOM() const
{
    return he_wrapper_;
}

LPVOID QView::QuerySpecialCtrl( __in HELEMENT heCtl, __in ENUM_WIDGET_TYPE eWidgetType )
{
    return p_widgets_->QuerySpecialCtrl(heCtl,eWidgetType);
}

