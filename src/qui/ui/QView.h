#ifndef QVIEW_H__
#define QVIEW_H__

#pragma once

#ifndef __ATLBASE_H__
#include <atlbase.h>
#endif

#ifndef __ATLAPP_H__
#include <atlapp.h>
#endif

#ifndef __ATLCRACK_H__
#include <atlcrack.h>
#endif

#ifndef __ATLMISC_H__
#include <atlmisc.h>
#endif

#ifndef __ATLWIN_H__
#include <atlwin.h>
#endif

#include "QUIEvent.h"
#include "QUIGlobal.h"
#include "htmlayout/wtl_htmlayout.h"
#include "htmlayout/wtl_htmlayouthost.h"

//#include "dock/MultiPaneCtrl.h"

#ifdef UIBASE_SUPPORT_FLASH
    #include "FlashWnd.h"
#endif

#ifdef UIBASE_SUPPORT_SHELLCTRL
    #include "wtl/ShellCtrls.h"
#endif

#pragma comment(lib,"htmlayout.lib")

// 扩展风格，和其他的ws_ex 不冲突
enum
{
    WS_QEX_ROUNDCONNER = 0x1,	 // 圆角窗口
    WS_QEX_WNDSHADOW = 0x2L,	// 阴影效果
    WS_QEX_BACKTRANSPARENT = 0x4L,	// 适用于实体子窗口(HWND)的假性透明
    // !!! 2013 / 10/ 31 取消支持
    // WS_QEX_DRAGMOVE = 0x8L,  // 在窗口内任何地方点击，都能移动窗口，（否则只能在title上点击才可以移动）
    // WS_QEX_FULLSCRNWND = 0x10L,  // 窗口全屏
    WS_QEX_THICKFRAME = 0x20L,  // 不用系统的ws_thickframe也能调整手动调整窗口大小
};

// 自定义的窗口控件类型 <widget type="theWidget" />
enum ENUM_WIDGET_TYPE
{
    WIDGET_TYPE_UNKNOWN = 0,
    WIDGET_TYPE_CUSTOM,         // type="windowed"
    WIDGET_TYPE_TOOLBAR,        // type="toolbar"
    WIDGET_TYPE_DOCKVIEW,        // type="dockview"
    WIDGET_TYPE_DOCKCONTAINER,        // type="dock-container"
    WIDGET_TYPE_WEBBROWSER,     // type="WebBrowser"
    WIDGET_TYPE_FLASH,          // type="flash"
    WIDGET_TYPE_SHELLLIST,      // type="shell-list"
    WIDGET_TYPE_SHELLTREE,      // type="shell-tree"
    WIDGET_TYPE_SHELLCOMBOBOX,  // type="shell-combobox"
};

// 使用一个函数处理QUI_MSG_QUI_MSG_CODEMSG
#define MSG_QUI_MSG(func)\
	if (uMsg == UIBASE_MSG_CODEMSG) \
	{ \
		SetMsgHandled(TRUE); \
		func(wParam,lParam); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}
// 使用多个函数处理QUI_MSG_CODEMSG的wParam号分支
#define BEGIN_QUI_MSG\
	if (uMsg == UIBASE_MSG_CODEMSG) \
	{ \
		SetMsgHandled(TRUE);\
        lResult = 0; \
		switch(wParam)\
		{
// 无返回值的code函数
#define MSG_QUI_CODE(__c,__f)\
		case __c:{ __f(lParam); break;}
// 带有返回值的code函数
#define MSG_QUI_CODE_WITHRET(__c,__f)\
        case __c:{ lResult = __f(lParam); break;}
// 结束消息代码段
#define END_QUI_MSG\
		}\
		if(IsMsgHandled()) \
			return TRUE; \
	}

class _SpecialWidgets;

//////////////////////////////////////////////////////////////////////////
// 基础htmlayout实体窗口类
//		提供嵌入式的子窗口封装实现
//		不可用于顶层窗口
// html 属性值
//      1，tranparent  子实体窗口透明            
//                     仅对子QView窗口有效    <body tranparent=1>
//      2，-border-radius  启用顶级窗口圆角效果       
//                     仅对顶层QFrame有效    <body style="-border-radius=w/h;" > 属性值单位是px
//                     w / h -> 参见CreateRoundRectRgn最后两个参数
//      3，drag-move   在任何地方点击都能移动窗口  
//                     仅对顶层QFrame有效    <body drag-move=1 />
//      4，fullscrn-window    全屏显示窗口               
//                     仅对顶层QFrame有效    <body fullscrn-window=1>
// 高级自定义窗口
//      <widget type="theWidget">   theWidget为下列值中的一个
//      浏览器         WebBrowser
//      flash播放器    flash
//      shell树        shell-tree
//      shell列表      shell-list
//      shell组合框    shell-combobox
class _Skinbag;
class QView : 
	public QUIEventTarget,
	public CWindowImpl<QView,CHTMLayoutCtrl>,
	public htmlayout::event_handler,
	public CHTMLayoutHost<QView>,
	public htmlayout::notification_handler<QView>//,
//    public MultiPaneCtrlNotify      // 任何一个QView都可以获得dock支持
{
    // 如果子类要使用 BEGIN_MSG_MAP 处理消息，那么一定要先使用
    // CHAIN_MSG_MAP() 将 消息先交给它的父类处理。
    // !!!! 如果派生类要处理WM_CLOSE消息，那么处理后一定要将bHandled设置为false
    //		以便wm_close 消息传递到上层处理，否则不会产生wm_destroy 消息
    //		窗口资源得不到释放，将会发生内存泄露
    BEGIN_MSG_MAP_EX(QView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        MSG_WM_DESTROY(OnDestroy)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

	typedef CHTMLayoutHost<QView> BaseHost;
	typedef CWindowImpl<QView,CHTMLayoutCtrl> BaseWnd;

public:
	QView(LPCWSTR  szResName=NULL);

    virtual ~QView();

    /**
     *  获取UI资源的路径
    **/
	inline CStdString GetHtmlPath()const 
    { 
        return m_sHtmlPath; 
    }

	inline HWND GetSafeHwnd()const 
    {
        return m_hWnd; 
    }
    /**
     *  获取当前具有输入焦点控件
    **/
	inline ECtrl GetFocus()const;
    
    /**
     *  获取标签<html>对应的控件值
    **/
	inline ECtrl GetRoot() const;

    /**
     *  获取标签<body>对应的控件值
    **/
	inline ECtrl GetBody() const;
    
    /** 某点下的控件
     *	return:
     *      ptClient点下的控件值
     *	params:
     *		-[in]
     *          ptClient        客户区坐标，非window坐标
    **/
	inline ECtrl CtrlFromPoint(POINT ptClient)const;

    /** 查询特殊控件
     *	return:
     *      控件指针值，需要自己根据需要进行类型转换
     *	params:
     *		-[in]
     *          heCtl       控件包裹dom值
     *          eWidgetType 控件类型
    **/
    LPVOID QuerySpecialCtrl(__in HELEMENT heCtl, __in ENUM_WIDGET_TYPE eWidgetType);

    /** 调用此函数使得本窗口能够实现假性半透明效果（使父窗口背景平铺到本窗口上）
     *      1，顶层窗口（ws_popup 不能使用此效果）
     *      2，此函数会改写标签<html>背景style和背景的图像地址，原来的背景数据将丢失
                  如果使用原来自定义的背景图像，那么就不要调用此函数
     *      3，只适用于【大小不变化、非动态背景】的窗体
     *      4，父窗口背景变化的时候，应该再次手动调用此函数
     *	return:
     *      返回FALSE的情况：
     *          1，窗口不具有ws_child属性
     *          2，父亲背景获取不成功
     *	params:
     *		-[in]
     *          bTransparent       TRUE     透明
    **/
    BOOL MakeTransparent(__in BOOL bTransparent = TRUE);

    /** !!! 此函数仅供上级QView通过CreateControl在
     *      实例化<widget type="windowed" src="path_of_html" />
     *      时调用，详见CreateControl的实现
     *	params:
     *		-[in]
     *          szHtmlPath      设置为src的属性值path_of_html
     *          heWrapper       设置为widget的DOM值
    **/
    void SetPrivateData(__in LPCWSTR szHtmlPath, __in HELEMENT heWrapper) 
    {
        m_sHtmlPath = szHtmlPath; 
        he_wrapper_ = heWrapper;
    }

    /** 修改QEX style，
     *	params:
     *		-[in]           参数参见 WS_QEX_...
     *          bitsAdd     添加的风格
     *          bitsRemove  移除的风格
    **/
    inline void ModifyQEXStyle(UINT bitsAdd,UINT bitsRemove)
    {
        ATLASSERT( (bitsRemove & bitsAdd) == 0); // 无交集的bit位
        _AddFlag(styles_, bitsAdd);
        _RemoveFlag(styles_, bitsRemove);
    }

    inline BOOL HasQEXStyle(UINT bits) const 
    {
        return styles_ & bits;
    }

    /**
     *	对整个HL的DOM树中所有szSelector元素替换属性
     *	param
     *		-[in]
     *          szSelector      选择器
     *          szAtti          属性字段
     *          szVal           属性值
     *
    **/
    void SetElementsAttibute(__in LPCWSTR szSelector, 
        __in LPCSTR szAtti, __in LPCWSTR szVal);

    virtual BOOL ApplySkinbag(__in _Skinbag* pBag);

    // 事件和消息
protected:
	/** 
	 *	派生类不要处理OnCreate消息，如果需要做初始化，可以重载OnHtmlLoaded
	 **/
	LRESULT OnCreate(LPCREATESTRUCT lpcs);
    /** 
	 * 在窗口大小变化的时候处理透明子窗口
	 **/
	void OnSize(UINT nState,WTL::CSize sz);
	/** 
	 * 派生类如果处理了WM_DESTROY消息，最后一定要将消息再传递到这儿来
	 **/
	void OnDestroy();

protected:
	/** 点击在具有xrole属性的按钮上的响应函数
	 *	-return:	
	 *			TRUE，继续处理，关闭窗口
	 *			FALSE，不再执行默认行为（不会关闭窗口了）
	 *	-params:	
	 *		-[in]	nBtn	[IDOK|IDCANCEL...]
	 **/
	virtual BOOL OnDefaultButton(INT_PTR nBtn) { return TRUE; };

	/** 重写此函数完成自定义的控件的创建
	 *	-return:		QView 类的指针
	 *				自定义的控件必须派生自QView
	 *	-params:	
	 *		-[in]	he	在QView窗体创建之前，它的DOM元素就已经创建好了
	 **/
    virtual QView* CustomControlCreate(HELEMENT he) { return NULL; };

	// 自定义窗口控件销毁
	virtual LRESULT OnDestroyControl(LPNMHL_DESTROY_CONTROL pnmhl);

    inline ENUM_WIDGET_TYPE DetectWidgetType(ECtrl ctlWidget);

	// 自定义窗口控件创建
	virtual LRESULT CreateControl(HWND hWnd, LPNMHL_CREATE_CONTROL pnmcc);

    // HL的事件通知
    virtual LRESULT OnHtmlNotify(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	/**窗口（HWND）创建完成后调用此函数以加载UI
	 *		可以覆盖此函数自己加载HTML代码
	 *	-return:		TRUE 加载成功，将调用OnHtmlLoaded
	 **/
	virtual BOOL OnLoadHtml() ;

	/** 加载磁盘HTML文件 
	 *	-return:		TRUE	加载成功
	 *	-params:	
	 *		-[in]	pszFile  磁盘文件路径，可以是全路径和相对路径，
	 *							文件内容必须是html格式
	 **/
	BOOL LoadFile(LPCWSTR pszFile);

	/** 从内存加载HTML文档
	 *	-return:		TRUE	加载成功
	 *	-params:		
	 *		-[in]	pData	指向数据内存
	 *				nLen	数据长度，以Byte计数
	 *				pBaseUrl	文档使用资源的查找路径
	 **/
	BOOL LoadHtmlData(LPCBYTE pData,int nLen,LPCWSTR pBaseUrl=NULL);

	/** 设置UI的CSS
	 *	-return:		TRUE	设置成功
	 *	-params:	
	 *		-[in]	pData	数据
	 *				nLen	数据长度，以Byte计数
	 *				pBaseUrl	CSS使用资源的查找路径
	 *				pMediaType	参考SCiter API
	 **/
	BOOL SetCss( LPCBYTE pData,int nDataLen,LPCWSTR pBaseUrl=NULL,LPCWSTR pMediaType=L"screen" );
    
    /** 自定义的实体窗口控件（DOM-HWND）资源解析
     *      语法: <widget src="....html_path_name" />
     *      src属性值html_path_name需要经过QUIMgr解析。
     *	return
     *      pCtl 对应的窗口HWND
     *	param
     *		-[in]
     *          pcc         DOM
     *          pCtl        DOM对应的实体窗口
    **/
    BOOL _DoCreateCustomCtrl( __in LPNMHL_CREATE_CONTROL& pcc, __in QView* pCtl );

protected: 
	/** 选中第一个匹配selector的DOM元素
	 *	-return:		DOM		匹配的元素	
	 *	-params:	
	 *		-[in]	selector	标准的CSS语法
	 **/
	ECtrl GetCtrl(const char* selector, ...)const;

    inline ECtrl GetWrapperDOM()const;
    
protected:
	CStdString		m_sHtmlPath;	// 在QUIMgr中的窗体UI名字

private:
    // 假如本窗口是<widget type="windowed" />的展开实例化（见CreateControl）
    //      he_wrapper_则为widget的DOM值（使用GetRoot().parent()）是获取不到这个值的
    // 如果本窗口是WS_POPUP窗口（非CreateControl创建）
    //      he_wrapper_无效
    HELEMENT            he_wrapper_; 
    UINT		        styles_;  // 见WS_QEX_...
    _SpecialWidgets*    p_widgets_;
};

#endif // QView_h__
