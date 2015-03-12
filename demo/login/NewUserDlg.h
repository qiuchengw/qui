#pragma once

#include "ui/QDialog.h"

class NewUserDlg : public QDialog
{
public:
	NewUserDlg(void);
	~NewUserDlg(void);

protected:
	virtual BOOL OnDefaultButton(INT_PTR nBtn);
};

