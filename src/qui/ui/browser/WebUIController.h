// class name: CWebUIController
// author: Dandy Cheung
// email: dandycheung@21cn.com
// date: 2005-3-18

#ifndef __WEBUICONTROLLER_H__
#define __WEBUICONTROLLER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MSHTMHST.H>
#include <EXDISP.H>
#include <EXDISPID.H>
#include "string"
#include "map"

static const DISPID BASE_DIPSPID = 100000;


#define CONNECT_JS_CALL_HANDLER(name, func) ConnectHandler(L#name, (CallbackFunction)&func)
#define CONNECT_JS_CALL_SIMPLE_HANDLER(name, func) ConnectHandler(L#name, (CallbackFunction)&func, true)


#define DECLARE_JSCALL_DESCRIPTION(x)             \
public:                                         \
    typedef void (x::*CallbackFunction)(DISPPARAMS* params, VARIANT* retval);    \
    typedef const char* (x::*SimpleCallbackFunction)(int argc, const char** argv);   \
    void ConnectHandler(BSTR name, CallbackFunction func, bool simple = false); \
private:    \
    DISPID NameId(BSTR name) const; \
    virtual HRESULT GetIDsOfJSFunc(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgDispId); \
    virtual HRESULT InvokeJsCall(DISPID id, DISPPARAMS* params, VARIANT* retval);   \
    char* InPlaceWToA(BSTR bstr) const; \
    struct JsCallHandler { DISPID id; union { CallbackFunction normal; SimpleCallbackFunction simple; } callback; bool simple; };  \
    typedef std::map<std::wstring, JsCallHandler> JsHandlers;   \
    JsHandlers jsHandlers;  \

#define IMPLEMENT_JSCALL_DESCRIPTION(x)             \
void x::ConnectHandler(BSTR name, CallbackFunction func, bool simple /*= false*/ ) {   \
    ATLASSERT(jsHandlers.find(name) == jsHandlers.end());   \
    JsCallHandler handler;  \
    handler.id = BASE_DIPSPID + (DISPID)jsHandlers.size();  \
    handler.callback.normal = func; \
    handler.simple = simple;    \
    jsHandlers[name] = handler; }\
DISPID x::NameId( BSTR name ) const {   \
    JsHandlers::const_iterator it;  \
    it = jsHandlers.find(name); \
    if(it == jsHandlers.end()) { ATLTRACE(name) ; return -1; } \
    return it->second.id;   }   \
HRESULT x::InvokeJsCall( DISPID id, DISPPARAMS* params, VARIANT* retval ) { \
    JsHandlers::iterator it; \
    for (it = jsHandlers.begin(); it->second.id != id && it != jsHandlers.end(); it ++);    \
    if (it == jsHandlers.end()) return E_NOTIMPL;   \
    if (! it->second.simple) {  \
        (this->*it->second.callback.normal)(params, retval); }  \
    else {  \
        int count = params->cArgs;  \
        if (count > 256) return E_INVALIDARG;   \
        const char* argv[256] = {0}; count --;  \
        for (int i=0; count >= 0; i++, count--) \
            argv[i] = InPlaceWToA(params->rgvarg[count].bstrVal);   \
        const char* retstr = (this->*it->second.callback.simple)(params->cArgs, argv);  \
        if (retval) { CComVariant res(static_cast<LPSTR>((LPSTR)retstr)); res.Detach(retval); } \
    }   \
    return S_OK;    }   \
char* x::InPlaceWToA( BSTR bstr ) const {   \
    if (bstr[0] > 0xFF) return (char*)bstr; \
    WCHAR* wp = bstr;       char* p = (char*)bstr;    while (*(p++) = (char)*(wp++));\
    return (char*)bstr; } \
HRESULT x::GetIDsOfJSFunc( REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgDispId ){ \
     if (jsHandlers.size() < 1) return E_NOTIMPL; *rgDispId = NameId(rgszNames[0]); if(*rgDispId == -1) return E_NOTIMPL;  return S_OK; }


interface IDocHostUIEventHandler
{
    // DocHostUIHandler overrideables
    virtual HRESULT OnShowContextMenu(DWORD dwID, LPPOINT ppt, LPUNKNOWN pcmdtReserved, LPDISPATCH pdispReserved) { return S_OK;}
    virtual HRESULT OnGetExternal(LPDISPATCH *lppDispatch) { return E_NOTIMPL;}
    virtual HRESULT OnGetHostInfo(DOCHOSTUIINFO *pInfo) { return E_NOTIMPL;}
    virtual HRESULT OnShowUI(DWORD dwID, LPOLEINPLACEACTIVEOBJECT pActiveObject, LPOLECOMMANDTARGET pCommandTarget, LPOLEINPLACEFRAME pFrame, LPOLEINPLACEUIWINDOW pDoc) { return E_NOTIMPL;}
    virtual HRESULT OnHideUI() { return E_NOTIMPL;}
    virtual HRESULT OnUpdateUI() { return E_NOTIMPL;}
    virtual HRESULT OnEnableModeless(BOOL fEnable) { return E_NOTIMPL;}
    virtual HRESULT OnDocWindowActivate(BOOL fActivate) { return E_NOTIMPL;}
    virtual HRESULT OnFrameWindowActivate(BOOL fActivate) { return E_NOTIMPL;}
    virtual HRESULT OnResizeBorder(LPCRECT prcBorder, LPOLEINPLACEUIWINDOW pUIWindow, BOOL fFrameWindow) { return E_NOTIMPL;}
    virtual HRESULT OnTranslateAccelerator(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID) { return E_NOTIMPL;}
    virtual HRESULT OnGetOptionKeyPath(LPOLESTR* pchKey, DWORD dwReserved) { return E_NOTIMPL;}
    virtual HRESULT OnFilterDataObject(LPDATAOBJECT pDataObject, LPDATAOBJECT* ppDataObject) { return E_NOTIMPL;}
    virtual HRESULT OnTranslateUrl(DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut) { return E_NOTIMPL;}
    virtual HRESULT OnGetDropTarget(LPDROPTARGET pDropTarget, LPDROPTARGET* ppDropTarget) { return E_NOTIMPL;}

    virtual HRESULT InvokeJsCall(DISPID id, DISPPARAMS* params, VARIANT* retval) { return E_NOTIMPL;}
    virtual HRESULT GetIDsOfJSFunc(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgDispId) { return E_NOTIMPL;}
};

interface IBrowserEventSink
{
    virtual HRESULT BeforeNavigate2( IDispatch *pDisp,VARIANT *&url,VARIANT *&Flags,VARIANT *&TargetFrameName,VARIANT *&PostData,VARIANT *&Headers,VARIANT_BOOL *&Cancel ) { return E_NOTIMPL; }
    virtual HRESULT NavigateError(IDispatch *pDisp,VARIANT * &url,VARIANT *&TargetFrameName,VARIANT *&StatusCode,VARIANT_BOOL *&Cancel)  { return E_NOTIMPL; }
    virtual HRESULT NavigateComplete2(IDispatch *pDisp,VARIANT *&url) { return E_NOTIMPL; }
    virtual HRESULT ProgressChange(LONG nProgress, LONG nProgressMax) { return E_NOTIMPL; }
    virtual HRESULT NewWindow3(IDispatch **pDisp, VARIANT_BOOL *&Cancel, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl) { return E_NOTIMPL; }
    virtual HRESULT CommandStateChange(long Command,VARIANT_BOOL Enable) { return E_NOTIMPL; }
};

inline HRESULT _CoAdvise(IUnknown* pUnkCP, IUnknown* pUnk, const IID& iid, LPDWORD pdw)
{
    IConnectionPointContainer* pCPC = NULL;
    IConnectionPoint* pCP = NULL;
    HRESULT hRes = pUnkCP->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC);
    if(SUCCEEDED(hRes) && pCPC != NULL)
    {
        hRes = pCPC->FindConnectionPoint(iid, &pCP);
        if(SUCCEEDED(hRes) && pCP != NULL)
        {
            hRes = pCP->Advise(pUnk, pdw);
            pCP->Release();
        }
        pCPC->Release();
    }
    return hRes;
}

inline HRESULT _CoUnadvise(IUnknown* pUnkCP, const IID& iid, DWORD dw)
{
    IConnectionPointContainer* pCPC = NULL;
    IConnectionPoint* pCP = NULL;
    HRESULT hRes = pUnkCP->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC);
    if(SUCCEEDED(hRes) && pCPC != NULL)
    {
        hRes = pCPC->FindConnectionPoint(iid, &pCP);
        if(SUCCEEDED(hRes) && pCP != NULL)
        {
            hRes = pCP->Unadvise(dw);
            pCP->Release();
        }
        pCPC->Release();
    }
    return hRes;
}

class CWebUIController : public DWebBrowserEvents2, public IDocHostUIHandler
{
public:
    CWebUIController();
    virtual ~CWebUIController();

protected:
    // IUnknown Methods
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject)
    {
        *ppvObject = NULL;

        if(IsEqualGUID(riid, DIID_DWebBrowserEvents2) ||
            IsEqualGUID(riid, IID_IDispatch))
        {
            *ppvObject = (DWebBrowserEvents2*)this;
            AddRef();
            return S_OK;
        }
        else if(IsEqualGUID(riid, IID_IDocHostUIHandler) ||
            IsEqualGUID(riid, IID_IUnknown))
        {
            *ppvObject = (IDocHostUIHandler*)this;
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)(void)
    {
        m_uRefCount++;
        return m_uRefCount;
    }

    STDMETHOD_(ULONG, Release)(void)
    {
        m_uRefCount--;
        ULONG uRefCount = m_uRefCount;
        if(uRefCount == 0)
            delete this;
        return uRefCount;
    }

    // IDispatch Methods
    STDMETHOD(GetTypeInfoCount)(unsigned int FAR* pctinfo) { return E_NOTIMPL; }
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo) { return E_NOTIMPL; }
    STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgDispId);
    STDMETHOD(Invoke)(DISPID dispidMember,REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pvarResult,
        EXCEPINFO* pexcepinfo, UINT* puArgErr);

    // IDocHostUIHandler Methods
protected:
    STDMETHOD(ShowContextMenu)(DWORD dwID, POINT FAR* ppt, IUnknown FAR* pcmdtReserved, IDispatch FAR* pdispReserved);
    STDMETHOD(GetHostInfo)(DOCHOSTUIINFO FAR* pInfo);
    STDMETHOD(ShowUI)(DWORD dwID, IOleInPlaceActiveObject FAR* pActiveObject, IOleCommandTarget FAR* pCommandTarget,
        IOleInPlaceFrame FAR* pFrame, IOleInPlaceUIWindow FAR* pDoc) { return E_NOTIMPL; }
    STDMETHOD(HideUI)(void) { return E_NOTIMPL; }
    STDMETHOD(UpdateUI)(void) { return E_NOTIMPL; }
    STDMETHOD(EnableModeless)(BOOL fEnable) { return E_NOTIMPL; }
    STDMETHOD(OnDocWindowActivate)(BOOL fActivate) { return E_NOTIMPL; }
    STDMETHOD(OnFrameWindowActivate)(BOOL fActivate) { return E_NOTIMPL; }
    STDMETHOD(ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow FAR* pUIWindow, BOOL fRameWindow) { return E_NOTIMPL; }
    STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID FAR* pguidCmdGroup, DWORD nCmdID) { return E_NOTIMPL; }
    STDMETHOD(GetOptionKeyPath)(LPOLESTR FAR* pchKey, DWORD dw) { return E_NOTIMPL; }
    STDMETHOD(GetDropTarget)(IDropTarget* pDropTarget, IDropTarget** ppDropTarget) { return E_NOTIMPL; }
    STDMETHOD(GetExternal)(IDispatch** ppDispatch) { *ppDispatch = this; AddRef(); return S_OK; }
    STDMETHOD(TranslateUrl)(DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut) { return E_NOTIMPL; }
    STDMETHOD(FilterDataObject)(IDataObject* pDO, IDataObject** ppDORet) { return E_NOTIMPL; }

public:
    HRESULT SetWebBrowser(IWebBrowser2* pWebBrowser2);
    HRESULT SetExternalUIEventHandler(IDocHostUIEventHandler* pExternalDocHostUIHandler);
    HRESULT SetBrowserEventSink(IBrowserEventSink* pBrowserEventSink);
    void Enable3DBorder(BOOL bEnable = TRUE) { m_bEnable3DBorder = bEnable; }
    void EnableScrollBar(BOOL bEnable = TRUE) { m_bEnableScrollBar = bEnable; }

private:
    void SetCustomDoc(LPDISPATCH lpDisp);

private:
    IDocHostUIEventHandler* m_pExternalDocHostUIEventHander;
    IBrowserEventSink*      m_pBrowserEventSink;
    IWebBrowser2*           m_pWebBrowser2;
    ULONG   m_uRefCount;
    DWORD   m_dwCookie;
    BOOL    m_bEnable3DBorder;
    BOOL    m_bEnableScrollBar;
};

#endif // __WEBUICONTROLLER_H__