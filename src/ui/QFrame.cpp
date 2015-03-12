#include "stdafx.h"
#include "QFrame.h"
#include "QUIMgr.h"
#include "QApp.h"
#include "AppHelper.h"
#include "WndHelper.h"

// 特殊控件（创建、销毁、查询）管理器

//////////////////////////////////////////////////////////////////////////
// QFrame
QFrame::QFrame( LPCWSTR szResName/*=NULL*/ )
    :QView(szResName), m_nRetID(IDCANCEL)
{
}

QFrame::~QFrame()
{
}

BOOL QFrame::Create( HWND hWndParent /*= ::GetAncestor(::GetActiveWindow(),GA_ROOT)*/, 
	UINT nStyle/*=WS_POPUP|WS_VISIBLE*/,UINT nStyleEx/*=0*/, 
	UINT nDecoratedStyle /*= WS_QEX_WNDSHADOW|WS_QEX_ROUNDCONNER*/,
    LPCRECT rcWnd/* = NULL*/)
{
	ATLASSERT(!IsWindow());
	_RemoveFlag(nStyle,WS_CHILD);     // QFrame 不能用于创建子窗口
	_AddFlag(nStyle,WS_CLIPCHILDREN); // 防止内嵌子HWND窗口时闪烁
    ModifyQEXStyle(nDecoratedStyle, 0);

    CRect rc(0, 0, 0, 0);
    if (NULL != rcWnd)
    {
        rc = *rcWnd;
    }

	if (QView::Create(hWndParent, rc, NULL, nStyle, nStyleEx) != NULL)
	{	
        // 看下都具有什么系统的按钮
        if (GetCtrl("#wc-caption [xrole=\"MINIMAZE\"]").is_valid())
        {
            ModifyStyle(0,WS_MINIMIZEBOX);
        }
        if (GetCtrl("#wc-caption [xrole=\"MAXIMAZE\"]").is_valid())
        {
            ModifyStyle(0,WS_MAXIMIZEBOX);
        }

		if ( _HasFlag(QUIGetApp()->GetTopFrameStyle(), WS_QEX_WNDSHADOW) 
                && HasQEXStyle(WS_QEX_WNDSHADOW) )
		{
			// 使用阴影美化窗体
            SetShadowPosition(0,0);
            SetShadowSharpness(4);
            SetShadowSize(3);
            SetShadowColor(RGB(80, 80, 80));
            wnd_shadow_.Create(GetSafeHwnd());
		}
        return TRUE;
	}
	return FALSE;
}

void QFrame::OnDestroy()
{
	// 如果有阴影效果，先销毁阴影
    wnd_shadow_.Destroy();

	// 激活父窗口
	HWND hWndParent = GetParent();
	if (NULL != hWndParent)
	{
		::EnableWindow(hWndParent,TRUE);
		::SetForegroundWindow(hWndParent);
	}

	// 设置为false，因为其派生类可能也需要处理这个消息
	SetMsgHandled(FALSE);
}

LRESULT QFrame::HackWndShadowMsg( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch (uMsg)
	{
	case WM_SIZE:
		{
			if (_HasFlag(QUIGetApp()->GetTopFrameStyle(), WS_QEX_ROUNDCONNER) 
                 && HasQEXStyle(WS_QEX_ROUNDCONNER))
			{
                int nRaidus = GetBody().attribute("-border-radius",0);
                if (nRaidus > 0)
                {
                    rgn_frame_.CreateRoundRectRgn(0,0,LOWORD(lParam)+2,
                            HIWORD(lParam)+2,nRaidus, nRaidus);
                    SetWindowRgn(rgn_frame_, TRUE);
                }
			} 
			// !!!!DO NOT break;
		}
	case WM_MOVE:
	case WM_EXITSIZEMOVE:
	case WM_SHOWWINDOW:
		{	// 如果需要阴影美化，则给阴影窗口处理
			if ( _HasFlag(QUIGetApp()->GetTopFrameStyle(), WS_QEX_WNDSHADOW) 
                && HasQEXStyle(WS_QEX_WNDSHADOW)
				&& wnd_shadow_.IsValid())
			{
				wnd_shadow_.ParentProc(GetSafeHwnd(),uMsg,wParam,lParam);
			}
			break;
		}
	}

    SetMsgHandled(FALSE);
	
    return 0;
}

BOOL QFrame::handle_event( HELEMENT he, BEHAVIOR_EVENT_PARAMS& params )
{
	if (BUTTON_CLICK == params.cmd)
	{
		const wchar_t* btnRole = element(he).get_attribute("xrole");
		if(btnRole)
		{
			if( aux::wcseqi(btnRole,L"OK") )            { m_nRetID =IDOK    ; }
			else if( aux::wcseqi(btnRole,L"CANCEL") )   {  m_nRetID =IDCANCEL;}
			else if( aux::wcseqi(btnRole,L"ABORT") )    { m_nRetID =IDABORT ;}
			else if( aux::wcseqi(btnRole,L"RETRY") )    { m_nRetID =IDRETRY ; }
			else if( aux::wcseqi(btnRole,L"IGNORE") )    { m_nRetID =IDIGNORE;}
			else if( aux::wcseqi(btnRole,L"YES") )      { m_nRetID =IDYES   ; }
			else if( aux::wcseqi(btnRole,L"NO") )        { m_nRetID =IDNO    ;}
			else if( aux::wcseqi(btnRole,L"CLOSE") )     { m_nRetID =IDCLOSE ;}
			else if( aux::wcseqi(btnRole,L"HELP") )      { m_nRetID =IDHELP  ; }
			else 
			{ // 最大最小化按钮
				if( aux::wcseqi(btnRole,L"MINIMAZE") ) 
                    ShowWindow(SW_MINIMIZE);
				else if( aux::wcseqi(btnRole,L"MAXIMAZE") ) 
					ToggleFrameMaxium();
				return FALSE;
			}
            // 上级类可以重载OnDefaultButton来定制点击上面各种按钮的行为
            // 上级返回TRUE，则关闭并销毁窗口，返回FALSE，不做任何操作
			if (OnDefaultButton(m_nRetID)) 
            {// 关闭对话框
                ::PostMessage(m_hWnd, WM_CLOSE, 0,0 );
            }
		}
	}
	return QView::handle_event(he,params);
}

void QFrame::ToggleFrameMaxium()
{
    if (!(GetStyle() & WS_MAXIMIZEBOX))
        return;

// 	ECtrl btnMax = GetCtrl("#wc-caption table[sysbtn] td[xrole=\"MAXIMAZE\"]");
// 	if (!btnMax.is_valid())
// 		return;

	if (quibase::IsWindowMaximized(GetSafeHwnd()))
	{
//		btnMax.remove_attribute("maximazed");
		::ShowWindow(GetSafeHwnd(), SW_RESTORE);
	}
	else
	{
// 		btnMax.set_attribute("maximazed",L"true");
// 		if (btnMax.get_attribute("full-screen") != NULL)
// 		{
// 			// 只是为了设置 sw_showmaxiumzed 属性
// 			// 此属性可用GetWindowPlacement检测，
// 			// IsWindowMaximized据此判断窗口是否最大化了
// 			::ShowWindow(GetSafeHwnd(), SW_MAXIMIZE);
// 			// 覆盖到全屏
// 			quibase::MoveToFullScreen(GetSafeHwnd());
// 		}
// 		else
// 		{
// 			::ShowWindow(GetSafeHwnd(), SW_MAXIMIZE); 
// 		}
        ::ShowWindow(GetSafeHwnd(), SW_MAXIMIZE); 
	}
}

void QFrame::SetTitle( LPCWSTR pszTitle )
{
	if (ctl_title_.is_valid())
	{
		ctl_title_.SetText(pszTitle);
	}
	SetWindowText(pszTitle);
}

void QFrame::OnGetMinMaxInfo( LPMINMAXINFO pi )
{
	WTL::CRect   rt(0,0,0,0); 
    // 只有工作区，不包括任务栏部分
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rt, 0);
    
    pi->ptMaxSize.x = rt.Width();
    pi->ptMaxSize.y = rt.Height();
    pi->ptMinTrackSize.x = size_min_.cx;
    pi->ptMinTrackSize.y = size_min_.cy;
}

void QFrame::OnActive( UINT nState, BOOL bIsMinimazed, HWND hWnd )
{
	SetMsgHandled(FALSE);

	if (bIsMinimazed || !wnd_shadow_.IsValid())
		return ;

	if (WA_INACTIVE == nState)
	{
        SetShadowDarkness(120);
	}
	else
	{
        SetShadowDarkness(170);
	}
}

LRESULT QFrame::OnWmNCHittest( WTL::CPoint pt )
{
    const int FRAME_THICKNESS = 5;
    ScreenToClient(&pt);

    if (ctl_title_.is_valid() && ctl_title_.is_inside(pt) )
        return HTCAPTION;

//     if( button_icon.is_valid() && button_icon.is_inside(pt) )
//         return HTSYSMENU;

    if ( HasQEXStyle(WS_QEX_THICKFRAME) // 只有在具有可调风格下才去计算
        && !quibase::IsWindowMaximized(GetSafeHwnd()))  // 窗口最大化的时候不可手动调大小
    {
        // 窗口大小可以由用户改变
        CRect rc;
        GetClientRect(&rc);

        if( pt.y < rc.top + FRAME_THICKNESS ) 
        {
            if( pt.x < rc.left + FRAME_THICKNESS ) 
                return HTTOPLEFT;
            if( pt.x > rc.right - FRAME_THICKNESS ) 
                return HTTOPRIGHT;
        }
        else if( pt.y > rc.bottom - FRAME_THICKNESS ) 
        {
            if( pt.x < rc.left + FRAME_THICKNESS ) 
                return HTBOTTOMLEFT;
            if( pt.x > rc.right - FRAME_THICKNESS ) 
                return HTBOTTOMRIGHT;
        }

        if( pt.y <= rc.top + FRAME_THICKNESS) 
            return HTTOP;
        if( pt.y >= rc.bottom - FRAME_THICKNESS) 
            return HTBOTTOM;
        if( pt.x <= rc.left + FRAME_THICKNESS) 
            return HTLEFT;
        if( pt.x >= rc.right - FRAME_THICKNESS) 
            return HTRIGHT;
    }
    return HTCLIENT;
}

LRESULT QFrame::OnHtmlNotify( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch(((NMHDR*)lParam)->code) 
    {
    case HLN_DOCUMENT_COMPLETE:
        {
            if ( !_HasFlag(GetStyle(),WS_CHILD) )
            {
                WTL::CRect r;
                GetWindowRect(&r);

                // 非子窗口，调整窗体为合适大小
                int nWidth = max(HTMLayoutGetMinWidth(m_hWnd), (UINT)r.Width()); 
                WTL::CRect rcNew;
                rcNew.SetRect(r.left, r.top, 
                    nWidth + r.left, 
                    r.top + max((UINT)r.Height(), HTMLayoutGetMinHeight(m_hWnd,nWidth)));
                AdjustWindowRectEx( &rcNew, GetStyle(), FALSE, GetExStyle());

                // 调整大小，位置不变
                SetWindowPos(NULL, 0, 0, rcNew.Width(), rcNew.Height(), SWP_NOZORDER | SWP_NOMOVE);

                size_min_ = rcNew.Size();
                // 设置title
                ctl_title_ = GetCtrl("#wc-title");
                if (ctl_title_.is_valid())
                {
                    SetWindowText(ctl_title_.GetText());
                }
            }
            break;
        }
    }
    return QView::OnHtmlNotify(uMsg, wParam, lParam);
}

inline void QFrame::SetShadowSize( int NewSize /*= 0*/ )
{
    wnd_shadow_.SetSize(NewSize);
}

inline void QFrame::SetShadowSharpness( unsigned int NewSharpness /*= 5*/ )
{
    wnd_shadow_.SetSharpness(NewSharpness);
}

inline void QFrame::SetShadowColor( COLORREF NewColor /*= 0*/ )
{
    wnd_shadow_.SetColor(NewColor);
}

inline void QFrame::SetShadowPosition( int NewXOffset /*= 5*/, int NewYOffset /*= 5*/ )
{
    wnd_shadow_.SetPosition(NewXOffset,NewYOffset);
}

inline void QFrame::SetShadowDarkness( unsigned int NewDarkness /*= 200*/ )
{
    wnd_shadow_.SetDarkness(NewDarkness);
}
