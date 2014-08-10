#include "stdafx.h"
#include "apphelper.h"
#include <lmerr.h>
#include "ui/QUIGlobal.h"
#include "ui/WndHelper.h"
#include "qmemfile.h"
#include "sys/singleinst.h"

namespace quibase
{
#ifndef AUTORUN_KEYNAME
#define AUTORUN_KEYNAME  _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")
#endif

	CStdString GetModulePath()
	{
		TCHAR path[1024];
		int nSize = ::GetModuleFileName(NULL,path,1024);
		path[nSize] = _T('\0');
		CStdString sRet(path);
		sRet.Replace(_T('\\'),_T('/'));
		int idx = sRet.ReverseFind(_T('/'));
		ATLASSERT(idx != -1);
		sRet = sRet.Left(idx+1);
		return sRet;
	}

    CStdString GetModuleName(BOOL bWithExt/* = FALSE*/)
    {
        TCHAR szFullPath[1024] = {0};
        ::GetModuleFileName(NULL,szFullPath,1024);

        CStdString sRet(szFullPath);
        int idx = sRet.ReverseFind(_T('\\'));
        sRet = sRet.Mid(idx + 1);
        if (!bWithExt)
        {
            idx = sRet.ReverseFind(L'.');
            if (-1 != idx)
            {
                sRet = sRet.Left(idx);
            }
        }
        return sRet;
    }

	BOOL IsFileExist(LPCTSTR pszFile)
	{
		BOOL bRet = FALSE;
		if( pszFile == NULL )
			return bRet;
		if( pszFile[0] == 0 )
			return bRet;

		WIN32_FIND_DATA fd = {0};
		HANDLE hFile = FindFirstFile(pszFile, &fd);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			FindClose(hFile);
			if( !(fd.dwFileAttributes &  FILE_ATTRIBUTE_DIRECTORY) )
				bRet = TRUE;
		}
		return bRet;
	}

	CStdString SysErrorMessage(DWORD dwLastError )
	{
		CStdString strRet(_T("Unknown error"));
		HMODULE hModule = NULL; // default to system source
		LPSTR MessageBuffer;
		DWORD dwBufferLength;

		DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_FROM_SYSTEM ;

		//
		// If dwLastError is in the network range, 
		// load the message source.
		//

		if(dwLastError >= NERR_BASE && dwLastError <= MAX_NERR) {
			hModule = LoadLibraryEx(TEXT("netmsg.dll"),NULL,LOAD_LIBRARY_AS_DATAFILE);
			if(hModule != NULL)
				dwFormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
		}

		//
		// Call FormatMessage() to allow for message 
		// text to be acquired from the system 
		// or from the supplied module handle.
		//
		if(dwBufferLength = FormatMessageA(
			dwFormatFlags,
			hModule, // module to get message from (NULL == system)
			dwLastError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
			(LPSTR) &MessageBuffer,
			0,
			NULL
			))
		{
			//
			// Output message string on stderr.
			//
			strRet=CStdString(MessageBuffer,dwBufferLength);
			//
			// Free the buffer allocated by the system.
			//
			LocalFree(MessageBuffer);
		}

		//
		// If we loaded a message source, unload it.
		//
		if(hModule != NULL)
			FreeLibrary(hModule);
		return strRet;
	}

	void CopyTexttoClipboard(const CStdString& sText, HWND hwnd) 
	{
		if (!::OpenClipboard(hwnd)) 
			return; 

		::EmptyClipboard(); 

		// Allocate a global memory object for the text. 
		HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (sText.GetLength() + 1) * sizeof(TCHAR)); 

		if (!hglbCopy) 
		{ 
			CloseClipboard(); 
			return; 
		} 

		// Lock the handle and copy the text to the buffer. 
		LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy); 

		memcpy(lptstrCopy, (LPVOID)(LPCTSTR)sText, sText.GetLength() * sizeof(TCHAR)); 

		lptstrCopy[sText.GetLength()] = (TCHAR) 0;    // null character 
		GlobalUnlock(hglbCopy); 

		// Place the handle on the clipboard. 
		::SetClipboardData(CF_TEXT, hglbCopy); 

		::CloseClipboard();
	}

	CStdString GetClipboardText(HWND hwnd)
	{
		if (!::OpenClipboard(hwnd)) 
			return ""; 

		HANDLE hData = ::GetClipboardData(CF_TEXT);
		char* buffer = (char*)GlobalLock(hData);

		CStdString sText(buffer);

		::GlobalUnlock(hData);
		::CloseClipboard();

		return sText;
	}

// 
// 	int CompareVersions(LPCTSTR szVersion1, LPCTSTR szVersion2)
// 	{
// 		// if the first character of either string is not a number
// 		// then fall back on a standard string comparison
// 		if (!szVersion1 || !isdigit(szVersion1[0]) || !szVersion2 || !isdigit(szVersion2[0]))
// 			return wcscmp(szVersion1, szVersion2);
// 
// 		CStringArray aVer1, aVer2;
// 
// 		Split(szVersion1, '.', aVer1);
// 		Split(szVersion2, '.', aVer2);
// 
// 		// compare starting from the front
// 		for (int nItem = 0; nItem < aVer1.GetSize() && nItem < aVer2.GetSize(); nItem++)
// 		{
// 			int nThis = _wtoi(aVer1[nItem]);
// 			int nOther = _wtoi(aVer2[nItem]);
// 
// 			if (nThis < nOther)
// 				return -1;
// 
// 			else if (nThis > nOther)
// 				return 1;
// 
// 			// else try next item
// 		}
// 
// 		// if we got here then compare array lengths
// 		if (aVer1.GetSize() < aVer2.GetSize())
// 			return -1;
// 
// 		else if (aVer1.GetSize() > aVer2.GetSize())
// 			return 1;
// 
// 		// else
// 		return 0;
// 	}

	// private method for implementing the bubblesort
// 	BOOL CompareAndSwap(CStringArray& array, int pos, BOOL bAscending)
// 	{
// 		QString temp;
// 		int posFirst = pos;
// 		int posNext = pos + 1;
// 
// 		QString sFirst = array.GetAt(posFirst);
// 		QString sNext = array.GetAt(posNext);
// 
// 		int nCompare = sFirst.CompareNoCase(sNext);
// 
// 		if ((bAscending && nCompare > 0) || (!bAscending && nCompare < 0))
// 		{
// 			array.SetAt(posFirst, sNext);
// 			array.SetAt(posNext, sFirst);
// 
// 			return TRUE;
// 
// 		}
// 		return FALSE;
// 	}
// 
// 	void SortArray(CStringArray& array, BOOL bAscending)
// 	{
// 		BOOL bNotDone = TRUE;
// 
// 		while (bNotDone)
// 		{
// 			bNotDone = FALSE;
// 
// 			for(int pos = 0; pos < array.GetUpperBound(); pos++)
// 				bNotDone |= CompareAndSwap(array, pos, bAscending);
// 		}
// 
// 		/*
// 		#ifdef _DEBUG
// 		TRACE(L"SortArray(%s)\n", bAscending ? "Ascending" : "Descending");
// 
// 		for (int pos = 0; pos < array.GetSize(); pos++)
// 		TRACE(L"%s\n", array[pos]);
// 		#endif
// 		*/
// 	}

    BOOL ShutdownComputer( __in BOOL bReboot,__in LPWSTR pszMsg, 
            __in DWORD dwTimeout,__in BOOL bAskUserCloseApp/*=TRUE*/)
    {
        HANDLE hToken; // handle to process token 
        TOKEN_PRIVILEGES tkp; // pointer to token structure 
        BOOL fResult; // system shutdown flag 
        // Get the current process token handle so we can get shutdown 
        // privilege. 
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        {
            return FALSE;
        }

        // Get the LUID for shutdown privilege. 
        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 
        tkp.PrivilegeCount = 1; // one privilege to set 
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
        // Get shutdown privilege for this process. 
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0); 
        // Cannot test the return value of AdjustTokenPrivileges. 
        if (GetLastError() != ERROR_SUCCESS) 
            return FALSE;

        // Display the shutdown dialog box and start the time-out countdown. 
        fResult = InitiateSystemShutdown( 
            NULL, // shut down local computer 
            pszMsg, // message to user 
            dwTimeout, // time-out period 
            !bAskUserCloseApp, // ask user to close apps 
            bReboot); // reboot after shutdown 
        if (!fResult) 
        {
            return FALSE;
        } 
        // Disable shutdown privilege. 
        tkp.Privileges[0].Attributes = 0; 
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
            (PTOKEN_PRIVILEGES) NULL, 0); 

        return (GetLastError() == ERROR_SUCCESS);
    }

    // 取消关机
    BOOL PreventSystemShutdown()
    {
        HANDLE hToken;              // handle to process token 
        TOKEN_PRIVILEGES tkp;       // pointer to token structure 

        // Get the current process token handle  so we can get shutdown 
        // privilege. 

        if (!OpenProcessToken(GetCurrentProcess(), 
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
            return FALSE; 

        // Get the LUID for shutdown privilege. 

        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
            &tkp.Privileges[0].Luid); 

        tkp.PrivilegeCount = 1;  // one privilege to set    
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

        // Get shutdown privilege for this process. 

        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
            (PTOKEN_PRIVILEGES)NULL, 0); 

        if (GetLastError() != ERROR_SUCCESS) 
            return FALSE; 

        // Prevent the system from shutting down. 

        if ( !AbortSystemShutdown(NULL) ) 
            return FALSE; 

        // Disable shutdown privilege. 

        tkp.Privileges[0].Attributes = 0; 
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
            (PTOKEN_PRIVILEGES) NULL, 0); 

        return TRUE;
    }

    BOOL UpgradeProcessPrivilege()
    {
        HANDLE hToken; 
        TOKEN_PRIVILEGES tkp;

        // Get a token for this process.
        if (!OpenProcessToken(GetCurrentProcess(), 
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
            return FALSE ; 

        // Get the LUID for the shutdown privilege.
        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
            &tkp.Privileges[0].Luid); 

        tkp.PrivilegeCount = 1; // one privilege to set 
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

        // Get the shutdown privilege for this process.
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
            (PTOKEN_PRIVILEGES)NULL, 0); 

        if (GetLastError() != ERROR_SUCCESS) 
            return FALSE; 

        return TRUE;
    }

    BOOL ReleaseRes(LPCTSTR filename,WORD wResID, LPCTSTR filetype)
    {
        DWORD dwWrite=0;
        HANDLE  hFile = CreateFile(filename, GENERIC_WRITE,FILE_SHARE_WRITE,
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile == INVALID_HANDLE_VALUE)
            return FALSE;

        // 查找资源文件中、加载资源到内存、得到资源大小   
        HRSRC hrsc=FindResource(NULL, MAKEINTRESOURCE(wResID), filetype);
        HGLOBAL hG=LoadResource(NULL, hrsc);
        DWORD dwSize=SizeofResource( NULL, hrsc);
        // 写入文件
        WriteFile(hFile,hG,dwSize,&dwWrite,NULL);
        CloseHandle(hFile);
        return TRUE;
    }
  };

//////////////////////////////////////////////////////////////////////////
QMemFile::LstME        QMemFile::ms_LstMe;

BOOL QMemFile::SetEventData(HANDLE hEvent,DWORD dwData)  
{  
	MeItr itr = FindEvent(hEvent);
	if (!IsEndItr(itr))
	{
		ME memFile = *itr;
		if(memFile.pMapBuf == NULL)  
		{
			return FALSE;  
		}  
		//将数值拷贝到内存中  
		memcpy(memFile.pMapBuf,&dwData,sizeof(DWORD));  
	}
	return FALSE;
}  

DWORD QMemFile::GetEventData(HANDLE hEvent)  
{  
	MeItr itr = FindEvent(hEvent);
	if (!IsEndItr(itr))
	{
		ME memFile = *itr;
		if(memFile.pMapBuf == NULL)  
		{  
			return 0;  
		}  
		//从内存中获取DWORD数据
		DWORD dwVal = 0;  
		memcpy(&dwVal,memFile.pMapBuf,4); 
		return dwVal;
	}
	return 0;  
}  

QMemFile::MeItr QMemFile::FindEvent(HANDLE hEvent)
{
	for ( MeItr itr = ms_LstMe.begin(); itr != ms_LstMe.end(); ++itr)
	{
		if (itr->hEvent == hEvent)
			return itr;
	}
	return ms_LstMe.end();
}

BOOL QMemFile::CloseHandle(HANDLE hObject)  
{  
	MeItr itr;
	if(hObject == NULL || ( itr = FindEvent(hObject)) == ms_LstMe.end())  
		return FALSE;

	::CloseHandle(itr->hFileMap);
	htmlayout::mutex lck;
	lck.lock();
	ms_LstMe.erase(itr);
	lck.unlock();
	return ::CloseHandle(hObject);
} 

BOOL QMemFile::GetMemFile(LPCTSTR lpEventName, ME &memFile) 
{
	memFile.sEventName = GetEventName(lpEventName);
	// 创建映射文件  
	HANDLE hFileMap = ::OpenFileMapping(FILE_MAP_ALL_ACCESS,TRUE,memFile.sEventName);
	if (hFileMap == NULL
		&& (hFileMap = ::CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,4,memFile.sEventName)) == NULL)
	{
		return FALSE;  
	}
	//从映射文件句柄获得分配的内存空间  
	VOID *pMapBuf = MapViewOfFile(hFileMap,FILE_MAP_ALL_ACCESS,0,0,0);   
	if(pMapBuf != NULL)  
	{  
		//将数值保存到结构体中  
		memFile.hFileMap = hFileMap;  
		memFile.pMapBuf = pMapBuf; 
		return TRUE;
	} 
	CloseHandle(hFileMap);
	return FALSE;
}

CStdString QMemFile::GetEventName(LPCTSTR lpName,HANDLE hEvent/*=NULL*/)
{
	TCHAR buf[MAX_PATH] = {0};
	//先判断这个类是否只是内部使用。所谓的内部使用，指的是没有名字的事件，除了通过句柄来进行使用以外，无法通过再次打开获得。  
	if(lpName != NULL && _tcslen(lpName) > 0)  
	{  
		//因为内存映射文件和事件名是同一个命名空间，所以这两者的名字不能相同。故我们要创建的内存映射文件名为：EVENT_前缀 + 事件名。  
		_stprintf_s(buf,MAX_PATH,TEXT("EVENT_%s\0"),lpName);
	}  
	else  
	{  
		//如果该事件为内部使用，那么也就意味着这内存映射文件也是内部使用。故采用程序句柄的名字+事件名的方式进行内存映射文件的名字确定。  
		_stprintf_s(buf,MAX_PATH,TEXT("%ld_%ld\0"),(DWORD)GetModuleHandle(NULL),(DWORD)hEvent);
	}  
	return buf;
}

HANDLE QMemFile::CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,BOOL bManualReset,
	BOOL bInitialState,LPCTSTR lpName)
{  
	HANDLE hEvent = ::CreateEvent(lpEventAttributes,bManualReset,bInitialState,lpName);  
	if(hEvent != NULL)  
	{
		ME memFile;
		if (GetMemFile(lpName,memFile))
		{
			memFile.hEvent = hEvent;
			ms_LstMe.push_back(memFile);
			return hEvent;  
		}
	}
	::CloseHandle(hEvent);
	return NULL;
}  


BOOL CSingleInstance::InstanceAlreadyRun( LPCTSTR pszName,BOOL bBringLastTop/*=TRUE*/ )
{ 
    m_hEvent = QMemFile::CreateEvent(NULL,TRUE, FALSE, pszName);
    ASSERT (m_hEvent != NULL);
    if (NULL != m_hEvent)
    {
        SetEvent(m_hEvent);
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) // 已经有一个实例在运行了
    {
        if (bBringLastTop)
        {
            HWND hWnd = (HWND)QMemFile::GetEventData(m_hEvent);
            if (hWnd != NULL)
            {
                ShowWindow(hWnd,SW_HIDE);
                ShowWindow(hWnd,SW_RESTORE);
                quibase::SetForegroundWindowInternal(hWnd);
            }
        }
        return TRUE;
    }
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////
void CSingleInstance::RemoveRestrict()
{
    if (m_hEvent != NULL) 
    {
        QMemFile::CloseHandle(m_hEvent);
        m_hEvent = NULL;
    } 
}

HWND CSingleInstance::GetInstanceMainWnd()
{
    return (HWND)QMemFile::GetEventData(m_hEvent);
}

BOOL CSingleInstance::SetInstanceMainWnd( HWND hWnd )
{
    if (::IsWindow(hWnd))
    {
        QMemFile::SetEventData(m_hEvent,(DWORD)hWnd);
        return TRUE;
    }
    return FALSE;
}
