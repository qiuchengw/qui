#include "NewUserDlg.h"
#include "ui/QUIDlgs.h"

NewUserDlg::NewUserDlg(void) :QDialog(L"qabs:newuser/index.htm")
{
}

NewUserDlg::~NewUserDlg(void)
{
}

BOOL NewUserDlg::OnDefaultButton( INT_PTR nBtn )
{
	if (IDOK != nBtn)
		return TRUE;

	return TRUE;
}
