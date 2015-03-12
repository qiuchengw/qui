#include "stdafx.h"
#include "WebBrowser.h"
#include "ui/ECtrl.h"

CBrowserWindow::CBrowserWindow()
    : enableScrollBar_(TRUE)
{

}

void CBrowserWindow::EnableScrollBar(BOOL bEnableScrollBar/* = TRUE*/)
{
    enableScrollBar_= bEnableScrollBar;
}

LRESULT CBrowserWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
    LRESULT lRet;
    lRet = DefWindowProc(uMsg, wParam, lParam);
    // Create WebBrowser object
    CreateControl(L"{8856F961-340A-11D0-A96B-00C04FD705A2}");
    // Connect event
    HRESULT hRet = QueryControl(IID_IWebBrowser2, (void**)&this->m_pWebBrowser);
    if (FAILED(hRet))
    {
        return -1;
    }
    m_pWebBrowser->Navigate(_T("about:blank"), NULL, NULL, NULL, NULL);

    webUICtrl_.Enable3DBorder(FALSE);              // 禁止三维边框
    webUICtrl_.EnableScrollBar(enableScrollBar_);  // 此代码禁止滚动条
    webUICtrl_.SetWebBrowser(m_pWebBrowser);

    m_pWebBrowser->put_Silent(VARIANT_TRUE);       // 禁止包括脚本错误和其它对话框
    m_pWebBrowser->put_Offline(VARIANT_FALSE);

    SetUIEventHandler(this);
    SetEventSink(this);
    return S_OK;
}

void CBrowserWindow::Navigate2(LPCWSTR lpszURL)
{
    CComBSTR bstr = lpszURL;
    m_pWebBrowser->Navigate(bstr, NULL, NULL, NULL, NULL);
}

void CBrowserWindow::GoBack()
{
    m_pWebBrowser->GoBack();
}

bool CBrowserWindow::CanGoBack()
{
    return false;
}

void CBrowserWindow::GoForward()
{
    m_pWebBrowser->GoForward();
}

void CBrowserWindow::Refresh()
{
    m_pWebBrowser->Refresh();
}

void CBrowserWindow::Stop()
{
    m_pWebBrowser->Stop();
}

bool CBrowserWindow::CanGoForward()
{
    return false;
}

void CBrowserWindow::SetEventSink(IBrowserEventSink* pBrowserEventSink)
{
    webUICtrl_.SetBrowserEventSink(pBrowserEventSink);
}

void CBrowserWindow::SetUIEventHandler(IDocHostUIEventHandler* pExternalDocHostUIHandler)
{
    webUICtrl_.SetExternalUIEventHandler(pExternalDocHostUIHandler);
}

void CBrowserWindow::CallJsFunc(LPCWSTR lpszFunc, LPCWSTR lpszParam)
{
    IHTMLDocument2 *pHTMLDoc = NULL;
    IDispatch* pDoc = NULL;
    HRESULT hr = m_pWebBrowser->get_Document(&pDoc);
    hr = pDoc->QueryInterface(IID_IHTMLDocument2, (void**)&pHTMLDoc);
    if (FAILED(hr))
    {
        return;
    }

    if ( pHTMLDoc != NULL  ) 
    {
        IHTMLWindow2*  pHTMLWnd; 
        hr = pHTMLDoc->get_parentWindow( &pHTMLWnd );
        if (FAILED(hr))
        {
            return;
        }

        CComVariant ret;
        ret.vt =  VT_EMPTY;
        if (pHTMLWnd != NULL)
        {
            CStdString strJSFunc;
            strJSFunc.Format(_T("%s(%s)"), lpszFunc, lpszParam);
            pHTMLWnd->execScript(CComBSTR(strJSFunc), CComBSTR("JScript"), &ret);
            pHTMLWnd->Release();
        } 
        pHTMLDoc->Release();
    }
}

//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(CWebBrowser,QView)
    //    BN_CLICKED_ID(L"BTN-OPENFILE",&CWebBrowser::OnBtnClkOpenFile)
    BN_CLICKED_ID(L"BTN-GOBACK",&CWebBrowser::OnBtnClkGoBack)
    BN_CLICKED_ID(L"BTN-GONEXT",&CWebBrowser::OnBtnClkGoNext)
    BN_CLICKED_ID(L"BTN-STOP",&CWebBrowser::OnBtnClkGoStop)
    BN_CLICKED_ID(L"BTN-REFRESH",&CWebBrowser::OnBtnClkRefresh)
    BN_CLICKED_ID(L"BTN-ZOOM100",&CWebBrowser::OnBtnClkZoom100)
QUI_END_EVENT_MAP()

CWebBrowser::CWebBrowser(void)
    :QView(L"qabs:common/Browser.htm")
{

}

CWebBrowser::~CWebBrowser(void)
{

}

BOOL CWebBrowser::GotoURL(LPCWSTR pszURL)
{
    browser_.Navigate2(pszURL);
    return FALSE;
}

LRESULT CWebBrowser::CreateControl( HWND hWnd, LPNMHL_CREATE_CONTROL pcc )
{
    LPCWSTR pszType = ECtrl(pcc->helement).get_attribute("type");
    if ( StrCmpIW(pszType,L"browser") == 0 )
    {
        CRect rcClient = (0, 0, 0, 0);
        browser_.EnableScrollBar(FALSE);
        browser_.Create(m_hWnd, &rcClient, _T("WebBrowser"), WS_CHILD | WS_VISIBLE);
        return 0;
    }

    return QView::CreateControl(hWnd, pcc);
}

void CWebBrowser::OnBtnClkOpenFile( HELEMENT )
{
    //     CFileDialog dlg(TRUE, L"htm files", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
    //         L"HTML Files (*.htm,*.html)\0*.HTM;*.HTML\0"
    //         L"All Files (*.*)\0*.*\0", m_hWnd);
    //     if(dlg.DoModal() == IDOK)
    //     {
    //         CTextFileRead fr(dlg.GetPathName());
    //         wstring all;
    //         if ( fr.IsOpen() && fr.Read(all))
    //         {
    //             m_hostWebPage.DisplayHtml(all.c_str());
    //         }
    //         else
    //         {
    //             XMsgBox::ErrorMsgBox(L"未能加载文件数据！");
    //         }
    //     }
}

void CWebBrowser::OnBtnClkGoBack( HELEMENT )
{
    browser_.GoBack();
}

void CWebBrowser::OnBtnClkGoNext( HELEMENT )
{
    browser_.GoForward();
}

void CWebBrowser::OnBtnClkZoom100( HELEMENT )
{
    assert(false);
}

void CWebBrowser::OnBtnClkGoStop( HELEMENT )
{
    browser_.Stop();
}

void CWebBrowser::OnBtnClkRefresh( HELEMENT )
{
    browser_.Refresh();
}

BOOL CWebBrowser::DisplayHtml( LPCWSTR pszHtmlBuffer )
{
    assert(false);
    return FALSE;
}

void CWebBrowser::SetSize(int cx, int cy)
{
    browser_.MoveWindow(0, 0, cx, cy, FALSE);
}

void CWebBrowser::CallJsFunc(LPCWSTR lpszFunc, LPCWSTR lpszParam)
{
    browser_.CallJsFunc(lpszFunc, lpszParam);
}

void CWebBrowser::SetUIEventHandler(IDocHostUIEventHandler* handler)
{
    browser_.SetUIEventHandler(handler);
}

void CWebBrowser::OnDestroy()
{
    ATLASSERT(browser_.IsWindow());
    browser_.DestroyWindow();

    QView::OnDestroy();
}
