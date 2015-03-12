#ifndef WndHelper_h__
#define WndHelper_h__

#pragma  once

#include "stdstring.h"
#include "BaseType.h"

namespace quibase
{
    /**
     *	获取屏幕大小	
     *
    **/
	inline SIZE GetScreenSize();

    /**
     *	窗口大小	
     *
    **/
	SIZE GetWindowSize(HWND hWnd);

    /**
     *	窗口宽、高、 客户区宽、高	
     *
    **/
	int WinWidth(HWND hWnd);
	int WinHeight(HWND hWnd);
	int ClientWidth(HWND hWnd);
	int ClientHeight(HWND hWnd);

    /**
     * 窗口是否最小化		
     *
    **/
    BOOL IsWindowMaximized( __in HWND hWnd);

    /**
     * 全屏显示窗口		
     *
    **/
    void MoveToFullScreen(__in HWND hWnd);

    /**
     * 将窗口提前到最前端显示		
     *
    **/
    void SetForegroundWindowInternal(HWND hWnd);

    /**
     * 将hWndMove显示在hWndIn的正中央		
     *
    **/
    void CenterWindowInWindow(HWND hWndMove,HWND hWndIn);

    /**
     *	使hWnd指定的窗口透明,	
     *
     *	@param
     *		-[in]
     *       uAlpha     透明度, [0,255], 0:完全透明,255:完全不透明
     *       uFlag  	LWA_COLORKEY|LWA_ALPHA
     *
    **/
	BOOL TransparentWindow(HWND hWnd,UINT uAlpha,
		COLORREF clr=0,UINT uFlag=LWA_ALPHA /*|LWA_COLORKEY*/);

    /**
     *	将指定窗口的矩形中心与指定点对齐(重合),不改变大小	
     *
     *	@return
     *		
     *	@param
     *		-[in]
     *          point ,窗口新的中心,注意是screen坐标,不是窗口坐标
     *
    **/
	BOOL CenterWindowPoint(HWND hWndMove,POINT point);

    /**
     *	检测当前光标是否在hWnd标示的窗口内
     *
    **/
	BOOL IsCursorInWindow(HWND hWnd);
	
    /**
     *	将窗口置于屏幕的右下角
     *
    **/
	void PosWindowAtBottomRightScreen(HWND hWnd);

    /**
     *	显示一个显示Folder Select	
     *
     *	@param
     *		-[out]
     *      sFolder     如果选择了Ok,包含了返回的文件夹路径
    **/
	BOOL ShowFolderSelectBox(__out CStdString &sFolder);

    /** 为指定窗口画上一个边框
     *	param
     *		-[in]
     *          hWnd        需要高亮显示的窗口
    **/
    void HighlightWindow(HWND hWnd, int nBorderWidth = 3);

    /**
     * win7 桌面listview的父亲窗口句柄
     *  类名： SHELLDLL_DefView
    **/
    HWND GetShellDLLForWin7();

    /**
     *	检查并试图把桌面设为自己的父窗口，返回为原父窗口
     *
    **/
    HWND CheckParent(HWND hWnd); 

    /**
     *	跟据进行和窗口名查找窗口
     *
    **/
    HWND GetProcessHwnd(LPCTSTR lpszProcess, LPCTSTR lspzCaption); //

};


#endif // WndHelper_h__

