#include "stdafx.h"
#include "WebUIController.h"

CWebUIController::CWebUIController()
: m_uRefCount(0)
, m_pWebBrowser2(NULL)
, m_dwCookie(0)
, m_pExternalDocHostUIEventHander(NULL)
, m_pBrowserEventSink(NULL)
, m_bEnable3DBorder(TRUE)
, m_bEnableScrollBar(TRUE)
{

}

CWebUIController::~CWebUIController()
{

}

STDMETHODIMP CWebUIController::GetIDsOfNames( REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgDispId )
{
    if (m_pExternalDocHostUIEventHander)
    {
        return m_pExternalDocHostUIEventHander->GetIDsOfJSFunc(riid, rgszNames, cNames, lcid, rgDispId);
    }
    return E_NOTIMPL;
}

STDMETHODIMP CWebUIController::Invoke( DISPID dispidMember,REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams,
                                      VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr )
{
    if(!pDispParams)
        return E_INVALIDARG;

    switch(dispidMember)
    {
        // The parameters for this DISPID are as follows:
        // [0]: URL to navigate to - VT_BYREF|VT_VARIANT
        // [1]: An object that evaluates to the top-level or frame
        // WebBrowser object corresponding to the event. 
    case DISPID_NAVIGATECOMPLETE2:
        // The IDocHostUIHandler association must be set
        // up every time we navigate to a new page.
        if(m_pBrowserEventSink)
        {
            m_pBrowserEventSink->NavigateComplete2(pDispParams->rgvarg[1].pdispVal, pDispParams->rgvarg[0].pvarVal);
        }
        SetCustomDoc(pDispParams->rgvarg[1].pdispVal);
        break;
    case DISPID_NAVIGATEERROR:
        if(m_pBrowserEventSink)
        {
            return m_pBrowserEventSink->NavigateError( pDispParams->rgvarg[4].pdispVal, pDispParams->rgvarg[3].pvarVal,
                pDispParams->rgvarg[2].pvarVal, pDispParams->rgvarg[1].pvarVal, pDispParams->rgvarg[0].pboolVal);
        }
        else
        {
            return E_NOTIMPL;
        }
        break;
    case DISPID_NEWWINDOW3:
        if(m_pBrowserEventSink)
        {
            return m_pBrowserEventSink->NewWindow3( pDispParams->rgvarg[4].ppdispVal, pDispParams->rgvarg[3].pboolVal, pDispParams->rgvarg[2].uintVal,
                pDispParams->rgvarg[1].bstrVal, pDispParams->rgvarg[0].bstrVal);
        }
        else
        {
            return E_NOTIMPL;
        }
        break;
    default:
        break;
    }
    if (m_pExternalDocHostUIEventHander)
    {
        if (dispidMember >= BASE_DIPSPID)
            return m_pExternalDocHostUIEventHander->InvokeJsCall(dispidMember, pDispParams, pvarResult);
    }
    return S_OK;
}

STDMETHODIMP CWebUIController::GetHostInfo( DOCHOSTUIINFO FAR* pInfo )
{
    if(pInfo != NULL)
    {
        pInfo->dwFlags |= (m_bEnable3DBorder ? 0 : DOCHOSTUIFLAG_NO3DBORDER);
        pInfo->dwFlags |= (m_bEnableScrollBar ? 0 : DOCHOSTUIFLAG_SCROLL_NO);
    }

    return S_OK;
}

HRESULT CWebUIController::SetWebBrowser( IWebBrowser2* pWebBrowser2 )
{
    // Unadvise the event sink, if there was a
    // previous reference to the WebBrowser control.
    if(m_pWebBrowser2)
    {
        _CoUnadvise(m_pWebBrowser2, DIID_DWebBrowserEvents2, m_dwCookie);
        m_dwCookie = 0;

        m_pWebBrowser2->Release();
    }

    m_pWebBrowser2 = pWebBrowser2;
    if(pWebBrowser2 == NULL)
        return S_OK;

    m_pWebBrowser2->AddRef();

    return _CoAdvise(m_pWebBrowser2, (IDispatch*)this, DIID_DWebBrowserEvents2, &m_dwCookie);
}

HRESULT CWebUIController::SetExternalUIEventHandler( IDocHostUIEventHandler* pExternalDocHostUIHandler )
{
    if (pExternalDocHostUIHandler)
    {
        m_pExternalDocHostUIEventHander = pExternalDocHostUIHandler;
        return S_OK;
    }
    return E_FAIL;
}


HRESULT CWebUIController::SetBrowserEventSink( IBrowserEventSink* pBrowserEventSink )
{
    if (pBrowserEventSink)
    {
        m_pBrowserEventSink = pBrowserEventSink;
        return S_OK;
    }
    return E_FAIL;
}

void CWebUIController::SetCustomDoc( LPDISPATCH lpDisp )
{
    if(lpDisp == NULL)
        return;

    IWebBrowser2* pWebBrowser2 = NULL;
    HRESULT hr = lpDisp->QueryInterface(IID_IWebBrowser2, (void**)&pWebBrowser2);

    if(SUCCEEDED(hr) && pWebBrowser2)
    {
        IDispatch* pDoc = NULL;
        hr = pWebBrowser2->get_Document(&pDoc);

        if(SUCCEEDED(hr) && pDoc)
        {
            ICustomDoc* pCustDoc = NULL;
            hr = pDoc->QueryInterface(IID_ICustomDoc, (void**)&pCustDoc);
            if(SUCCEEDED(hr) && pCustDoc != NULL)
            {
                pCustDoc->SetUIHandler(this);
                pCustDoc->Release();
            }

            pDoc->Release();
        }

        pWebBrowser2->Release();
    }
}

STDMETHODIMP CWebUIController::ShowContextMenu( DWORD dwID, POINT FAR* ppt, IUnknown FAR* pcmdtReserved, IDispatch FAR* pdispReserved )
{
    if (m_pExternalDocHostUIEventHander)
    {
        return m_pExternalDocHostUIEventHander->OnShowContextMenu(dwID, ppt, pcmdtReserved, pdispReserved);
    }
    return S_OK;
}