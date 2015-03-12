#include "stdafx.h"
#include "QConfig.h"
#include "apphelper.h"

QConfig::QConfig()
{
}

BOOL QConfig::SetConfigPath( const CStdStringW &sc )
{
	m_sIniF = sc;
	return sm_ini.Load(sc);
}

CStdString QConfig::GetValue( LPCWSTR sSection,LPCWSTR sKey )
{
	return sm_ini.GetKeyValue(sSection,sKey);
}

int QConfig::GetIntValue( LPCWSTR sSection,LPCWSTR sKey )
{
    return StrToInt(GetValue(sSection, sKey));
}

CStdString QConfig::GetSettingValue( LPCWSTR szKey )
{
	return sm_ini.GetKeyValue(L"setting",szKey);
}

void QConfig::SetValue( LPCWSTR sSection,LPCWSTR sKey,LPCWSTR sValue )
{
	sm_ini.SetKeyValue(sSection,sKey,sValue);
	sm_ini.Save(m_sIniF);
}

void QConfig::SetValue( LPCWSTR sSection,LPCWSTR sKey,int nVal )
{
    CStdString s;
    s.Format(L"%d",nVal);
    SetValue(sSection, sKey, s);
}

BOOL QConfig::Save()
{
	return sm_ini.Save(m_sIniF);
}

double QConfig::GetDoubleValue( LPCWSTR sSection,LPCWSTR sKey)
{
    return _wtof(GetValue(sSection, sKey));
}

QTime QConfig::GetTimeValue( LPCWSTR sSection,LPCWSTR sKey )
{
    return _wtof(GetValue(sSection, sKey));
}

//////////////////////////////////////////////////////////////////////////
// 全局唯一实例
QUIConfig *_theConfig = NULL;

QUIConfig* QUIGetConfig()
{
    return _theConfig;
}

QUIConfig::QUIConfig() 
{
    // 此处初始化
    _theConfig = this;
}

// QString QUIConfig::GetMainQSS()
// {
//     QString sCss = sm_ini.GetKeyValue(L"UI",L"qss");
//     if (sCss.GetLength() <= 0)
//     {
//         sCss = L"Main.zip";
//     }
//     return sCss;
// }

// QString QUIConfig::GetLanguage()
// {
//     QString sLau = sm_ini.GetKeyValue(L"APP",L"lang");
//     if (sLau.IsEmpty())
//         return quibase::GetModulePath()+L"Laug/default.lng";
//     return quibase::GetModulePath()+L"Laug/"+sLau;
// }

CStdString QUIConfig::GetMasterResCssBaseUrl()
{
    return L"skin/xctrl";
}
