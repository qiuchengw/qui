#ifndef QUIGlobal_h__
#define QUIGlobal_h__

#pragma once

// // #include "QUI.h"
#include "BaseType.h"
#include "stdstring.h"
#include "htmlayout/htmlayout.h"
#include "htmlayout/htmlayout_dom.h"

// 自定义的QUI消息
// wParam 被用作子消息号码
DECLARE_USER_MESSAGE(UIBASE_MSG_CODEMSG);

// 系统右下角通知区域消息
DECLARE_USER_MESSAGE(UIBASE_MSG_SYSTRAY);

class QUIConfig;
class QApp;
class QView;

// 获取主程序实例指针
//      !!! 不要内联，指针变量是不暴露的
QApp* QUIGetApp();

// 获取应用程序名
CStdString QUIGetAppName();

#ifndef APP_NAME
#define APP_NAME QUIGetAppName()
#endif

// 获取配置文件信息
QUIConfig* QUIGetConfig();

// 应用程序是否已经退出了主消息循环
//      (进程即将退出)
BOOL QUIIsQuiting();

////////////////////////////////
// 凡是被内联的，都不能是DLL导出函数
// 获取主窗口句柄
inline HWND QUIGetMainWnd();

// 获取应用程序句柄
inline HINSTANCE QUIGetInstance();
inline HINSTANCE QUIGetResourceInstance();

// 获取版本号
inline CStdString QUIGetAppVersion();

// 应用程序消息相关的消息
inline LRESULT QUISendCodeTo( HWND hWnd, int nCode, LPARAM lParam)
{
    return SendMessage(hWnd,UIBASE_MSG_CODEMSG,nCode,lParam); 
}

inline BOOL QUIPostCodeTo( HWND hWnd, int nCode, LPARAM lParam)
{
    return PostMessage(hWnd,UIBASE_MSG_CODEMSG,nCode,lParam); 
}

inline BOOL QUIPostCodeToMainWnd( int nCode,LPARAM lParam )
{
    return QUIPostCodeTo(QUIGetMainWnd(),nCode,lParam);
}

inline LRESULT QUISendCodeToMainWnd( int nCode,LPARAM lParam )
{
    return QUISendCodeTo(QUIGetMainWnd(),nCode,lParam);
}

// 从窗口句柄转换到对应的HtmlayoutView
inline QView* QUIViewFromHWND(__in HWND hWnd);
 
inline QView* QUIViewFromHELEMENT(__in HELEMENT he);
 
// 针对自定义类型，DOM和HWND关联的控件
inline HELEMENT QUIWidgetDOMFromHwnd(__in HWND hWnd);
 
inline HWND QUIHwndFromWidgetDOM(HELEMENT he);

/** 查询特殊控件的指针
 *	return:
 *      控件指针值，需要自己根据需要进行类型转换
 *	params:
 *		-[in]
 *          heCtl       控件包裹dom值
 *          eWidgetType 控件类型 ENUM_WIDGET_TYPE
**/
LPVOID QUIQuerySpecialCtrl(__in HELEMENT heCtl, __in UINT eWidgetType);

typedef struct QUI_USERMSGPARAM
{
	__inout WPARAM			wParam;
	__inout LPARAM			lParam;
	__inout LPVOID			pData;
	__inout CStdString			sParam;
	__inout LRESULT			lResult;
	// 接收者接收到此结构指针
	//	使用完后是否应该删除它
	//	TRUE	接受者必须删除，否则内存泄露
	__in	BOOL			bFreeIt;
	BOOL ShouldIDeleteThis()const { return bFreeIt; }
}*LPQUI_USERMSGPARAM;

inline LPQUI_USERMSGPARAM QUIGetUserMsgParam(WPARAM wParam = 0,
	LPARAM lParam = 0,
	LPVOID pData = NULL,
	CStdString sParam = L"",
	BOOL bAcceptorFree=TRUE)
{
	LPQUI_USERMSGPARAM p = new QUI_USERMSGPARAM;
	p->wParam = wParam;
	p->lParam = lParam;
	p->pData = pData;
	p->sParam = sParam;
	p->lResult = 0;
	p->bFreeIt = bAcceptorFree;

	return p;
}

inline void QUIReleaseUserMsgParam(LPQUI_USERMSGPARAM p)
{
	delete p;
}

#ifndef	QUI_ACCEPTOR_HANDLE_USERMSGPARAM
#	define QUI_ACCEPTOR_HANDLE_USERMSGPARAM(p)\
	if ((NULL != (p)) && ((p)->ShouldIDeleteThis()))\
	{ \
		QUIReleaseUserMsgParam(p); \
	}
#endif

#endif // QUIGlobal_h__
