#ifndef singleinst_h__
#define singleinst_h__

#pragma once

// 实现在AppHelper.cpp中
class QMemFile;
class CSingleInstance
{
public:
	CSingleInstance():m_hEvent(NULL)
    {

    }

	~CSingleInstance()
    { 
        RemoveRestrict();
    }

    /** 移除单实例限制
     *      应用程序可以运行多实例
    **/
    void RemoveRestrict();

	/**
	 * @brief	:	
	 * @parma	:	pszName		事件名字
				bBringLastTop	如果已经有此实例,将其窗口带到前端
	 * @return	:	TRUE		已有实例运行
	 */
	BOOL InstanceAlreadyRun(LPCTSTR pszName,BOOL bBringLastTop=TRUE);

	BOOL SetInstanceMainWnd(HWND hWnd);

	HWND GetInstanceMainWnd();

private:
	HANDLE	m_hEvent;
};

#endif // apphelper_h__
