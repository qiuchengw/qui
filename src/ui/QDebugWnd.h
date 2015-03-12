#pragma once

#include "QWindow.h"
#include "QUIMgr.h"


//#define QWND_DEBUG

#ifdef QWND_DEBUG
#	define TRACE_TO_WND XDebug()
#else
#	define TRACE_TO_WND
#endif

class XDebug;
class QDebugWnd : public QFrame
{
	friend XDebug;
private:
	QDebugWnd():QFrame(L"qabs:common/Debug.htm") {  }

public:
	void AddString(LPCWSTR szStr)
	{
		if (!IsWindow())
		{
			if (Create(NULL,WS_POPUP,WS_EX_TOOLWINDOW|WS_EX_TOPMOST,0))
			{
				ShowWindow(SW_SHOW);
			}
		}
		ECtrl m_ctlBox = GetCtrl("#string-box");
		if (m_ctlBox.is_valid())
		{
			ECtrl eDiv = ECtrl::create("div");
			m_ctlBox.append(eDiv);
			utf8::ostream o;
			o<<szStr;
			eDiv.set_html(o.data(),o.length());
		}
	}

	virtual BOOL on_event(HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason )
	{
		if (BUTTON_CLICK != type)
			return false;
		dom::element el(target);
		if (aux::wcseqi(el.get_attribute("id"),L"btn_clear_string"))
			m_ctlBox.clear();
		return true;
	}
	
private:
	ECtrl		m_ctlBox;
};

class XDebug
{
public:
	  void operator() (LPCTSTR lpszFormat, ...)
	  {
		  va_list va;
		  va_start(va, lpszFormat);

		  QString str;
		  str.Format(L"<b .green>%s-线程[%d]：</b><br/>%s",
			  QTime::GetCurrentTime().Format(L"%c"),
			  GetCurrentThreadId(),lpszFormat);

		  // format the message as requested
		  TCHAR buf2[BUFFER_SIZE];
		  _vsntprintf_s(buf2, BUFFER_SIZE-1, (LPCWSTR)str, va);
		  buf2[_countof(buf2)-1] = _T('\0');

		  va_end(va);

		  // 写入到文本中
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

		  // 显示到界面
		  m_wnd.AddString(buf2);
	  }

private:
	enum	{ BUFFER_SIZE = 4096 };
	QDebugWnd		m_wnd;
};


