#ifndef qmemfile_h__
#define qmemfile_h__

#pragma once

#include <list>

//-------------------------------------------------------------
// QMUF
// зїеп	qiuchengw
//-------------------------------------------------------------
class QMemFile  
{  
public:
    static DWORD GetEventData(HANDLE hEvent) ;

    static BOOL SetEventData(HANDLE hEvent,DWORD dwData);

    static BOOL CloseHandle(HANDLE hObject)  ;

    static HANDLE CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
        BOOL bManualReset,BOOL bInitialState,LPCTSTR lpName)  ;

private:
    struct ME	// Event name and associated MemFile 
    {
        CStdString	sEventName;
        HANDLE hFileMap;  
        HANDLE pMapBuf;
        HANDLE hEvent;
    };

    typedef std::list<ME>		LstME;
    typedef LstME::iterator		MeItr;
    static LstME	ms_LstMe;

    static BOOL GetMemFile(LPCTSTR lpEventName,ME &memFile) ;
    static CStdString GetEventName(LPCTSTR lpName,HANDLE hEvent=NULL);
    static MeItr FindEvent(HANDLE hEvent);
    static BOOL IsEndItr(MeItr &itr) { return !(itr != ms_LstMe.end()); }
};  


#endif // apphelper_h__
