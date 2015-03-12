#include "stdafx.h"
#include "QDialog.h"

void QDialog::EndDialog( INT_PTR nResult )
{
    PostMessage(WM_CLOSE);
    
    m_nRetID = nResult;

    msg_loop_.EndRun();
}

void QDialog::OnDestroy()
{
	SetMsgHandled(FALSE);

    if (msg_loop_.IsRunning())
    {
        msg_loop_.EndRun();
    }
}

INT_PTR QDialog::DoModal( HWND hWndParent, UINT nStyleEx ,
    UINT nDecoratedStyle, LPCRECT rcWnd)
{
    // 代码复制自MFC::domodal
    // cannot call DoModal on a dialog already constructed as modeless
    ATLASSERT(m_hWnd == NULL);
    HWND hWndTop = NULL;

    // get window to start with
    QUIGetMainWnd();
    if (!::IsWindow(hWndParent))
    {
        hWndParent = GetActiveWindow(); /*QUIGetMainWnd();*/

        // determine toplevel window to disable as well
        hWndTop = hWndParent;
        HWND hWndTemp = hWndParent;
        while (true)
        {
            if (hWndTemp == NULL)
            {
                hWndParent = QUIGetMainWnd();
                break;
            }
            hWndTop = hWndTemp;
            hWndTemp = ::GetParent(hWndTop);
        }
        // get last active popup of first non-child that was found
        if (hWndParent != NULL)
        {
            hWndParent = ::GetLastActivePopup(hWndParent);
        }
    }

    // disable top level parent window if specified
    BOOL bRestoreTopWnd = FALSE;
    if ((hWndTop != NULL) && ::IsWindowEnabled(hWndTop) && (hWndTop != hWndParent))
    {
        bRestoreTopWnd = TRUE;
        ::EnableWindow(hWndTop, FALSE);
    }

    BOOL bRestoreParentWnd = FALSE;
    if (hWndParent && (hWndParent != ::GetDesktopWindow()) && ::IsWindowEnabled(hWndParent))
    {
        bRestoreParentWnd = TRUE;
        ::EnableWindow(hWndParent, FALSE);
    }

    // 创建窗口
    if (Create(hWndParent,WS_POPUP|WS_VISIBLE,nStyleEx,nDecoratedStyle))
    {
        // 将本窗口置为前台窗口
        SetForegroundWindow(m_hWnd);

        if (NULL == rcWnd)
        {
            CenterWindow(hWndParent);
        }

        // 开始模式对话框消息循环
        msg_loop_.Run();
    }
#ifdef _DEBUG
    else
    {
        ATLASSERT(FALSE);
    }
#endif

    // 窗口应该已经被销毁掉了。
    if (::IsWindow(GetSafeHwnd()))
    {
        DestroyWindow();
    }

    // 恢复原来状态
    if (bRestoreParentWnd)
    {
        ::EnableWindow(hWndParent, TRUE);
        if ((hWndParent != NULL) && (::GetActiveWindow() == m_hWnd))
        {
            ::SetActiveWindow(hWndParent);
        }
    }

    if (bRestoreTopWnd)
    {
        ::EnableWindow(hWndTop, TRUE);
    }

    return m_nRetID;
}


