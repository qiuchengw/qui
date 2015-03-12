#pragma once

#include "ui/QApp.h"
#include "ui/QConfig.h"
#include "AppHelper.h"
#include "ui/QUIGlobal.h"

class TUIApp : public QApp
{
public:
	TUIApp(void);
	~TUIApp(void);

	virtual BOOL InitRun();
};

class DemoConfig : public QUIConfig
{
public:
};

