#ifndef autohidewnd_h__
#define autohidewnd_h__

#pragma once

#include <map>

// ------------------------------------------------
// FOR	:	自动隐藏窗口代码
//      1> 在需要自动隐藏的窗口创建后调用RegisterAutoHide
//		2> 在窗口的
//              WM_ENTERSIZEMOVE/WM_EXITSIZEMOVE/WM_MOUSEMOVE消息处理时
//          调用 AutoHideWndProc(m_hWnd,message,wParam,lParam);
// ------------------------------------------------
class CAutoHideWnd
{
public:
    enum ENUM_WND_ALIGNMENT
    {
        ALIGN_NONE = 0x0,                     //不停靠
        ALIGN_TOP = 0x1,                     //停靠上边
        ALIGN_LEFT = 0x2,                     //停靠左边
        ALIGN_RIGHT = 0x4,                     //停靠右边
        ALIGN_ALL = 0xFF,  //所有的边都可以停靠 
    };

private:
    typedef struct AHW_DATA 
    {
        AHW_DATA(HWND h)
        {
            hWnd = h;
            bMoving = FALSE;
            bHided = FALSE;
            nNearSize = 10;
            nAlign = ALIGN_ALL;
            eAlignType = ALIGN_NONE;
            nWndExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
        }
        HWND	hWnd;
        ENUM_WND_ALIGNMENT	    eAlignType;       //用于记录窗体停靠状态
        int	    nNearSize;		// 靠近边界多少像素时隐藏
        int	    nSide;		// 边界大小
        UINT    nAlign;     // 可停靠的边界
        BOOL	bMoving;		// 正在调整大小中
        UINT    nWndExStyle;    // 在隐藏的时候，为其设置WS_EX_TOPMOST风格，显示的时候再恢复到这个风格
        BOOL    bHided;     // 是否已经隐藏了
    }*LPAHW_DATA;

    typedef std::map<HWND , AHW_DATA>	MAP_AHW;
    typedef MAP_AHW::iterator	MapAhwItr;
    typedef std::pair<HWND, AHW_DATA> PairAHW;

//     SINGLETON_ON_DESTRUCTOR(CAutoHideWnd)
//     {
//         if (NULL != m_hTimer)
//         {
//             ::DeleteTimerQueueTimer(NULL,m_hTimer,NULL);
//         }
//     }
    
public:
    CAutoHideWnd();
    ~CAutoHideWnd();

public:
	// 当靠近边界多少像素时候隐藏
	void RegisterAutoHide(HWND hWnd, UINT nAlign = ALIGN_ALL, 
        int nPixelAutoHide=10, int nPixelSide=4);

    void Unregister(HWND hWnd);

    BOOL ProcessWindowMessage(
        __in HWND hWnd,
        __in UINT uMsg,
        __in WPARAM wParam,
        __in LPARAM lParam,
        _Inout_ LRESULT& lResult);

    inline void DontDetectCur()
    {
        if (nullptr != m_pCurAHW)

        {
            m_pCurAHW->bMoving = TRUE;

        }
    }

protected:
    inline BOOL IsWndHided(LPAHW_DATA pAHW)const
    {
        ATLASSERT(NULL != pAHW);
        return pAHW->bHided;
    }
	
    static VOID CALLBACK AHWTimerCallback(PVOID lpParameter,BOOLEAN TimerOrWaitFired);

	inline LPAHW_DATA FindAHW(HWND hWnd)
	{
		MapAhwItr itr = m_MapAhw.find(hWnd);
		return  (m_MapAhw.end() != itr) ? (&(itr->second)) : NULL;
	}

    /** 检测最靠近那个边界
     *	return:
     *      FALSE   不靠近任何边界
     *	params:
     *		-[in]
     *          pAHW        要检测的窗口
    **/
	BOOL DetectWhichSideIsNearset(__in LPAHW_DATA pAHW);

    /** 隐藏/显示 注册的窗口
     *	params:
     *		-[in]
     *          pAHW    要隐藏的窗口
     *          bHide   TRUE，隐藏
    **/
	void  HideWnd(__in LPAHW_DATA pAHW,   __in BOOL   bHide);
private:
    MAP_AHW         m_MapAhw;
    HANDLE	        m_hTimer;
    LPAHW_DATA      m_pCurAHW;
    int	            m_nScreenWidth;
};


#endif // apphelper_h__
