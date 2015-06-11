#ifndef QConfig_h__
#define QConfig_h__

#pragma once

#include "BaseType.h"
#include "file/inifile.h"
#include "time/QTime.h"

class QConfig
{
public:
	QConfig();

	BOOL SetConfigPath(const CStdStringW &sc);

	CStdString GetValue(LPCWSTR sSection,LPCWSTR sKey);
    int GetIntValue(LPCWSTR sSection,LPCWSTR sKey);
    double GetDoubleValue(LPCWSTR sSection,LPCWSTR sKey);
    QTime GetTimeValue(LPCWSTR sSection,LPCWSTR sKey);

    void SetValue(LPCWSTR sSection,LPCWSTR sKey,LPCWSTR sValue);
    void SetValue(LPCWSTR sSection, LPCWSTR sKey, int nVal);
    void RemoveKey(LPCWSTR sSection, LPCWSTR sKey);

	CStdString GetSettingValue(LPCWSTR szKey);

	BOOL Save();

protected:
	CIniFile	sm_ini;
	CStdString		m_sIniF;
};

// 此类必须被重写 且只应该有一个实例
class QUIConfig : public QConfig
{
public:
    QUIConfig();

    // htmlayout 的master-css
    // virtual QString GetMainQSS();
    // virtual QString GetLanguage();
    virtual CStdString GetMasterResCssBaseUrl();
};

#endif // QConfig_h__
