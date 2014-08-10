#pragma once

#include "ui/QDialog.h"

enum
{
	LOGIN_TIMER_LOOKINGFORSERVER = 100,
	LOGIN_TIMER_COUNTDOWN = 101,
};

class LoginDlg : public QDialog
{
	BEGIN_MSG_MAP_EX(LoginDlg)
		CHAIN_MSG_MAP(QDialog)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_CLOSE(OnClose)
	END_MSG_MAP()

public:
	LoginDlg();
	~LoginDlg(void);

protected:
	virtual LRESULT OnDocumentComplete();
	virtual BOOL OnDefaultButton(INT_PTR nBtn);
	void OnClose();
	void SetTip(LPCWSTR szTip);
	void OnTimer(int nID);
	BOOL LoginAsServer();
	BOOL LoginAsServer(LPCWSTR szUser,LPCWSTR szPswd);
	BOOL LoginAsClient();
	void StartCountDown();
	void EndCountDown();

	void OnLoginResult(LPARAM p);
	void OnConnectedToServer( LPARAM p );
	void OnUpdateMenuDB(LPARAM p);
	void EnableOkButton(BOOL bEnable);

	BOOL StartupMenuList();

private:
	ECtrl		m_eTip;
	int			m_nCountDown;
};

