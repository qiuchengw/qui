#ifndef QUIDlgs_h__
#define QUIDlgs_h__

#pragma once

#include "QDialog.h"

#ifndef QUI_USE_COLORBTN
#   define QUI_USE_COLORBTN
#endif

class XMsgBox : public QDialog
{
    BEGIN_MSG_MAP_EX(XMsgBox)
        MSG_WM_TIMER(OnTimer)
        CHAIN_MSG_MAP(QDialog)
    END_MSG_MAP()

	enum ENUM_MSGBOX_TYPE
	{
		MSGBOX_OK,
		MSGBOX_ERROR,
		MSGBOX_YESNO,
	};

    enum ENUM_MSGBOX_COUNTDOWN_BTN
    {
        ID_MSGBOX_BTN_OK,
        ID_MSGBOX_BTN_YES,
        ID_MSGBOX_BTN_NO,
    };

    enum
    {
        ID_TIMER_COUNTDOWN = 1000,
    };
public:
	XMsgBox(void):QDialog(L"qabs:common/MsgBox.htm")
    {
        m_dwCountdown = 0;
    };
	
public:
	static INT_PTR OkMsgBox( LPCTSTR pszMsg,LPCTSTR pszCaption=NULL,
        DWORD dwCountdown = 0)
	{
		return MakeMsgBox(MSGBOX_OK,pszMsg,pszCaption, dwCountdown, ID_MSGBOX_BTN_OK);
	}

	static INT_PTR YesNoMsgBox( LPCTSTR pszMsg,LPCTSTR pszCaption=NULL,
        DWORD dwCountdown = 0, ENUM_MSGBOX_COUNTDOWN_BTN eBtn = ID_MSGBOX_BTN_YES)
	{
		return MakeMsgBox(MSGBOX_YESNO,pszMsg,pszCaption, dwCountdown, eBtn);
	}

	static INT_PTR ErrorMsgBox( LPCTSTR pszMsg,LPCTSTR pszCaption=NULL ,DWORD dwCountdown = 0)
	{
		return MakeMsgBox(MSGBOX_ERROR,pszMsg,pszCaption, dwCountdown, ID_MSGBOX_BTN_OK);
	}

protected:
	static INT_PTR MakeMsgBox( ENUM_MSGBOX_TYPE eType ,LPCTSTR pszMsg,
        LPCTSTR pszCaption, DWORD dwCountdown, ENUM_MSGBOX_COUNTDOWN_BTN eBtn)
	{
		XMsgBox mb;
		mb.m_sCaption = pszCaption;//?pszCaption:QGetTitleString(L"7");
		mb.m_sMsg = pszMsg;
		mb.m_eBoxType = eType;
        mb.m_dwCountdown = dwCountdown;
        mb.m_eBtn = eBtn;

		return mb.DoModal();
	}

    void OnTimer(UINT nTimerID)
    {
        if (ID_TIMER_COUNTDOWN != nTimerID)
        {
            SetMsgHandled(FALSE);
            return;
        }

        if (m_dwCountdown-- > 0)
        {
            wchar_t szCap[32] = {0};
            swprintf_s(szCap, 32, m_sBtnCaption, m_dwCountdown);
            m_btnCountdown.SetText(szCap);
            return;
        }

        // 自动关闭对话框
        KillTimer(ID_TIMER_COUNTDOWN);
        m_btnCountdown.send_event(BN_CLICKED);
    }

	virtual LRESULT OnDocumentComplete()
	{
		CStdString sSmallIcon,sBigIcon,sTitle,sButtons;
		switch (m_eBoxType)
		{
		case MSGBOX_OK:
			{
				sSmallIcon = L"qrel:icons/info.png";
				sBigIcon = L"qrel:icons/info_48.png";
				sTitle = m_sCaption.IsEmpty()?L"提示":m_sCaption;
#ifdef QUI_USE_COLORBTN
                sButtons = L"<div .color_button .blue xrole=\"OK\" role=\"default-button\">确定</div>";
#else
                sButtons = L"<button xrole=\"OK\" role=\"default-button\">确定</button>";
#endif
				break;
			}
		case MSGBOX_ERROR:
			{
				sSmallIcon = L"qrel:icons/error.png";
				sBigIcon = L"qrel:icons/error_48.png";
				sTitle = m_sCaption.IsEmpty()?L"错误":m_sCaption;
#ifdef QUI_USE_COLORBTN
                sButtons = L"<div .color_button .blue xrole=\"OK\" role=\"default-button\">确定</div>";
#else
                sButtons = L"<button xrole=\"OK\" role=\"default-button\">确定</button>";
#endif
				break;
			}
		case MSGBOX_YESNO:
			{
				sSmallIcon = L"qrel:icons/ask.png";
				sBigIcon = L"qrel:icons/ask_48.png";
				sTitle = m_sCaption.IsEmpty()?L"确认":m_sCaption;
#ifdef QUI_USE_COLORBTN
                sButtons = 
                    L"<div .color_button .blue xrole=\"YES\" role=\"default-button\" accesskey=\"!Y\">是(Y)</div>"
                    L"<div .color_button .orange xrole=\"NO\" accesskey=\"!N\" style=\"margin-left:20px;\">否(N)</div>";
#else
                sButtons = 
                    L"<button xrole=\"YES\" role=\"default-button\" accesskey=\"!Y\">是(Y)</button>"
                    L"<button xrole=\"NO\" accesskey=\"!N\" style=\"margin-left:20px;\">否(N)</button>";
#endif
				break;
			}
		}
		GetCtrl("#wc-sysicon").SetBkgndImage(sSmallIcon);
		GetCtrl("#DIV-ICON").SetBkgndImage(sBigIcon);
		GetCtrl("#wc-title").SetText(sTitle);
		GetCtrl("#DIV-MSG").SetHtml(m_sMsg);
		GetCtrl("#DIV-BUTTONS").SetHtml(sButtons);

        if (m_dwCountdown > 0)
        {
            switch (m_eBtn)
            {
            case ID_MSGBOX_BTN_OK:
                {
                    m_sBtnCaption = L"确定( = %u)";
                    m_btnCountdown = GetCtrl("[xrole=\"OK\"]");
                    break;
                }
            case ID_MSGBOX_BTN_YES:
                {
                    m_sBtnCaption = L"是(Y = %u)";
                    m_btnCountdown = GetCtrl("[xrole=\"YES\"]");
                    break;
                }
            case ID_MSGBOX_BTN_NO:
                {
                    m_sBtnCaption = L"否(N = %u)";
                    m_btnCountdown = GetCtrl("[xrole=\"NO\"]");
                    break;
                }
            }
            SetTimer(ID_TIMER_COUNTDOWN, 1000);
        }
		return 0;
	}

public:
	CStdString		        m_sCaption;
	ENUM_MSGBOX_TYPE	m_eBoxType;
	CStdString		        m_sMsg;
    ECtrl               m_btnCountdown;
    ENUM_MSGBOX_COUNTDOWN_BTN   m_eBtn;
    DWORD               m_dwCountdown;
    CStdString             m_sBtnCaption;
};


class XInputBox : public QDialog
{
    friend class XInputBox;

public:
    static INT_PTR InputBox( const CStdString& sTip ,
        __out CStdString & sInput,int nMinInput=0)
    {
        XInputBox InputDlg(sTip,nMinInput);
        INT_PTR nRet = InputDlg.DoModal();
        if (IDOK == nRet)
        {
            sInput = InputDlg.GetInput();
        }
        return nRet;
    }

private:
    XInputBox(LPCWSTR szTip,int nMinInput=0)
        :QDialog(L"qabs:common/InputBox.htm"),
        m_sTip(szTip),
        m_nMinInput(nMinInput)
    {

    }
    ~XInputBox(void){};

protected:
    virtual BOOL OnDefaultButton(INT_PTR nBtn)
    {
        if (IDOK == nBtn)
        {
            EEdit eInput = GetCtrl("#inputbox");

            m_sInput = eInput.GetText();

            if (m_sInput.GetLength() < m_nMinInput)
            {
                CStdString sTip;
                sTip.Format(L"请至少输入 [%d] 个字符",m_nMinInput);
                eInput.ShowTooltip(sTip);
                return FALSE;
            }
        }
        return TRUE;
    }
    virtual LRESULT OnDocumentComplete()
    {
        ECtrl eMsgTip = GetCtrl("#input_tip"); 
        if (m_sTip.IsEmpty())
        {
            m_sInput = L"请输入信息";
        }
        eMsgTip.SetText(m_sTip);
        return QDialog::OnDocumentComplete();
    }
    CStdString	GetInput() { return m_sInput; }

private:
    CStdString		m_sTip;
    CStdString		m_sInput;
    int			m_nMinInput;
};

#endif // QUIDlgs_h__
