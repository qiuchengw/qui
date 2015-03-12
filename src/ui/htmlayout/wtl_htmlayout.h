//
// Windows Template Library Extension for
// Terra Informatica Lightweight Embeddable HTMLayout control
// http://htmlayout.com
//
// Written by Andrew Fedoniouk / <andrew@terrainformatica.com>
// Portions: Pete Kvitek <pete@kvitek.com>  
//
// This file is NOT part of Windows Template Library.
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
// (C) 2003, Andrew Fedoniouk <andrew@TerraInformatica.com>
//

#ifndef __WTL_HTMLAYOUT_H__
#define __WTL_HTMLAYOUT_H__

#pragma once

#ifndef __cplusplus
  #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
  #error wtl_htmlayout.h requires atlapp.h to be included first
#endif

#ifndef __ATLWIN_H__
  #error wtl_htmlayout.h requires atlwin.h to be included first
#endif

#include <atlmisc.h>

#if _ATL_VER >= 0x0700 
#define _Module _AtlBaseModule
#endif


#include "htmlayout.h"

/////////////////////////////////////////////////////////////////////////////
// Classes in this file
//
// CHTMLayoutCtrlT<TBase>
// CHTMLayoutDelegate
//

namespace WTL
{

bool GetHtmlResource(LPCTSTR pszName, /*out*/PBYTE& pb, /*out*/DWORD& cb, HMODULE hModule = NULL);


/////////////////////////////////////////////////////////////////////////////
// CHTMLayoutCtrl - client side for a HTMLayout control

template <class TBase>
class CHTMLayoutCtrlT : public TBase
{
public:

  // Constructors

  CHTMLayoutCtrlT(HWND hWnd = NULL) : TBase(hWnd) { }

  CHTMLayoutCtrlT< TBase >& operator=(HWND hWnd)
  {
    m_hWnd = hWnd;
    return *this;
  }

  // Attributes

 
  static LPCTSTR GetWndClassName()
  {
    return ::HTMLayoutClassNameT();
  }

  bool LoadHtml(LPCBYTE pb, DWORD nBytes, LPCWSTR baseUrl = 0)
  {
    SetupMediaType();
    ATLASSERT(::IsWindow(m_hWnd));
    return (baseUrl? 
      ::HTMLayoutLoadHtmlEx(m_hWnd, pb, nBytes, baseUrl):
      ::HTMLayoutLoadHtml(m_hWnd, pb, nBytes)) != 0;
  }

  bool LoadHtmlResource(LPCTSTR pszName, HMODULE hModule = NULL)
  {
    SetupMediaType();
    // This code assumes that the host and control windows are the same
    ATLASSERT(::IsWindow(m_hWnd));

    LPBYTE pb;
    DWORD  cb;

    if(!GetHtmlResource(pszName, pb, cb, hModule))
      return false;

    //if(strstr((const char*)pb,"<%include ") != NULL) - engine suppoort <include src=""> internally 
    //  return LoadHtmlResourceWithIncludes(pb, cb);

    return LoadHtml(pb, cb);
  }

  bool LoadHtmlResource(DWORD resID, HMODULE hModule = NULL)
  {
    return LoadHtmlResource(MAKEINTRESOURCE(resID), hModule);
  }


  HELEMENT GetRootElement()
  {
    HELEMENT he = 0;
    ::HTMLayoutGetRootElement(m_hWnd,&he);
    return he;
  }

  static inline void AppendBytes(CSimpleValArray<BYTE>& buf, const BYTE* pb, DWORD cb)
  {
    while(cb--)
      buf.Add(*pb++);
  }

  bool OpenFile(LPCTSTR lpszFilePath)
  {
    ATLASSERT(::IsWindow(m_hWnd));
    SetupMediaType();
    USES_CONVERSION;
    return ::HTMLayoutLoadFile(m_hWnd,T2W(const_cast<LPTSTR>(lpszFilePath))) == TRUE;
  }

  unsigned int GetDocumentMinWidth()      
  { 
    return ::HTMLayoutGetMinWidth(m_hWnd); 
  }

  unsigned int GetDocumentMinHeight(unsigned int width) 
  { 
    return ::HTMLayoutGetMinHeight(m_hWnd, width); 
  }

  // this function will return width of the HTMLayout window needed to 
  // show document in full width - which means without horizontal scrollbar 
  int     GetWindowWidth()  
  { 
    CRect rc(0,0,0,0);
    AdjustWindowRectEx(&rc, GetWindowLong(GWL_STYLE),GetMenu() != NULL, GetWindowLong(GWL_EXSTYLE));
    return GetDocumentMinWidth() + rc.Width();
  }

  // this function will return height of HTMLayout window for proposed width
  // needed to show the document in full thus without vertical scrollbar.
  int     GetWindowHeight(int windowWidth)  
  { 
    CRect rc(0,0,0,0);
    AdjustWindowRectEx(&rc, GetWindowLong(GWL_STYLE),GetMenu() != NULL, GetWindowLong(GWL_EXSTYLE));
    int clientWidth = windowWidth - rc.Width();
    int h = GetDocumentMinHeight(clientWidth);
    h += rc.Height(); // add borders extra
    if(clientWidth < (int)GetDocumentMinWidth()) // horz scrollbar will appear, add its height
      h += GetSystemMetrics(SM_CYHSCROLL);
    return h;
  }


  //  George [12/3/2005, 19:39]
  //  a few additions for methods not present at the time of writing
  
  void SetSelectionMode()
  {
    ATLASSERT(::IsWindow(m_hWnd));
    ::HTMLayoutSetMode(m_hWnd, 1);
  }
  void SetNavigationMode()
  {
    ATLASSERT(::IsWindow(m_hWnd));
    ::HTMLayoutSetMode(m_hWnd, 0);
  }

  bool CopySelection()
  {
    ATLASSERT(::IsWindow(m_hWnd));
    return ::HTMLayoutClipboardCopy(m_hWnd) != 0; // true is selection exist
  }

  // Methods

  bool CALLBACK HTMLayoutDataReady(LPCWSTR uri, LPBYTE data, DWORD dataLength)
  {
    return ::HTMLayoutDataReady(m_hWnd, uri, data, dataLength) != FALSE;
  }

  LPCBYTE GetSelectedHTML(LPUINT dataLength)
  {
    return HTMLayoutGetSelectedHTML(m_hWnd,dataLength);
  }

  LRESULT CALLBACK HTMLayoutProc(UINT msg, WPARAM wParam, LPARAM lParam)
  {
    return ::HTMLayoutProc(m_hWnd, msg, wParam, lParam);
  }

  void    SetupMediaType()
  {
#ifndef _WIN32_WCE
    ATLASSERT(::IsWindow(m_hWnd));
    SIZED_STRUCT(HIGHCONTRAST, hc);
    ::SystemParametersInfo(SPI_GETHIGHCONTRAST, 0, &hc, 0);
    LPCWSTR mediaType = hc.dwFlags & HCF_HIGHCONTRASTON?L"contrast-screen":L"screen";
    ::HTMLayoutSetMediaType(m_hWnd, mediaType);
#endif
  }
};

typedef CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE> CHTMLayoutWinTraits;
typedef CHTMLayoutCtrlT<CWindow> CHTMLayoutCtrl;

#ifndef RT_HTML
  #define RT_HTML         MAKEINTRESOURCE(23)
#endif

inline bool GetHtmlResource(LPCTSTR pszName, /*out*/PBYTE& pb, /*out*/DWORD& cb, HMODULE hModule)
{
  ATLASSERT(pszName != NULL);

  // Find specified resource and check if ok

  if(!hModule)
    hModule = _Module.GetResourceInstance();
  
  HRSRC hrsrc = ::FindResource(hModule, pszName, MAKEINTRESOURCE(RT_HTML));

  if(!hrsrc) 
    return false;

  // Load specified resource and check if ok
  
  HGLOBAL hgres = ::LoadResource(hModule, hrsrc);
  if(!hgres) return false;

  // Retrieve resource data and check if ok

  pb = (PBYTE)::LockResource(hgres); if (!pb) return false;
  cb = ::SizeofResource(hModule, hrsrc); if (!cb) return false;

  return true;
}

} //namespace WTL

//
// "Delegate" handling of painting,resizing and other stuff to HTMLayout winproc,
// Any CWindowImpl derived container can be converted into HTMLayout by 
// including CHAIN_TO_HTMLAYOUT() into message map
// Example: to assign HTML layout functionality to some Dialog it is enough 
//          to declare it as:
//
//    class CAboutDlg : 
//      public CDialogImpl<CAboutDlg, CHTMLayoutCtrl>,
//      public CHTMLayoutHost<CAboutDlg>, ...
//
//    ....
//
//    BEGIN_MSG_MAP(CMainDlg)
//       CHAIN_TO_HTMLAYOUT()
//       MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
//
// This macro must be first very first one in message map
#define CHAIN_TO_HTMLAYOUT() \
  { \
    BOOL bHandled = FALSE; \
    lResult = ::HTMLayoutProcND(hWnd,uMsg, wParam, lParam, &bHandled); \
    if(bHandled) return TRUE; \
} \

// GEORGE
#ifndef HTMENGINE_STATIC_LIB

// Link against HTMLayout library
#pragma comment(lib, "HTMLayout.lib")

#endif

#ifndef RT_HTML
#define RT_HTML         MAKEINTRESOURCE(23)
#endif

#endif // __WTL_HTMLayout_H__
