#pragma once
#include "ui/QView.h"
#include "WebUIController.h"

class CBrowserWindow
    : public CWindowImpl<CBrowserWindow, CAxWindow>
    , public IDocHostUIEventHandler
    , public IBrowserEventSink
{
    BEGIN_MSG_MAP(CBrowserWindow)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
    END_MSG_MAP()

public:
    CBrowserWindow();

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
    void EnableScrollBar(BOOL bEnableScrollBar = TRUE);
    void SetEventSink(IBrowserEventSink* pBrowserEventSink);
    void SetUIEventHandler(IDocHostUIEventHandler* pExternalDocHostUIHandler);
    void CallJsFunc(LPCWSTR lpszFunc, LPCWSTR lpszParam);

    virtual void Navigate2(LPCWSTR lpszURL);
    virtual void GoBack();
    virtual bool CanGoBack();
    virtual void GoForward();
    virtual bool CanGoForward();
    virtual void Stop();
    virtual void Refresh();
protected:
    CComPtr<IWebBrowser2>   m_pWebBrowser;
    CWebUIController        webUICtrl_;
    bool                    enableScrollBar_;
};

class CWebBrowser : public QView
{
    QUI_DECLARE_EVENT_MAP
    BEGIN_MSG_MAP_EX(CWebBrowser)
        MSG_WM_DESTROY(OnDestroy)
        CHAIN_MSG_MAP(QView)
    END_MSG_MAP()
public:
    CWebBrowser(void);
    ~CWebBrowser(void);

    BOOL GotoURL(LPCWSTR pszURL);

    // ÏÔÊ¾ÄÚ´æhtml
    BOOL DisplayHtml(LPCWSTR pszHtmlBuffer);
    void SetSize(int cx, int cy);
    void CallJsFunc(LPCWSTR lpszFunc, LPCWSTR lpszParam);
    void SetUIEventHandler(IDocHostUIEventHandler* handler);

protected:
    void OnDestroy();

    virtual LRESULT CreateControl(HWND hWnd, LPNMHL_CREATE_CONTROL pcc);

    void OnBtnClkOpenFile(HELEMENT);
    void OnBtnClkGoBack(HELEMENT);
    void OnBtnClkGoNext(HELEMENT);
    void OnBtnClkGoStop(HELEMENT);
    void OnBtnClkRefresh(HELEMENT);
    void OnBtnClkZoom100(HELEMENT);
private:
    CBrowserWindow browser_;
};

