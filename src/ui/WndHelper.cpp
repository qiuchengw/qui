#include "stdafx.h"
#include "WndHelper.h"
#include <atlmisc.h>
#include <ShlObj.h>
#include "autohidewnd.h"

namespace quibase
{
	SIZE GetScreenSize()
	{
		HDC hDC = ::GetDC(NULL);
		SIZE sz;
		sz.cx = ::GetDeviceCaps(hDC,HORZRES);
		sz.cy = ::GetDeviceCaps(hDC,VERTRES);
		::ReleaseDC(NULL,hDC);
		return sz;
	}
    
	int WinWidth(HWND hWnd)
	{
		RECT rt;
		GetWindowRect(hWnd,&rt);

		return (rt.right-rt.left);
	}

	int WinHeight(HWND hWnd)
	{
		RECT rt;
		GetWindowRect(hWnd,&rt);

		return (rt.bottom-rt.top);
	}

	int ClientWidth(HWND hWnd)
	{
		RECT rt;
		GetClientRect(hWnd,&rt);

		return rt.right;
	}

	int ClientHeight(HWND hWnd)
	{
		RECT rt;
		GetClientRect(hWnd,&rt);

		return rt.bottom;
	}

	SIZE GetWindowSize(HWND hWnd)
	{
		SIZE sz = {0,0};
		if ((hWnd != NULL) && ::IsWindow(hWnd))
		{
			RECT rect;
			GetWindowRect(hWnd,&rect);
			sz.cx = rect.right - rect.left;
			sz.cy = rect.bottom - rect.top;
		}
		return sz;
	}

	BOOL TransparentWindow(HWND hWnd,UINT uAlpha,COLORREF clrTrans,UINT uFlag)
	{
		// 使要透明的窗口具有 相应的透明属性
		if (hWnd == NULL || !::IsWindow(hWnd))
			return FALSE;
		LONG dwStyleEx = ::GetWindowLong(hWnd,GWL_EXSTYLE);
        if (!(dwStyleEx & WS_EX_LAYERED))
        {
		    dwStyleEx |= WS_EX_LAYERED;
		    ::SetWindowLong(hWnd,GWL_EXSTYLE,dwStyleEx);
        }

		// 使之透明
		if (uAlpha > 255) 
            uAlpha = 255;


		typedef BOOL (WINAPI *pLayeredWindow)(HWND ,COLORREF,BYTE,DWORD);
		HMODULE hUser32Dll = LoadLibrary(_T("user32.dll"));
		if (hUser32Dll != NULL)
		{
            pLayeredWindow SetLWA =        
                (pLayeredWindow)GetProcAddress(hUser32Dll, "SetLayeredWindowAttributes");
            if (SetLWA != NULL)
            {
                SetLWA(hWnd,clrTrans,uAlpha,uFlag);
                ::RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN); 

                FreeLibrary(hUser32Dll);

                return TRUE;
            }
		}
        return FALSE;
	}
	
	// 保存重建窗口位置
	#ifndef INI_PROGRAM
	#   define INI_PROGRAM _T("qsoft_program")
	#endif

	#pragma warning(disable : 4996)	// disable bogus deprecation warning

	//---------------------------------------------------------------
    void MoveToFullScreen(__in HWND hWnd)
    {
        HWND hDesk = ::GetDesktopWindow();
        RECT rc;
        GetClientRect(hDesk,&rc);
        MoveWindow(hWnd,0,0,rc.right,rc.bottom,FALSE);
    }

    BOOL IsWindowMaximized( __in HWND hWnd )
    {
        WINDOWPLACEMENT wp;
        ZeroMemory(&wp,sizeof(wp));
        if (GetWindowPlacement(hWnd,&wp))
        {
            return (wp.showCmd == SW_SHOWMAXIMIZED);
        }
        return FALSE;
    }

    void CenterWindowInWindow(HWND hWndMove,HWND hWndIn)
    {
        ATLASSERT(::IsWindow(hWndMove));

        // determine owner window to center against
        DWORD dwStyle = ((DWORD)GetWindowLong(hWndMove, GWL_STYLE));
        if ( !::IsWindow(hWndIn) )
        {
            if (dwStyle & WS_CHILD)
                hWndIn = ::GetParent(hWndMove);
            else
                hWndIn = ::GetWindow(hWndMove, GW_OWNER);
            if (NULL == hWndIn )
            {
                hWndIn = GetDesktopWindow();
            }
        }

        // get coordinates of the window relative to its parent
        CRect rcDlg;
        GetWindowRect(hWndMove, &rcDlg);
        CRect rcArea;
        CRect rcCenter;
        HWND hWndParent;
        if (!(dwStyle & WS_CHILD))
        {
            // don't center against invisible or minimized windows
            if (hWndIn != NULL)
            {
                DWORD dwAlternateStyle = ((DWORD)GetWindowLong(hWndIn, GWL_STYLE));
                if (!(dwAlternateStyle & WS_VISIBLE) || (dwAlternateStyle & WS_MINIMIZE))
                    hWndIn = NULL;
            }

            MONITORINFO mi;
            mi.cbSize = sizeof(mi);

            // center within appropriate monitor coordinates
            if (hWndIn == NULL)
            {
                GetMonitorInfo(
                    MonitorFromWindow(hWndMove, MONITOR_DEFAULTTOPRIMARY), &mi);
                rcCenter = mi.rcWork;
                rcArea = mi.rcWork;
            }
            else
            {
                ::GetWindowRect(hWndIn, &rcCenter);
                GetMonitorInfo(
                    MonitorFromWindow(hWndIn, MONITOR_DEFAULTTONEAREST), &mi);
                rcArea = mi.rcWork;
            }
        }
        else
        {
            // center within parent client coordinates
            hWndParent = ::GetParent(hWndMove);
            ATLASSERT(::IsWindow(hWndParent));

            ::GetClientRect(hWndParent, &rcArea);
            ATLASSERT(::IsWindow(hWndIn));
            ::GetClientRect(hWndIn, &rcCenter);
            ::MapWindowPoints(hWndIn, hWndParent, (POINT*)&rcCenter, 2);
        }

        // find dialog's upper left based on rcCenter
        int xLeft = (rcCenter.left + rcCenter.right) / 2 - rcDlg.Width() / 2;
        int yTop = (rcCenter.top + rcCenter.bottom) / 2 - rcDlg.Height() / 2;

        // if the dialog is outside the screen, move it inside
        if (xLeft + rcDlg.Width() > rcArea.right)
            xLeft = rcArea.right - rcDlg.Width();
        if (xLeft < rcArea.left)
            xLeft = rcArea.left;

        if (yTop + rcDlg.Height() > rcArea.bottom)
            yTop = rcArea.bottom - rcDlg.Height();
        if (yTop < rcArea.top)
            yTop = rcArea.top;

        // map screen coordinates to child coordinates
        ::SetWindowPos(hWndMove,NULL, xLeft, yTop, -1, -1,
            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void SetForegroundWindowInternal(HWND hWnd)
    {
        if(!::IsWindow(hWnd)) return;

        //relation time of SetForegroundWindow lock
        DWORD lockTimeOut = 0;
        HWND  hCurrWnd = ::GetForegroundWindow();
        DWORD dwThisTID = ::GetCurrentThreadId(),
            dwCurrTID = ::GetWindowThreadProcessId(hCurrWnd,0);

        //we need to bypass some limitations from Microsoft :)
        if(dwThisTID != dwCurrTID)
        {
            ::AttachThreadInput(dwThisTID, dwCurrTID, TRUE);

            ::SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT,0,&lockTimeOut,0);
            ::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,0,0,
                SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);

            ::AllowSetForegroundWindow(ASFW_ANY);
        }

        ::SetForegroundWindow(hWnd);

        LONG dwExStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
        if (!(dwExStyle & WS_EX_TOPMOST))
        {
            ::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
            ::SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        }

        if(dwThisTID != dwCurrTID)
        {
            ::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,0,
                (PVOID)lockTimeOut,SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);

            ::AttachThreadInput(dwThisTID, dwCurrTID, FALSE);
        }
    }
	//---------------------------------------------------------------
	BOOL CenterWindowPoint(HWND hWndMove,CPoint point)
	{
		ATLASSERT(::IsWindow(hWndMove));
		CRect rect;
		::GetWindowRect(hWndMove,&rect);
		int height = rect.Height();
		int width = rect.Width();
		CPoint ptOffset = point-rect.CenterPoint();
		rect.OffsetRect(ptOffset);
		return ::MoveWindow(hWndMove,rect.left,rect.top,
				width,height,FALSE);
	}
	//---------------------------------------------------------------
	BOOL IsCursorInWindow(HWND hWnd)
	{
		CPoint point;
		GetCursorPos(&point);
		CRect rect;
		GetWindowRect(hWnd,&rect);
		return rect.PtInRect(point);
	}
	//---------------------------------------------------------------
	void PosWindowAtBottomRightScreen(HWND hWnd)
	{
		ATLASSERT(::IsWindow(hWnd));
		CRect rect;
		GetClientRect(hWnd,&rect);
		int nX = rect.Width();
		int nY = rect.Height();
		CSize sz = GetScreenSize();
		int X = sz.cx-nX;
		int Y = sz.cy-nY;
		SetWindowPos(hWnd,HWND_NOTOPMOST,X,Y,nX,nY,
			SWP_NOREDRAW|SWP_NOSIZE|SWP_NOZORDER);
	}
	//---------------------------------------------------------------
	BOOL ShowFolderSelectBox(CStdString &sFolder)
	{
		BROWSEINFO bi;
		ZeroMemory(&bi,sizeof(BROWSEINFO));
		bi.ulFlags = BIF_USENEWUI;
		LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
		TCHAR path[MAX_PATH];
		if(pidl != NULL)
		{// pressed ok button
			SHGetPathFromIDList(pidl,path);
			sFolder = path;
			// free mem
			CoTaskMemFree(pidl);
			return TRUE;
		}
		// cancel select
		return FALSE;
	}

    void HighlightWindow(HWND hWnd, int nBorderWidth/* = 3*/)
    {
        // Get coordinates
        RECT rect;   
        ::GetWindowRect(hWnd, &rect);

        // Setup to draw
        HDC hDC = ::GetWindowDC(hWnd);
        if(!hDC)
            return;
        HPEN hPen = ::CreatePen(PS_SOLID, nBorderWidth, RGB(255, 0, 0));
        HGDIOBJ hPrevPen = ::SelectObject(hDC, hPen);
        HGDIOBJ hPrevBrush = ::SelectObject(hDC, ::GetStockObject(HOLLOW_BRUSH));

        // Draw highlight
        ::Rectangle(hDC, 0, 0, (rect.right - rect.left), (rect.bottom - rect.top));

        // Clean up
        ::SelectObject(hDC, hPrevPen);
        ::SelectObject(hDC, hPrevBrush);
        ::DeleteObject(hPen);
        ::ReleaseDC(hWnd, hDC);
    }

    HWND GetShellDLLForWin7()
    {
        HWND hWndShell = NULL;
        HWND hWndDesktop = GetDesktopWindow();
        if(hWndDesktop == NULL) return NULL;

        HWND hWndWorkerW = NULL;
        while(TRUE)
        {
            hWndWorkerW = FindWindowEx(hWndDesktop, hWndWorkerW, _T("WorkerW"), NULL);
            if(hWndWorkerW == NULL) break;

            if(::GetWindowLong(hWndWorkerW, GWL_STYLE) & WS_VISIBLE)
            {
                break;
            }
            else
            {
                continue;
            }
        }

        if(hWndWorkerW != NULL)
        {
            hWndShell = FindWindowEx(hWndWorkerW, NULL, _T("SHELLDLL_DefView"), NULL); 
        }

        return hWndShell;
    }

    HWND CheckParent(HWND hWnd)
    {
        HWND hWndOldParent = NULL;

        HWND hWndProgram = NULL;
        HWND hWndShellDLL = NULL;
        hWndProgram = FindWindow(_T("Progman"), _T("Program Manager"));
        if(hWndProgram != NULL)
        {
            hWndShellDLL = FindWindowEx(hWndProgram, NULL, _T("SHELLDLL_DefView"), NULL);

            //Win7
            if(hWndShellDLL == NULL)
            {
                hWndShellDLL = GetShellDLLForWin7();
            }
        }

        if(hWndShellDLL != NULL
            && hWndShellDLL != hWndOldParent)
        {
            SetWindowLong(hWnd, GWL_HWNDPARENT, (LONG)hWndShellDLL);
            hWndOldParent = hWndShellDLL;
        }

        return hWndOldParent;
    }

    //     HWND GetProcessHwnd(LPCTSTR lpszProcess, LPCTSTR lspzCaption)
    //     {
    //         HWND hWnd = NULL;
    //         DWORD dwProcs[1024];
    //         DWORD dwNeeded;	
    //         //枚举所有进程ID。
    //         if (!EnumProcesses(dwProcs, sizeof(dwProcs), &dwNeeded))
    //             return hWnd;
    //         // 计算有多少个进程ID。
    //         DWORD dwProcCount = dwNeeded/sizeof(DWORD);
    //         //遍历所有进程ID，打开进程。
    //         for (DWORD i=0; i<dwProcCount; i++)
    //         {
    //             HMODULE hModule;
    //             DWORD needed;
    //             TCHAR processName[MAX_PATH] = {0};
    //             //根据进程ID打开进程。
    //             HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcs[i]);
    //             if(hProcess)
    //             {
    //                 if(!EnumProcessModules(hProcess, &hModule, sizeof(hModule), &needed))
    //                     CloseHandle(hProcess);
    //                 GetModuleBaseName(hProcess, hModule, processName, sizeof(processName));
    //                 if(_tcscmp(processName, lpszProcess) == 0)
    //                 {
    //                     hWnd = FindWindow(NULL, lspzCaption);
    //                     break;
    //                 }
    //             }
    //         }
    //         return hWnd;
    //     }

    // callback function called by EnumDisplayMonitors for each enabled monitor  
    BOOL CALLBACK EnumDispProc(HMONITOR hMon, HDC dcMon, RECT* rc, LPARAM lParam)
    {
        std::map < HMONITOR, RECT > *pArg = (std::map < HMONITOR, RECT >*)(lParam);
        pArg->insert({ hMon, *rc });
        return TRUE;
    }

    BOOL GetMonitors(__out std::map<HMONITOR, RECT>& mons)
    {
        return EnumDisplayMonitors(0, 0, EnumDispProc, reinterpret_cast<LPARAM>(&mons));
    }
};

//////////////////////////////////////////////////////////////////////////

CAutoHideWnd::CAutoHideWnd()
{
    std::map<HMONITOR, RECT> mons;
    if (quibase::GetMonitors(mons))
    {
        for (auto i : mons)
        {
            m_nScreenWidth = max(i.second.right, m_nScreenWidth);
        }
    }
    else
    {
        m_nScreenWidth = quibase::GetScreenSize().cx;
    }
    m_pCurAHW = nullptr;

    // create the queue timer
    CreateTimerQueueTimer(&m_hTimer,NULL,
        &CAutoHideWnd::AHWTimerCallback,
        this,1200,1200,WT_EXECUTEDEFAULT);
}

CAutoHideWnd::~CAutoHideWnd()
{
    if (NULL != m_hTimer)
    {
        ::DeleteTimerQueueTimer(NULL,m_hTimer,NULL);
    }
}

void CAutoHideWnd::RegisterAutoHide( HWND hWnd, UINT nAlign /*= ALIGN_ALL*/, 
    int nPixelAutoHide/*=10*/, int nPixelSide/*=4*/ )
{
    if (!IsWindow(hWnd) || (GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD))
    {
        ATLASSERT(FALSE);
        return;
    }
    LPAHW_DATA pAHW = FindAHW(hWnd);
    if (NULL == pAHW)
    {
        AHW_DATA ad(hWnd);
        ad.nNearSize = nPixelAutoHide;
        ad.nSide = nPixelSide;
        ad.nAlign = nAlign;
        m_MapAhw.insert(PairAHW(hWnd,ad));
    }
    else
    {
        ATLASSERT(pAHW->hWnd == hWnd);
        pAHW->nNearSize = nPixelAutoHide;
        pAHW->nSide = nPixelSide;
        pAHW->nWndOldExStyle = GetWindowLong(pAHW->hWnd, GWL_EXSTYLE);
        pAHW->nAlign = nAlign;
    }
}

void CAutoHideWnd::Unregister(HWND hWnd) 
{
    m_MapAhw.erase(hWnd);




    if (NULL != m_pCurAHW)

    {
        if (m_pCurAHW->hWnd == hWnd)



        {
            m_pCurAHW = NULL;








        }
    }
}

VOID CALLBACK CAutoHideWnd::AHWTimerCallback( PVOID lpParameter,BOOLEAN TimerOrWaitFired )
{
    CAutoHideWnd* pThis = reinterpret_cast<CAutoHideWnd*>(lpParameter);
    LPAHW_DATA p = pThis->m_pCurAHW;
    if ((NULL != p) && !(p->bMoving))
    {
        if (pThis->IsWndHided(p))
        {
            if (quibase::IsCursorInWindow(pThis->m_pCurAHW->hWnd))
            {
                pThis->HideWnd(p, FALSE);
            }
        }
        else
        {
            if (!quibase::IsCursorInWindow(pThis->m_pCurAHW->hWnd))
            {
                pThis->HideWnd(p, TRUE);
            }
        }
    }
}

BOOL CAutoHideWnd::DetectWhichSideIsNearset( LPAHW_DATA pAHW )
{
    if (NULL == pAHW)
        return FALSE;

    RECT rect;
    GetWindowRect(pAHW->hWnd,   &rect);
    pAHW->eAlignType   =   ALIGN_NONE;
    if   (rect.left   <   pAHW->nNearSize)
    {
        if (pAHW->nAlign & ALIGN_LEFT) 
        {
            pAHW->eAlignType   =   ALIGN_LEFT;
        }
    }
    else   if   (rect.right   >   m_nScreenWidth   -   pAHW->nNearSize)
    {
        if (pAHW->nAlign & ALIGN_RIGHT) 
        {
            pAHW->eAlignType   =   ALIGN_RIGHT;
        }
    }
    else   if   (rect.top   <   pAHW->nNearSize)
    {
        if (pAHW->nAlign & ALIGN_TOP) 
        {
            pAHW->eAlignType   =   ALIGN_TOP;
        }
    }
    return pAHW->eAlignType != ALIGN_NONE;
}

void CAutoHideWnd::HideWnd( LPAHW_DATA pAHW, BOOL bHide )
{
    pAHW->bMoving = TRUE;
    if ( !DetectWhichSideIsNearset(pAHW) )
    {
        pAHW->bMoving = FALSE;
        return ;
    }

    //动画滚动窗体的步数, 如果你觉得不够平滑,可以增大该值.
    const int STEPS_OF_MOVE   =   1;
    int xStep,   yStep;
    int xEnd,   yEnd;
    RECT rcWnd;
    GetWindowRect(pAHW->hWnd, &rcWnd);
    int nWidth   =   rcWnd.right   -   rcWnd.left;
    int nHeight   =   rcWnd.bottom   -   rcWnd.top;
    //下边判断窗体该如何移动,由停靠方式决定
    switch (pAHW->eAlignType)
    {
    case   ALIGN_TOP:
        {
            //向上移藏
            xStep   =   0;
            xEnd   =   rcWnd.left;
            if   (bHide)
            {
                yStep   =   -rcWnd.bottom   /   STEPS_OF_MOVE;
                yEnd   =   -nHeight   +   pAHW->nSide;
            }
            else
            {
                yStep   =   -rcWnd.top   /   STEPS_OF_MOVE;
                yEnd   =   0;
            }
            break;
        }
    case   ALIGN_LEFT:
        {
            //向左移藏
            yStep   =   0;
            yEnd   =   rcWnd.top;
            if   (bHide)
            {
                xStep   =   -rcWnd.right   /   STEPS_OF_MOVE;
                xEnd   =   -nWidth   +   pAHW->nSide;
            }
            else
            {
                xStep   =   -rcWnd.left   /   STEPS_OF_MOVE;
                xEnd   =   0;
            }
            break;
        }
    case   ALIGN_RIGHT:
        {
            //向右移藏
            yStep   =   0;
            yEnd   =   rcWnd.top;
            if   (bHide)
            {
                xStep   =   (m_nScreenWidth   -   rcWnd.left)   /   STEPS_OF_MOVE;
                xEnd   =   m_nScreenWidth   -   pAHW->nSide;
            }
            else
            {
                xStep   =   (m_nScreenWidth   -   rcWnd.right)   /   STEPS_OF_MOVE;
                xEnd   =   m_nScreenWidth   -   nWidth;
            }
            break;
        }
    default:
        return;
    }

    //动画滚动窗体.
    HWND hInsertAfter = NULL; 
    if ( bHide )
    {
        pAHW->nWndOldExStyle = GetWindowLong(pAHW->hWnd, GWL_EXSTYLE);
        // 隐藏的时候让它成为最顶层窗口
        hInsertAfter = HWND_TOPMOST;
    }
    else
    {   
        // 显示的时候恢复其原来的窗口属性
        hInsertAfter = ((pAHW->nWndOldExStyle) & WS_EX_TOPMOST) ? HWND_TOPMOST : NULL;
        if (!(pAHW->nWndOldExStyle & WS_EX_TOPMOST))
        {
            SetWindowLong(pAHW->hWnd, GWL_EXSTYLE, pAHW->nWndOldExStyle);
        }
    }

    for (int i = 0; i < STEPS_OF_MOVE; i++)
    {
        rcWnd.left += xStep;
        rcWnd.top += yStep;
        SetWindowPos(pAHW->hWnd, hInsertAfter, rcWnd.left, rcWnd.top, 0, 0,
            SWP_NOSIZE|SWP_NOSENDCHANGING);
        InvalidateRect(pAHW->hWnd, NULL, TRUE);
        Sleep(15);
    }
    SetWindowPos(pAHW->hWnd,   hInsertAfter,   xEnd,   yEnd,   0,   0,   SWP_NOSIZE);
    SetForegroundWindow(pAHW->hWnd);
    pAHW->bHided = bHide;
    pAHW->bMoving = FALSE;

    // 发送notify消息
    PostMessage(pAHW->hWnd, MSG_AUTOHIDEWND_WNDVISIBLECHGED, 0, (LPARAM)!bHide);
}

BOOL CAutoHideWnd::ProcessWindowMessage(__in HWND hWnd, __in UINT uMsg, __in WPARAM wParam,
    __in LPARAM lParam, _Inout_ LRESULT& lResult) 
{
    UNREFERENCED_PARAMETER(lParam);
    if (NULL == (m_pCurAHW = FindAHW(hWnd)))
        return FALSE;

    switch (uMsg)
    {
    case   WM_ENTERSIZEMOVE: { m_pCurAHW->bMoving = TRUE; break; }
    case   WM_EXITSIZEMOVE: { m_pCurAHW->bMoving = FALSE; break; }
    case   WM_MOUSEMOVE:   //受到窗体移动消息时,判断窗体是否显示,
        {
            // 显示窗口
            RECT rc;
            GetWindowRect(hWnd,   &rc);
            if   (rc.left < 0 || rc.top < 0 || rc.right > m_nScreenWidth)   //未显示
            {   
                HideWnd(m_pCurAHW, FALSE);
            }
            break;
        }
    }
    lResult = 0;
    return FALSE;
}
//////////////////////////////////////////////////////////////////////////

