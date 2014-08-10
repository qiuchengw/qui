#ifndef QApp_h__
#define QApp_h__

#pragma once

// // #include "QUI.h"
#ifndef __ATLBASE_H__
#include <atlbase.h>
#endif

#include <atlapp.h>
#include "QUIGlobal.h"

class QFrame;

// 必须被重写，且只应该有一个实例

// #ifdef QUI_NOT_DLL
// class QApp : public CAppModule
// #else
// class QApp : public CAtlDllModuleT<QApp>
// #endif
class QApp : public CAppModule
{
public:
	QApp();
	~QApp();

public:
	BOOL Run( HINSTANCE hInstance );

    inline BOOL IsQuiting()const
    {
        return m_bIsQuiting;
    }

	QFrame* GetMainWnd()const
    { 
        return m_pMainWnd; 
    }
	
    HWND GetMainSafeWnd(); 
	/** 通过此函数启用或者禁用顶级窗口的默认美化
	 *	-params:	
	 *		-[in]	
	 *				WS_QEX_ROUNDCONNER		圆角美化
	 *				WS_QEX_WNDSHADOW		阴影美化
	 **/
	void SetTopFrameStyle(DWORD dwWndStyle = 0) 
    { 
        m_dwWndStyle = dwWndStyle; 
    }

	DWORD GetTopFrameStyle()const 
    {
        return m_dwWndStyle; 
    }
	
    // 重写此函数，提供版本号功能
    virtual LPCWSTR GetAppVersion()const 
    { 
        return L"0"; 
    }

    // 保存和恢复主窗口位置
    BOOL RestoreWindowPos();

    void SaveWindowPos();

    // 设定应用程序名称
    void SetAppName(__in LPCWSTR szAppName = NULL);

    // 读取应用程序名字
    CStdString GetAppName()const
    {
        return m_sAppName;
    }

protected:
	inline void SetMainWnd(QFrame* pFrame) 
    { 
        m_pMainWnd = pFrame; 
    }

	// 配置文件路径
	virtual CStdString GetConfigPath()const ;

	// 重写此函数以设置主窗口
	virtual BOOL InitRun() 
    { 
        return FALSE; 
    }

    // 应用中止时的操作
    virtual BOOL UnInit();

private:
	QFrame*				m_pMainWnd;
	DWORD				m_dwWndStyle;	// 顶级窗口的美化支持
    BOOL                m_bIsQuiting;   // 是否主消息循环已经跳出
    CStdString             m_sAppName;
};

inline HWND QUIGetMainWnd()
{
    return QUIGetApp()->GetMainSafeWnd();
}

inline HINSTANCE QUIGetResourceInstance()
{
    return QUIGetApp()->GetResourceInstance();
}

inline HINSTANCE QUIGetInstance()
{
    return QUIGetApp()->GetModuleInstance();
}

inline LPCWSTR QUIGetAppVersion()
{
    return QUIGetApp()->GetAppVersion();
}

// 调用此函数启动程序
int QUIRun(__in HINSTANCE hInst);

#endif // QApp_h__
