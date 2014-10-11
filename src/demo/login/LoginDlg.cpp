#include "LoginDlg.h"
#include "TUIApp.h"

LoginDlg::LoginDlg():QDialog(L"qabs:login/login.htm")
{
}

LoginDlg::~LoginDlg(void)
{

}

void LoginDlg::SetTip( LPCWSTR szTip )
{
	if (!m_eTip.is_valid())
	{
		m_eTip = GetCtrl("#TXT-TIP");
	}

	utf8::ostream o;
	o<<szTip;
	m_eTip.set_html(o.data(),o.length());
}

BOOL LoginDlg::OnDefaultButton( INT_PTR nBtn )
{
	if (IDOK != nBtn)
		return TRUE;

    return FALSE;
}

void LoginDlg::OnConnectedToServer( LPARAM p )
{
}

void LoginDlg::OnTimer( int nID )
{
}

void LoginDlg::EnableOkButton(BOOL bEnable)
{
	GetCtrl("[xrole=\"OK\"]").EnableCtrl(bEnable);
}

BOOL LoginDlg::LoginAsClient()
{
	// 还不能关闭窗口。必须等待
	return FALSE;
}

BOOL LoginDlg::LoginAsServer()
{

	return TRUE;
}

void LoginDlg::StartCountDown()
{
	// 如果m_nCountDown秒内还未能查找到服务器，那么就认为服务器未运行
	m_nCountDown = 7;
	SetTimer(LOGIN_TIMER_LOOKINGFORSERVER,m_nCountDown * 1000);
	SetTimer(LOGIN_TIMER_COUNTDOWN,1000);

}

void LoginDlg::EndCountDown()
{
	KillTimer(LOGIN_TIMER_COUNTDOWN);
	KillTimer(LOGIN_TIMER_LOOKINGFORSERVER);

}

// 客户端的启动的结果
// 服务器端不会运行到这儿
void LoginDlg::OnLoginResult( LPARAM p )
{

}

void LoginDlg::OnUpdateMenuDB( LPARAM p )
{
}

LRESULT LoginDlg::OnDocumentComplete()
{

    EEdit(GetCtrl("#TXT-NAME")).SetText(L"name");
    EPassword(GetCtrl("#TXT-PSWD")).SetText(L"pswd");

    ECheck(GetCtrl("#CHK-REMEBER")).SetCheck(TRUE);
	
	return 0;
}

BOOL LoginDlg::StartupMenuList()
{
    return FALSE;
}

void LoginDlg::OnClose()
{
	SetMsgHandled(FALSE);

}

