#include "stdafx.h"

#ifdef _DEBUG
//#include "./dbg/vld/include/vld.h"
#endif

// lib工程和dll工程，需要包含这个头文件，以支持behavior
// #if defined QUI_LIBPRJ || !defined QUI_NOT_DLL
// #pragma message("--->behavior support")
#include "htmlayout/behaviors/support_.h"
//#endif

//#include "stdafx.h"

#include "QApp.h"
#include "QUIMgr.h"
#include "QConfig.h"
#include "QFrame.h"
#include "AppHelper.h"
#include "WndShadow.h"
#include "QMessageLoop.h"

QApp* QUIGetApp()
{
    return dynamic_cast<QApp*>(ATL::_pAtlModule);
}

int QUIRun(__in HINSTANCE hInst)
{
    // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
    ::DefWindowProc(NULL, 0, 0, 0L);

    // AtlInitCommonControls(ICC_BAR_CLASSES); // add flags to support other controls
    int nRet = -1;
    QApp *pApp = QUIGetApp();
    if (NULL != pApp)
    {
        if (SUCCEEDED(pApp->Init(NULL, hInst)))
        {
			// 否则被优化掉了
            QUIGetInstance();
            nRet = pApp->Run(hInst);
        }
        pApp->Term();
    }
    return nRet;
}

BOOL QUIIsQuiting()
{
    return QUIGetApp()->IsQuiting();
}

CStdString QUIGetAppName()
{
    return QUIGetApp()->GetAppName();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
QApp::~QApp()
{
	m_bIsQuiting = TRUE;
}

QApp::QApp()
{
//	_theApp = this;
	m_pMainWnd = NULL;
	m_dwWndStyle = 0;
}

BOOL QApp::Run( HINSTANCE hInstance )
{
    SetAppName(NULL);

    // 设置工作目录
    SetCurrentDirectory(quibase::GetModulePath());

	// 配置应用程序
    QConfig *pCfg = QUIGetConfig();
	if ((NULL == pCfg) || !(pCfg->SetConfigPath(GetConfigPath())))
	{
        // 这个时候还没有加载UI，不能使用QUI里面的MessageBox
        ::MessageBox(NULL,L"读取配置文件失败！", L"错误", MB_OK);
        ATLASSERT(FALSE);
        return FALSE;
	}
	// 配置UI管理器
	if (!QUIMgr::GetInstance()->Startup())
	{
        ::MessageBox(NULL,L"UI管理器启动失败！", L"错误", MB_OK);
		ATLASSERT(FALSE);
		return FALSE;
	}

	// 默认开启窗口的圆角和阴影效果
	CWndShadow::Initialize(hInstance);
	SetTopFrameStyle(WS_QEX_ROUNDCONNER|WS_QEX_WNDSHADOW);

	// 主界面线程循环
	QMessageLoop loop;
	AddMessageLoop(&loop);

	// 启动主界面
	if (!InitRun() || (m_pMainWnd == NULL))
	{
		return FALSE;
	}

	loop.Run();
	RemoveMessageLoop();

    UnInit();

    QUIGetMainWnd();

	return TRUE;
}

HWND QApp::GetMainSafeWnd()
{
	if (NULL == m_pMainWnd)
	{
		return NULL;
	}
	return m_pMainWnd->GetSafeHwnd();
}

CStdString QApp::GetConfigPath() const
{
    return quibase::GetModulePath() + L"config.ini"; 
}

void QApp::SaveWindowPos()
{
    QConfig *pCfg = QUIGetConfig();
    HWND hWnd = GetMainSafeWnd();
    if (!::IsWindow(hWnd) || (NULL == pCfg))
    {
        ATLASSERT(FALSE);
        return;
    }

    WINDOWPLACEMENT wp = { 0 };
    wp.length = sizeof(WINDOWPLACEMENT);

    // get window position and iconized/maximized status
    ::GetWindowPlacement(hWnd, &wp);

    wp.flags   = 0;
    wp.showCmd = SW_SHOWNORMAL;

    TCHAR tmp[200];
    WTL::SecureHelper::sprintf_x(tmp, 200,_T("%u,%u,%d,%d,%d,%d"),
        wp.flags,
        wp.showCmd,
        wp.rcNormalPosition.left,
        wp.rcNormalPosition.top,
        wp.rcNormalPosition.right,
        wp.rcNormalPosition.bottom);

    // write position to INI file
    pCfg->SetValue(L"app",L"MainWindowPos", tmp);
}

BOOL QApp::RestoreWindowPos()
{
    HWND hWnd = GetMainSafeWnd();
    QConfig *pCfg = QUIGetConfig();
    if (!::IsWindow(hWnd) || (NULL == pCfg))
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
    // read window position from INI file
    // MainWindow format =0,1,395,198,1012,642
    CStdString sPos = pCfg->GetValue(L"app",L"MainWindowPos");
    if (sPos.IsEmpty())
        return FALSE;

    TCHAR tmp[256];
    _tcsncpy_s(tmp, sPos, _countof(tmp)-2);

    // get WINDOWPLACEMENT info
    WINDOWPLACEMENT wp = { 0 };
    wp.length = sizeof(WINDOWPLACEMENT);
    wp.ptMaxPosition = CPoint(-::GetSystemMetrics(SM_CXBORDER), -::GetSystemMetrics(SM_CYBORDER));
    TCHAR *cp = 0;
    wp.rcNormalPosition = CRect(200, 100, 850, 550);
    if ((cp = _tcstok(tmp, _T(",\r\n"))) != NULL)
        wp.flags = _ttoi(cp);
    if ((cp = _tcstok(NULL,  _T(",\r\n"))) != NULL)
        wp.showCmd = _ttoi(cp);
    if ((cp = _tcstok(NULL,  _T(",\r\n"))) != NULL)
        wp.rcNormalPosition.left = _ttoi(cp);
    if ((cp = _tcstok(NULL,  _T(",\r\n"))) != NULL)
        wp.rcNormalPosition.top = _ttoi(cp);
    if ((cp = _tcstok(NULL,  _T(",\r\n"))) != NULL)
        wp.rcNormalPosition.right = _ttoi(cp);
    if ((cp = _tcstok(NULL,  _T(",\r\n"))) != NULL)
        wp.rcNormalPosition.bottom = _ttoi(cp);
    // sets window's position and iconized/maximized status
    ::SetWindowPlacement(hWnd, &wp);

    // 最后给它一个wm_move 消息
    ::SendMessage(hWnd, WM_EXITSIZEMOVE, 0, 0);
    return TRUE;
}

BOOL QApp::UnInit()
{
    // 程序即将退出了， 设置这个状态
    m_bIsQuiting = TRUE;

    // 有些窗口是永久窗口，现在给这些窗口一个最后的关闭机会
    QUIMgr::GetInstance()->Shutdown();

    return TRUE;
}

void QApp::SetAppName( __in LPCWSTR szAppName /*= NULL*/ )
{
    // 调用一下呗，否则会被优化掉的
    QUIGetAppName();

    m_sAppName = szAppName;

    if (m_sAppName.Trim().IsEmpty())
    {
        m_sAppName = quibase::GetModuleName(FALSE);
    }
}
