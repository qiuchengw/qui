#include "TUIApp.h"
#include "ui/WndShadow.h"
#include "LoginDlg.h"
#include "ui/QUIDlgs.h"
#include "crypt/QDataEnDe.h"

TUIApp _Module;
DemoConfig _theStkToolConfig;

TUIApp::TUIApp(void)
{
}

TUIApp::~TUIApp(void)
{
}

BOOL TUIApp::InitRun()
{
	DemoConfig *pCfg = (DemoConfig*)QUIGetConfig();
	
	// 初始化阴影效果组件
	CWndShadow::Initialize(QUIGetInstance());
	SetTopFrameStyle(WS_QEX_ROUNDCONNER|WS_QEX_WNDSHADOW);

	//////////////////////////////////////////////////////////////////////////
	// 登录系统
	LoginDlg().DoModal(NULL, WS_EX_TOOLWINDOW|WS_EX_TOPMOST);

	return FALSE;
}
