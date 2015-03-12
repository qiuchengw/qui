#ifndef QUIMgr_h__
#define QUIMgr_h__

#pragma once

// // #include "QUI.h"
#include <list>
#include "BaseType.h"
#include "QBuffer.h"
#include "QUIGlobal.h"
#include "file/inifile.h"
#include "crypt/CXUnzip.h"
#include "crypt/QDataEnDe.h"

#include "QUIResource.h"

// html文件中的变量分隔符
#define CHAR_HTMLVAR_SEP L'^'

// 资源包中的资源指示
//	如 $:Mainframe\index.html. 资源包根目录下的Mainframe\index.html文件。
#define QUI_UIPAK_ABSINDICATOR L"qabs:"		// ui包中的绝对路径
#define QUI_UIPAK_RELATEINDICATOR L"qrel:"	// ui包中的相对路径

class QSkinUI;
class QUIFunction;
class _Skinbag;

/**
 *	QUIBase 的一些功能性配置
 *		
**/

// 不支持多线程
// 资源包中数据规范：
//	1，html必须是utf-8格式
//	2，
class QUIMgr
{
	friend class QView;
    friend class QApp;

	SINGLETON_ON_DESTRUCTOR(QUIMgr)
	{
		Shutdown();
	}

    class _UIData : public QUIResource
    {
    public:
        virtual BOOL DecodeResource(__in const CStdString &sResName, __inout QBuffer* pData);

    protected:
        virtual BOOL OnLoadFromZip(CXUnzip *zip);

    private:
        QDataEnDe   ende_;
    }res_;

public:
	/** 在使用htmlayout相关特性（如创建窗口）之前，必须调用此函数
	 *	-return:	
	 *		TRUE	成功
	 **/
	BOOL Startup(__in const wchar_t* ui_zip = L"UI.zip");

    inline void SetUIFunction(__in QUIFunction* p)
    {
        m_pFunction = p;
    }

    inline QUIFunction* GetUIFunction()const
    {
        return m_pFunction;
    }

	inline BOOL IsStartup()const 
    { 
        return res_.GetResourceCount() > 0; 
    }

	/** 从皮肤包中加载数据
	 *	-return:	
	 *		LOAD_OK		加载成功
	 *		LOAD_DISCARD	加载失败
	 *	-params:	
	 *		-[inout]	
	 *			pns		参见代码QView::OnLoadData
	 **/
	LRESULT OnHtmlayoutLoadData( __inout LPNMHL_LOAD_DATA pns );
	/** 获取HTML文件数据
	 *	-return:	
	 *		TRUE	成功
	 *	-params:	
	 *		-[in]	
	 *			szFileName		资源文件名（资源包路径或者本地文件路径）
	 *			pView			如果资源文件是资源包中的相对路径，那么需要此参数重新解析路径
	 *		-[out]	
	 *			pBuf			读取到的数据
	 **/
    BOOL LoadData(LPCWSTR szResPath,__in QView* pView,__out QBuffer* &pBuf);
    BOOL LoadData(LPCWSTR szResPath,__in HELEMENT hPrincipal,__out QBuffer* &pBuf);
    /** 从窗口句柄获得其对应的QView
	 *	-return:	
	 *		成功返回对应的QView*，否则为NULL
	 *	-params:	
	 *		-[in]	hWnd	窗口句柄
	 **/
	static QView* QWindowFromHWND(__in HWND hWnd);

	/** 从Html元素获得其对应的QView
	 *	-return:	
	 *		成功返回对应的QView*，否则为NULL
	 *	-params:	
	 *		-[in]	he	dom元素
	 **/
	static QView* QWindowFromHELEMENT(__in HELEMENT he);

    static HWND HwndOfWidgetDOM(__in HELEMENT he);
    static HELEMENT DOMOfWidgetHwnd( __in HWND hWnd );

    /**
     *	在进程范围内进行换肤
     *      针对的是QView及其派生类的窗口换肤
     *      在调用之前，应该确保数据包已经被完全加载（LoadSkinbag）
     *	param
     *		-[in]
     *          pBag    皮肤数据
     *                  如果为NULL，新创建的HL窗口不使用换肤功能
    **/
    static BOOL ApplyStylebag(__in _Skinbag* pBag);
    // 如果加载数据包是异步的，则需要提供回调机制，通知何时加载完成了
    static BOOL PrepareSkinbag(__in _Skinbag* pBag);
    static BOOL LoadSkinbag(__in _Skinbag* pBag);
    static BOOL SkinbagComplete(__inout _Skinbag* p);

    // 对所有的QView && popup窗口设置透明度， 范围[0, 255]
    static void ApplyOpacity(__in int nAlpha);
    
    /**
     *	向当前进程中所有HL窗口uri指引资源更新数据
     *		
     *	params
     *		-[in]
     *			uri     资源路径
     *          pData   数据
     *          nDataLen    数据长度
     *			bIfExist    TRUE    只有在已经存在这个uri的时候才更新
    **/
    void UpdateResDataWith(__in LPCWSTR uri, __in LPBYTE pData, __in int nDataLen, BOOL bIfExist = TRUE);
    // 设置数据为空（其实是一个字节，内容为 0）
    void UpdateResDataWithNULL(LPCWSTR uri)
    {
        char NULL_RES_DATA = 0;
        UpdateResDataWith(uri, (LPBYTE)&NULL_RES_DATA, sizeof(char));
    }

public:
	/** QView窗口创建的时候要注册到此处
	 *	-params:	
	 *		-[in]	
	 *			pView	窗口初次创建时注册，只能注册一次
	 **/
	static void OnWindowCreate(__in QView *pView);
	/** Htmlayout窗口销毁前做清理工作 
	 *	-params:	
	 *		-[in]	
	 *			pView	窗口即将销毁时调用
	 **/
	static void OnWindowDestroy(__in QView* pView);

    static BOOL OnWidgetWindowCreate( __in HELEMENT hWidget, __in HWND hWnd );
    static void OnWidgetWindowDestroy(__in HELEMENT hWidget, __in HWND hWnd);

    inline std::vector<QView*>* GetViews()
    {
        return &m_lstWnd;
    }

    static _Skinbag* GetCurSkin()
    {
        return m_pCurSkin;
    }

private:
	enum ENUM_UI_RESPATH 
	{
		UI_RESPATH_ERROR = -1,	// 错误的,字符串为空
		UI_RESPATH_UNKOWN = 0,	// 未知，字符串不为空，且不为以下任何一种情况
		UI_RESPATH_PKGRELATE,	// [qrel:] 本地包，相对路径
        UI_RESPATH_PKGABS,		// [qabs:] 本地包，绝对路径
        //		UI_RESPATH_HTMLAYOUT,	// [res:] | [theme:]；HTMLayout 使用的资源符号，
        //		UI_RESPATH_LOCAL,		// [x:xx.jpg] | [xx.jpg] ,本地文件，绝对或相对路径
        //        UI_RESPATH_MEMORY,	    // [qmem:] 内存数据
	};
    static QUIMgr::ENUM_UI_RESPATH DetectDataPathType( __in const CStdString& sPath );

    /*
	 *	hPrincipal	为要求此资源的DOM元素
	 *				如果为NULL，那么相对路径会解析失败
	 */
	static ENUM_UI_RESPATH ParseDataPath(__inout CStdString& sPath,__in HELEMENT hPrincipal);
	/*
	 *	pView	为要求此资源的DOM元素所在的Htmlayout窗口
	 ×			如果为NULL，那么相对路径会解析失败
	 */
	static ENUM_UI_RESPATH ParseDataPath(__inout CStdString& sPath,__in QView *pView);
	static BOOL ConvertToAbsPackagePath( const CStdString& sAbsPath,__inout CStdString& sRelatePath );

    // 关闭所有的窗口
    void Shutdown();

    // szOriginPath ： 用于debug 
    BOOL _LoadData(__in LPCWSTR szResPath, __in LPCWSTR szOriginPath,
        ENUM_UI_RESPATH nType, __out QBuffer* &pBuf);

private:
	// 系统维护的已创建的QView/QFrame窗口列表
    static std::vector<QView*>    m_lstWnd;
    // <widget #id >关联的HWND
    static std::map<HELEMENT, HWND>    m_mapHwndDOM;
    // 当前程序使用的皮肤包
    static _Skinbag*           m_pCurSkin;
    static QUIFunction*         m_pFunction;
};

// class QHtmlMgr
// {
// 	QHtmlMgr():m_bStarted(FALSE) {}
// 	SINGLETON_ON_DESTRUCTOR(QHtmlMgr){}
// public:
// 	// 传入语言文件路径
// 	BOOL Startup();
// 	BOOL TranslateHtmlLanguageVar( __inout QString& sHtml );
// 	QString GetLaugString( LPCWSTR szSec,LPCWSTR szKey );
// 	inline BOOL IsStartup()const { return m_bStarted; }
// 
// protected:
// 	BOOL ParseVar(__in const QString& str, __out QString& sSection,__out QString &sKey);
// 
// private:
// 	CIniFile		m_cIni;
// 	BOOL			m_bStarted;
// };

inline QView* QUIViewFromHWND(HWND hWnd)
{
    return QUIMgr::QWindowFromHWND(hWnd);
}

inline  QView* QUIViewFromHELEMENT(HELEMENT he)
{
    return QUIMgr::QWindowFromHELEMENT(he);
}

inline  HELEMENT QUIWidgetDOMFromHwnd(__in HWND hWnd)
{
    return QUIMgr::DOMOfWidgetHwnd(hWnd);
}

inline  HWND QUIHwndFromWidgetDOM(HELEMENT he)
{
    return QUIMgr::HwndOfWidgetDOM(he);
}

// 检测资源类型（根据扩展名）
HTMLayoutResourceType QUIGetResourceType(__in const CStdString& uri);

// 提取 ':' 之后的数据，不包括 ':',如果没有找到 ':' 返回全部字符串
//      qrel:xxx -> xxx
inline void QUITrimPrefix(__inout CStdString& sPath)
{
    sPath = sPath.Trim();
    int idx = sPath.Find(L':', 0);
    if ( -1 != idx)
    {
        sPath = sPath.Mid(idx+1);
    }
}

/**
 *	分解URI为指示符和资源路径，以[:]符号分割两部分
 *		uri             qrel:xxx 
 *      indicator       qrel: (带有 [:])
 *      path            xxx
 *	params
 *		-[in]
 *			szURI       资源路径
 *		-[out]
 *			sIndicator  指示符号
 *          sPath       路径
**/
BOOL QUISplitResPath(__in LPCWSTR szURI, 
    __out CStdString& sIndicator, __out CStdString& sPath);


#endif // QUIMgr_h__
