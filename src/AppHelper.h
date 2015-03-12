#ifndef apphelper_h__
#define apphelper_h__

#pragma once

#include <sstream>
#include "stdstring.h"

namespace quibase
{
    /**
     *		
     * 每两个字符转换到一个字节的16进制数,
     * 
     *  不能有0x前导,合法字符为 0 ~ 9， A~F ,a ~ f
     *  字符串总厂应该是偶数 "FF0355" - FF0355
     *
    **/
	BOOL StrToHex(LPCTSTR pStr , BYTE val[]);

    /**
     *	文件是否存在	
     *
    **/
    BOOL IsFileExist(LPCTSTR pszFile);

    /**
     *	统计字符串pszStr中字符ch出现的次数	
     *
    **/
	int CountChar(__in LPCTSTR pszStr,__in TCHAR ch);
	
    /**
     *	获取当前程序运行路径 ， 以 '/' 结尾	
     *
    **/
    CStdString GetModulePath();

    /**
     *	获取当前程序名字，无路径	
     *
     *      bWithExt     TRUE        带上扩展名 .exe
    **/
	CStdString GetModuleName(BOOL bWithExt = FALSE);

    /**
     *	错误消息格式化函数	
     *
    **/
	CStdString SysErrorMessage(DWORD dwLastError );

    /** 关机/重启系统
     *	return:
     *      TRUE    成功
     *	params:
     *		-[in]
     *          bReboot     TRUE 重启
     *                      FALSE 关机
     *          pszMsg      对用户显示的消息
     *                      最大长度是3000个字符
     *          dwTimeout   对用户显示的对话框自动关闭的超时时间
     *                      单位秒（s）。
     *                      0   不显示用户对话框，直接关闭
     *                      (0,MAX_SHUTDOWN_TIMEOUT ]   显示的时间
     *          bAskUserCloseApp  提示用户保存数据
     *                          FALSE 将会强制关闭应用程序。有可能导致数据丢失
    **/
    BOOL ShutdownComputer( __in BOOL bReboot, __in LPWSTR pszMsg, 
        __in DWORD dwTimeout = 30,__in BOOL bAskUserCloseApp=TRUE );

    /**
     *		
     * 取消关机
    **/
    BOOL PreventSystemShutdown();

	/** 屏幕上两点间的距离
	 *	-return:		两点间距
	 *	-params:	
	 *		-[in]	p1,p2
	 **/
	inline int DistanceOfPoint(POINT &p1,POINT &p2)
	{
		return (int)sqrt(pow((float)abs(p1.x - p2.x),2) + pow((float)abs(p1.y - p2.y),2));
	}

    /**
     *	复制文本到剪贴板	
     *
     *	@param
     *		-[in]
     *      hwnd        Handle to the window to be associated with the open clipboard. 
     *                  If this parameter is NULL, 
     *                  the open clipboard is associated with the current task. 
    **/
	void CopyTexttoClipboard(const CStdString& sText, HWND hwnd = NULL); 

    /**
     *	读取剪贴板上的文本	
     *
    **/
	CStdString GetClipboardText(HWND hwnd = NULL); 

    /**  获取特殊路径
     *	return
     *      TRUE    获取成功
     *	param
     *		-[in]
     *          csidl       例如：CSIDL_STARTUP
     *		-[out]
     *			sPath       获取到的路径
    **/
    BOOL GetSpeialPath(__in int csidl, __out CStdString &sPath);

    /**
     *	提升权限	
     *
    **/
    BOOL UpgradeProcessPrivilege(); 
    
    /**
     *	把资源文件释放出来	
     *
    **/ 
    BOOL ReleaseRes(LPCTSTR filename,WORD wResID, LPCTSTR filetype);  

};

class QMutex 
{ 
	CRITICAL_SECTION cs;
public:
	void Lock()     { EnterCriticalSection(&cs); } 
	void Unlock()   { LeaveCriticalSection(&cs); } 
	QMutex()         { InitializeCriticalSection(&cs); }   
	~QMutex()        { DeleteCriticalSection(&cs); }
};

#endif // apphelper_h__
