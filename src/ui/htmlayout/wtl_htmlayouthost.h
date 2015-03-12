//
// Windows Template Library Extension for
// Terra Informatica Lightweight Embeddable HTMLayout control
// http://terra-informatica.org/htmlayout
//
// Written by Andrew Fedoniouk and Pete Kvitek

//
// This file is NOT part of Windows Template Library.
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
// (C) 2003, Pete Kvitek <pete@kvitek.com>
//       and Andrew Fedoniouk <andrew@TerraInformatica.com>
//

#ifndef __WTL_HTMLAYOUTHOST_H__
#define __WTL_HTMLAYOUTHOST_H__

#pragma once

#ifndef __cplusplus
  #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
  #error wtl_htmlayouthost.h requires atlapp.h to be included first
#endif

#ifndef __ATLWIN_H__
  #error wtl_htmlayouthost.h requires atlwin.h to be included first
#endif

#include <atlmisc.h>

#ifdef _WIN32_WCE
#include <commctrl.h>
#define stricmp _stricmp
#else
#include <richedit.h>
#endif

#include "htmlayout.h"
#include "htmlayout_behavior.h"
#include "htmlayout_dom.hpp"
#include "htmlayout_behavior.hpp"
#include "behaviors/notifications.h"
//#include "xool/xool.h"

/////////////////////////////////////////////////////////////////////////////
// Classes in this file
//
// CHTMLayoutHost<T>
//

namespace WTL
{

/////////////////////////////////////////////////////////////////////////////
// CHTMLayoutHost - host side implementation for a HTMLayout control

template <class T>
class CHTMLayoutHost
{
public:

  // HTMLayout callback
  static LRESULT CALLBACK callback(UINT uMsg, WPARAM wParam, LPARAM lParam, LPVOID vParam)
  {
      ATLASSERT(vParam);
      CHTMLayoutHost<T>* pThis = (CHTMLayoutHost<T>*)vParam;
      return pThis->OnHtmlNotify(uMsg, wParam, lParam);
  }

  void SetCallback()
  {
    T* pT = static_cast<T*>(this);
    ATLASSERT(::IsWindow(pT->m_hWnd));
    ::HTMLayoutSetCallback(pT->m_hWnd,callback, (CHTMLayoutHost<T>*)this);
  }

  void SetEventHandler(htmlayout::event_handler* peh)
  {
    T* pT = static_cast<T*>(this);
    ATLASSERT(::IsWindow(pT->m_hWnd));
    HTMLayoutWindowAttachEventHandler(pT->m_hWnd, htmlayout::event_handler::element_proc,peh,peh->subscribed_to); 
  }

  // Overridables

  virtual LRESULT OnHtmlNotify(UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
    ATLASSERT(uMsg == WM_NOTIFY);

    // Crack and call appropriate method
    
    // here are all notifiactions
    switch(((NMHDR*)lParam)->code) 
    {
      case HLN_CREATE_CONTROL:    return OnCreateControl((LPNMHL_CREATE_CONTROL) lParam);
      case HLN_CONTROL_CREATED:   return OnControlCreated((LPNMHL_CREATE_CONTROL) lParam);
      case HLN_DESTROY_CONTROL:   return OnDestroyControl((LPNMHL_DESTROY_CONTROL) lParam);
      case HLN_LOAD_DATA:         return OnLoadData((LPNMHL_LOAD_DATA) lParam);
      case HLN_DATA_LOADED:       return OnDataLoaded((LPNMHL_DATA_LOADED)lParam);
      case HLN_DOCUMENT_COMPLETE: return OnDocumentComplete();
      case HLN_ATTACH_BEHAVIOR:   return OnAttachBehavior((LPNMHL_ATTACH_BEHAVIOR)lParam );
      case HLN_BEHAVIOR_CHANGED:
      case HLN_DIALOG_CREATED:
      case HLN_DIALOG_CLOSE_RQ:
      case HLN_DOCUMENT_LOADED:   return 0; // not used in this wrapper.

      // generic common control notifications:
      /* not used in the wrapper, so they will go to OnHtmlGenericNotifications
      case NM_CLICK:
      case NM_DBLCLK:
      case NM_RETURN:
      case NM_RCLICK:
      case NM_RDBLCLK:
      case NM_SETFOCUS:
      case NM_KILLFOCUS:
      case NM_NCHITTEST:
      case NM_KEYDOWN:
      case NM_RELEASEDCAPTURE:
      case NM_SETCURSOR:
      case NM_CHAR:
        etc.
      //default:
        //ATLASSERT(FALSE); */
    }
    return OnHtmlGenericNotifications(uMsg,wParam,lParam);
  }


  virtual LRESULT OnHtmlGenericNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
    // all generic notifications 
    // are coming to the parent of HTMLayout
    T* pT = static_cast<T*>(this);

    ATLASSERT(::IsWindow(pT->m_hWnd));
    // Pass it to the parent window if any
    HWND hWndParent = pT->GetParent();
    if (!hWndParent) return 0;

    return ::SendMessage(hWndParent, uMsg, wParam, lParam);
  }

 
  virtual LRESULT OnCreateControl(LPNMHL_CREATE_CONTROL pnmcc)
  {
    // ATLTRACE(_T("CHTMLayoutHost::OnCreateControl: type='%s' \n"), GetElementType(pnmcc->helement) );

    // Try to create control and if failed, proceed with default processing.
    // Note that this code assumes that the host and control windows are the same. If
    // you are handling HTMLayout control notification in another window, you'll have
    // to override this method and provide proper hWnd.

    T* pT = static_cast<T*>(this);
    
    ATLASSERT(::IsWindow(pT->m_hWnd));

    return CreateControl(pT->m_hWnd, pnmcc);
  }

  virtual LRESULT OnControlCreated(LPNMHL_CREATE_CONTROL pnmcc)
  {
//    ATLTRACE(_T("CHTMLayoutHost::OnControlCreated: type='%s' \n"), GetElementType(pnmcc->helement) );
    return 0;
  }

  virtual LRESULT OnDestroyControl(LPNMHL_DESTROY_CONTROL pnmhl)
  {
    ATLTRACE(_T("CHTMLayoutHost::OnDestroyControl: HWND=%x\n"), pnmhl->inoutControlHwnd);
    
    //  use this if you don't want this child to be destroyed:
    //  pnmhl->inoutControlHwnd = 0;

    //  If you will not change pnmhl->inoutControlHwnd field then HTMLayout 
    //  will call ::DestroyWindow by itself.

    return 0;
  }


  virtual LRESULT OnLoadData(LPNMHL_LOAD_DATA pnmld)
  {
    ATLTRACE(_T("CHTMLayoutHost::OnLoadData: uri='%s'\n"), (pnmld->uri));

    // Try to load data from resource and if failed, proceed with default processing.
    // Note that this code assumes that the host and control windows are the same. If
    // you are handling HTMLayout control notification in another window, you'll have
    // to override this method and provide proper hWnd.

    T* pT = static_cast<T*>(this);
    
    ATLASSERT(::IsWindow(pT->m_hWnd));

    return LoadResourceData(pT->m_hWnd, pnmld->uri);
  }

  virtual LRESULT OnDataLoaded(LPNMHL_DATA_LOADED pnmld)
  {
    ATLTRACE(_T("CHTMLayoutHost::OnDataLoaded: uri='%s'\n"), pnmld->uri);
    return 0;
  }

  virtual LRESULT OnDocumentComplete()
  {
    ATLTRACE(_T("CHTMLayoutHost::OnDocumentComplete\n"));
    return 0;
  }

  virtual LRESULT OnAttachBehavior( LPNMHL_ATTACH_BEHAVIOR lpab )
  {
    htmlayout::behavior::handle(lpab);
    return 0;
  }

  // Define module manager that will free loaded module upon exit
struct Module 
  {
    HMODULE hModule;
    bool freeOnDestruct;
    inline Module() : hModule(_Module.GetResourceInstance()), freeOnDestruct(false) {}
    inline ~Module() { if (freeOnDestruct && hModule) ::FreeLibrary(hModule); }
    HMODULE Load(LPCTSTR pszModule, DWORD flags) { freeOnDestruct = true; return hModule = ::LoadLibraryEx(pszModule, 0, flags); }
    operator HMODULE() const { return hModule; }
  };

  // Load Data helper routines

#ifndef INVALID_FILE_ATTRIBUTES //wince has not it
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF
#endif


  static LRESULT LoadResourceData(HWND hWnd, LPCSTR uri )
  {
     USES_CONVERSION;
     return LoadResourceData(hWnd,A2CW(uri));
  }

  static LRESULT LoadResourceData(HWND hWnd, LPCWSTR uri )
  {
    USES_CONVERSION;

    ATLASSERT(::IsWindow(hWnd));
    // Check for trivial case

    if (!uri || !uri[0]) return LOAD_DISCARD;

    if (wcsncmp( uri, L"file:", 5 ) == 0 )
      return LOAD_OK;
    if (wcsncmp( uri, L"http:", 5 ) == 0 )
      return LOAD_OK;
    if (wcsncmp( uri, L"https:", 6 ) == 0 )
      return LOAD_OK;

    if (wcsncmp( uri, L"res:", 4 ) == 0 )
    {
      uri += 4;
    }
    //ATTN: you may wish to uncomment this 'else' and it will go further *only if* "res:...." url requested  
    //else 
    //  return LOAD_OK;


    // Retrieve url specification into a local storage since FindResource() expects 
    // to find its parameters on stack rather then on the heap under Win9x/Me

    TCHAR achURL[MAX_PATH]; lstrcpyn(achURL, W2CT(uri), MAX_PATH);

    Module module;

    // Separate name and handle external resource module specification

    LPTSTR psz, pszName = achURL;
    if ((psz = _tcsrchr(pszName, '/')) != NULL) {
      LPTSTR pszModule = pszName; pszName = psz + 1; *psz = '\0';
      DWORD dwAttr = ::GetFileAttributes(pszModule);
      if (dwAttr != INVALID_FILE_ATTRIBUTES && !(dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
        module.Load(pszModule, LOAD_LIBRARY_AS_DATAFILE);
      }
    }

    // Separate extension if any

    LPTSTR pszExt = _tcsrchr(pszName, '.'); if (pszExt) *pszExt++ = '\0';

    // Find specified resource and leave if failed. Note that we use extension
    // as the custom resource type specification or assume standard HTML resource 
    // if no extension is specified

    HRSRC hrsrc = 0;
    bool  isHtml = false;
#ifndef _WIN32_WCE
    if( pszExt == 0 || _tcsicmp(pszExt,TEXT("HTML")) == 0)
    {
      hrsrc = ::FindResource(module, pszName, RT_HTML);
      isHtml = true;
    }
    else
      hrsrc = ::FindResource(module, pszName, pszExt);
#else 
      hrsrc = ::FindResource(module, pszName, pszExt);
#endif

    if (!hrsrc) return LOAD_OK; // resource not found here - proceed with default loader

    // Load specified resource and check if ok

    HGLOBAL hgres = ::LoadResource(module, hrsrc);
    if (!hgres) return LOAD_DISCARD;

    // Retrieve resource data and check if ok

    PBYTE pb = (PBYTE)::LockResource(hgres); if (!pb) return LOAD_DISCARD;
    DWORD cb = ::SizeofResource(module, hrsrc); if (!cb) return LOAD_DISCARD;

    // Report data ready

    ::HTMLayoutDataReady(hWnd, uri, pb,  cb);
    
    return LOAD_OK;
  }

  LRESULT LoadResourceData(LPNMHL_LOAD_DATA pnmld)
  {
    // This code assumes that the host and control windows are the same

    T* pT = static_cast<T*>(this);
    
    ATLASSERT(::IsWindow(pT->m_hWnd));

    return LoadResourceData(pT->m_hWnd, pnmld->uri);
  }

  virtual LRESULT CreateControl(HWND hWnd, LPNMHL_CREATE_CONTROL pnmcc)
  {
    ATLASSERT(::IsWindow(hWnd));
    ATLASSERT(pnmcc != NULL);

    return 0;
    
  }
};

}; //namespace WTL

// George - 11.Mar.2005
#ifdef _ATL
#if _ATL_VER >= 7
#undef _Module
#endif
#endif

#endif // __ATLHTMENGINEHOST_H__
