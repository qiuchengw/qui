#ifndef TRACE_H__
#define TRACE_H__

//////////////////////////////////////////////////////////////////////////
// XTrace.h  Version 1.3
//
// Author:       Paul Mclachlan
//
// Modified by:  Hans Dietrich
//               hdietrich@gmail.com
//
// Description:
//     XTrace provides a non-MFC set of macros for logging.  It is a drop-in
//     replacement for the MFC TRACE macros, and may be used in debug and 
//     release modes.
//
// History:
//     Version 1.3
//     - added TRACERGB
//     - terminated all buffers with nul
//
//     Version 1.2
//     - fixed macro redefinition warnings
//
//     Version 1.1
//     - added Unicode support
//     - added optional thread id to output string
//     - added option to enable/disable full path
//     - added TRACERECT macro
//     - changed name to avoid conflicts with Paul's class.
//
// This code was taken from article by Paul Mclachlan, "Getting around 
// the need for a vararg #define just to automatically use __FILE__ and 
// __LINE__ in a TRACE macro".  For original article, see
//     http://www.codeproject.com/KB/debug/location_trace.aspx
//
// XTrace.h is a drop-in replacement for MFC's TRACE facility.  It has no
// dependency on MFC.  It is thread-safe and uses no globals or statics.
//
// It optionally adds source module/line number and thread id to each line 
// of TRACE output.  To control these features, use the following defines:
//
//     XTRACE_SHOW_FULLPATH
//     XTRACE_SHOW_THREAD_ID
//
// XTrace.h also provides extended macros for specific data formats:
//
//        MACRO                         DESCRIPTION
//     ------------    ------------------------------------------------------
//     TRACEHILO       Outputs high and low words, using HIWORD() and LOWORD()
//     TRACEPOINT      Outputs the contents of POINT struct
//     TRACERECT       Outputs the contents a RECT struct.  
//     TRACERGB        Outputs an RGB value
//     TRACESIZE       Outputs the contents of SIZE struct

// In Release builds, no output will be produced, unless you change
// the line "#ifdef _DEBUG" to :#ifdef _DEBUGnow" - see line 283.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef XTRACE_H
#define XTRACE_H

#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#pragma warning(push)
#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4996)		// disable bogus deprecation warning

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

#define XTRACE_SHOW_FULLPATH	FALSE	// FALSE = only show base name of file
#define XTRACE_SHOW_THREAD_ID	TRUE	// TRUE = include thread id in output
#define XTRACE_FILE				FALSE	// TRUE = output to file

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

class xtracing_output_debug_string
{
public:
    xtracing_output_debug_string(LPCTSTR lpszFile, int line) :
      m_file(lpszFile),
          m_line(line)
      {
      }

      void operator() (LPCTSTR lpszFormat, ...)
      {
          va_list va;
          va_start(va, lpszFormat);

          TCHAR buf1[BUFFER_SIZE];
          TCHAR buf2[BUFFER_SIZE];

          // add the __FILE__ and __LINE__ to the front
          TCHAR *cp = (LPTSTR) m_file;

          if (!XTRACE_SHOW_FULLPATH)
          {
              cp = (TCHAR *)_tcsrchr(m_file, _T('\\'));
              if (cp)
                  cp++;
          }

          if (XTRACE_SHOW_THREAD_ID)
          {
              _sntprintf_s(buf1, BUFFER_SIZE-1, _T("%s(%d) : [%X] %s"), 
                  cp, m_line, GetCurrentThreadId(), lpszFormat);
              buf1[_countof(buf1)-1] = _T('\0');
          }
          else
          {
              _sntprintf_s(buf1, BUFFER_SIZE-1, _T("%s(%d) : %s"), 
                  cp, m_line, lpszFormat);
              buf1[_countof(buf1)-1] = _T('\0');
          }

          // format the message as requested
          _vsntprintf_s(buf2, BUFFER_SIZE-1, buf1, va);
          buf2[_countof(buf2)-1] = _T('\0');

          va_end(va);

          if (XTRACE_FILE)
          {
              TCHAR szPathName[MAX_PATH*2] = { 0 };

              ::GetModuleFileName(NULL, szPathName, sizeof(szPathName)/sizeof(TCHAR)-2);

              TCHAR *cp = _tcsrchr(szPathName, _T('\\'));
              if (cp != NULL)
                  *(cp+1) = _T('\0');
              StringCchCat(szPathName,2*MAX_PATH, _T("\\_trace.log"));
              HANDLE hFile = ::CreateFile(szPathName, GENERIC_WRITE, FILE_SHARE_WRITE,
                  NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
              if (hFile != INVALID_HANDLE_VALUE)
              {
                  DWORD dwRC = ::SetFilePointer(hFile,		// handle to file
                      0,			// bytes to move pointer
                      NULL,			// bytes to move pointer
                      FILE_END);	// starting point

                  if (dwRC != INVALID_SET_FILE_POINTER)
                  {
                      DWORD dwWritten = 0;
                      ::WriteFile(hFile,							// handle to file
                          buf2,							// data buffer
                          (DWORD)_tcslen(buf2)*sizeof(TCHAR),	// number of bytes to write
                          &dwWritten,						// number of bytes written
                          NULL);							// overlapped buffer
                  }

                  ::CloseHandle(hFile);
              }
          }
          else
          {
              // write it out
              OutputDebugString(buf2);
          }
      }

private:
    LPCTSTR	m_file;
    int		m_line;
    enum	{ BUFFER_SIZE = 4096 };
};

class xtracing_entry_output_debug_string
{
public:
    xtracing_entry_output_debug_string(LPCTSTR lpszFile, int line) :
      m_file(lpszFile),
          m_line(line)
      {
      }

      ~xtracing_entry_output_debug_string()
      {
          TCHAR buf3[BUFFER_SIZE*3];
          StringCbPrintf(buf3, BUFFER_SIZE*3 ,_T("======  exiting scope:  %s"), buf2);
          buf3[_countof(buf3)-1] = _T('\0');
          OutputDebugString(buf3);
      }

      void operator() (LPCTSTR lpszFormat, ...)
      {
          va_list va;
          va_start(va, lpszFormat);

          TCHAR buf1[BUFFER_SIZE];

          // add the __FILE__ and __LINE__ to the front
          TCHAR *cp = (LPTSTR) m_file;

          if (!XTRACE_SHOW_FULLPATH)
          {
              cp = (TCHAR *)_tcsrchr(m_file, _T('\\'));
              if (cp)
                  cp++;
          }

          if (XTRACE_SHOW_THREAD_ID)
          {
              _sntprintf_s(buf1, BUFFER_SIZE-1, _T("%s(%d) : [%X] ======  %s"), 
                  cp, m_line, GetCurrentThreadId(), lpszFormat);
              buf1[_countof(buf1)-1] = _T('\0');
          }
          else
          {
              _sntprintf_s(buf1, BUFFER_SIZE-1, _T("%s(%d) : ======  %s"), 
                  cp, m_line, lpszFormat);
              buf1[_countof(buf1)-1] = _T('\0');
          }

          // format the message as requested
          _vsntprintf_s(buf2, BUFFER_SIZE-1, buf1, va);
          buf2[_countof(buf2)-1] = _T('\0');

          va_end(va);

          if (XTRACE_FILE)
          {
              TCHAR szPathName[MAX_PATH*2] = { 0 };

              ::GetModuleFileName(NULL, szPathName, sizeof(szPathName)/sizeof(TCHAR)-2);

              TCHAR *cp = _tcsrchr(szPathName, _T('\\'));
              if (cp != NULL)
                  *(cp+1) = _T('\0');
              StringCchCat(szPathName, 2*MAX_PATH,_T("\\_trace.log"));
              HANDLE hFile = ::CreateFile(szPathName, GENERIC_WRITE, FILE_SHARE_WRITE,
                  NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
              if (hFile != INVALID_HANDLE_VALUE)
              {
                  DWORD dwRC = ::SetFilePointer(hFile,		// handle to file
                      0,			// bytes to move pointer
                      NULL,			// bytes to move pointer
                      FILE_END);	// starting point

                  if (dwRC != INVALID_SET_FILE_POINTER)
                  {
                      DWORD dwWritten = 0;
                      ::WriteFile(hFile,							// handle to file
                          buf2,							// data buffer
                          (DWORD)_tcslen(buf2)*sizeof(TCHAR),	// number of bytes to write
                          &dwWritten,						// number of bytes written
                          NULL);							// overlapped buffer
                  }

                  ::CloseHandle(hFile);
              }
          }
          else
          {
              // write it out
              OutputDebugString(buf2);
          }
      }

private:
    LPCTSTR	m_file;
    int		m_line;
    enum	{ BUFFER_SIZE = 4096 };
    TCHAR	buf2[BUFFER_SIZE*2];
};

#undef TRACE
#undef TRACE0
#undef TRACE1
#undef TRACE2
#undef TRACERECT
#undef TRACERGB
#undef TRACEPOINT
#undef TRACESIZE
#undef TRACEHILO

#define _DEBUGnow

// change following line to _DEBUGnow to output trace in release mode
#ifdef _DEBUG

#define TRACE (xtracing_output_debug_string(_T(__FILE__), __LINE__ ))
#define TRACEENTRY (xtracing_output_debug_string(_T(__FILE__), __LINE__ ))
#define TRACEERROR (xtracing_output_debug_string(_T(__FILE__), __LINE__ ))
#define TRACE0 TRACE
#define TRACE1 TRACE
#define TRACE2 TRACE

#define TRACEHILO(d) \
    WORD ___hi = HIWORD(d); WORD ___lo = LOWORD(d); \
    TRACE(_T(#d) _T(":  HIWORD = %u  LOWORD = %u\n"), ___hi, ___lo)

#define TRACESIZE(s) TRACE(_T(#s) _T(":  cx = %d  cy = %d\n"), \
    (s).cx, (s).cy)

#define TRACEPOINT(p) TRACE(_T(#p) _T(":  x = %d  y = %d\n"), \
    (p).x, (p).y)

#define TRACERECT(r) TRACE(_T(#r) _T(":  left = %d  top = %d  right = %d  bottom = %d\n"), \
    (r).left, (r).top, (r).right, (r).bottom)

#define TRACERGB(cr) TRACE(_T(#cr) _T(":  RGB(%d,%d,%d)\n"), \
    GetRValue(cr), GetGValue(cr), GetBValue(cr));

#define TRACEDATE(dt) TRACE(_T(#dt) _T(": %d-%d-%d %d:%d:%d\n"),dt.GetYear(),dt.GetMonth(),\
    dt.GetDay(),dt.GetHour(),dt.GetMinute(),dt.GetSecond());


#else

#ifndef __noop
#if _MSC_VER < 1300
#define __noop ((void)0)
#endif
#endif

#define TRACE		__noop
#define TRACEERROR	__noop
#define TRACE0		__noop
#define TRACE1		__noop
#define TRACE2		__noop
#define TRACERECT	__noop
#define TRACERGB	__noop
#define TRACEPOINT	__noop
#define TRACESIZE	__noop
#define TRACEHILO	__noop
#define TRACEDATE	__noop
#endif	//_DEBUG

#pragma warning(pop)

#endif //XTRACE_H
#endif


//////////////////////////////////////////////////////////////////////////
// ÄÚ´æÐ¹Â©¼ì²â

#ifdef _DEBUG
    #define _CRTDBG_MAP_ALLOC 
    #define _CRTDBG_MAP_ALLOC_NEW
    #include<stdlib.h> 
    #include<crtdbg.h>

    #define _QUIMemCheckPoint(s) \
        _CrtMemState __ui_##s; \
        _CrtMemCheckpoint(&(__ui_##s));

    #define _QUIIsMemLeak(s1,s2) \
         { \
             _CrtMemState __ui_s3;\
            if ( _CrtMemDifference(&__ui_s3, &(__ui_##s1),&(__ui_##s2)) )\
            {\
                TRACE(L"------------ÄÚ´æÐ¹Â©---------------\n");\
                _CrtMemDumpStatistics(&__ui_s3);\
                TRACE(L"-----------------------------------\n");\
                _ASSERT_EXPR(FALSE,L"·¢ÉúÄÚ´æÐ¹Â©£¡Çë¼ì²é!");\
             }\
         }
#else
    #define _QUIMemCheckPoint(s) 
    #define _QUIIsMemLeak(s1,s2)
#endif

