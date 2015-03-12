#ifndef QFrame_h__
#define QFrame_h__

#pragma once

#include "QView.h"
#include "ECtrl.h"
#include "WndShadow.h"

class CWndShadow;

// 基础htmlayout实体窗口类
//		提供顶层窗口的封装实现。
class QFrame: public QView
{
	BEGIN_MSG_MAP_EX(QFrame)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
        MSG_WM_NCHITTEST(OnWmNCHittest)
		MESSAGE_HANDLER_EX(WM_SIZE,HackWndShadowMsg)
		MESSAGE_HANDLER_EX(WM_MOVE,HackWndShadowMsg)
		MESSAGE_HANDLER_EX(WM_EXITSIZEMOVE,HackWndShadowMsg)
		MESSAGE_HANDLER_EX(WM_SHOWWINDOW,HackWndShadowMsg)
		MSG_WM_ACTIVATE(OnActive)
        CHAIN_MSG_MAP(QView)
	END_MSG_MAP()
		
public:
	QFrame(LPCWSTR szResName=NULL);

    virtual ~QFrame();
    
	inline INT_PTR GetRetCode()const { return m_nRetID; }

	/** 创建非模态窗口，可以是顶层或者是子窗口
	 *	-return:		TRUE		创建成功
	 *	-params:	
	 *		-[in]	
	 *				hWndParent			默认是当前处于激活状态的顶级(top)窗口
	 *				nDecoratedStyle		自定义的窗口美化风格
	 *									查看风格标志定义时的说明
     *              rcWnd               窗口大小和位置，如果为NULL，使用默认算法
	 **/
	BOOL Create(HWND hWndParent = ::GetAncestor(::GetActiveWindow(),GA_ROOT),
		UINT nStyle=WS_POPUP|WS_VISIBLE,UINT nStyleEx=0,
		UINT nDecoratedStyle = WS_QEX_WNDSHADOW|WS_QEX_ROUNDCONNER,
        LPCRECT rcWnd = NULL);

	/** 设置标题栏的文字，标题栏（#wc-title）
	 *		GetWindowText 可以获得设置的标题
	 *	-params:	
	 *		-[in]	pszTitle		标题
	 **/
	void SetTitle(LPCWSTR pszTitle);

	// 此组函数用于设置顶层窗口的阴影（如果有）的美化效果
	inline void SetShadowSize(int NewSize = 0);

	inline void SetShadowSharpness(unsigned int NewSharpness = 5);

	inline void SetShadowDarkness(unsigned int NewDarkness = 200);
	
	inline void SetShadowPosition(int NewXOffset = 5, int NewYOffset = 5);
	
	inline void SetShadowColor(COLORREF NewColor = 0);

protected:
	// 事件处理器，派生类如果重载此函数，在返回之前须再调用此函数
	virtual BOOL handle_event (HELEMENT he, BEHAVIOR_EVENT_PARAMS& params );
	// 切换窗口的最大化和普通状态
	void ToggleFrameMaxium();
    // HL的事件通知
    virtual LRESULT OnHtmlNotify(UINT uMsg, WPARAM wParam, LPARAM lParam);

	// 系统消息（WM_XXXXX）处理器
	void OnDestroy();
    void OnGetMinMaxInfo(LPMINMAXINFO pi);
	void OnActive(UINT nState, BOOL bIsMinimazed, HWND hWnd);
    LRESULT OnWmNCHittest( WTL::CPoint pt );
    LRESULT HackWndShadowMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    LRESULT OnCreate( LPCREATESTRUCT lpcs );

protected:
	INT_PTR			m_nRetID;		// 模态对话框使用的返回值,当点击[xrole=]元素的时候设置

private:
    WTL::CRgn       rgn_frame_;         // 圆角美化
	CWndShadow		wnd_shadow_;		// 阴影效果组件
    CSize           size_min_;            // 窗口最小大小
    ECtrl           ctl_title_;       // 窗口标题
};

#endif // QFrame_h__
