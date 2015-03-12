#include "stdafx.h"
#include "QSkinUI.h"
#include "QUIMgr.h"

QUI_BEGIN_EVENT_MAP(QSkinUI, QFrame)
    BN_STATECHANGED_NAME(L"item_skin", &QSkinUI::OnItemSkinChanged)
    BN_STATECHANGED_ID(L"sld_opacity", &QSkinUI::OnSldOpacityChanged)
QUI_END_EVENT_MAP()

QSkinUI::QSkinUI() 
    : QFrame(L"qabs:common/skin/skin.htm")
{
    request_ = nullptr;
    curitm_ = NULL;
}

void QSkinUI::OnItemSkinChanged(HELEMENT he) 
{
    ECtrl itm(he);
    _Skinbag *pBag = _ItemData(itm);
    if (nullptr == pBag)
    {
        // 请求外部数据
        if (nullptr != request_)
        {
            CStdString src = _ThumbSrc(itm);
            pBag = (*request_)(src);
            if (nullptr != pBag)
            {
                // 记录下这个数据包
                skins_.push_back(pBag);
                itm.SetData(pBag);
            }
            else
            {
                // 还原到原来的皮肤项目
                if (curitm_.is_valid())
                {
                    curitm_.SetCheck(TRUE, TRUE);
                }
            }
        }
    }

    if (nullptr != pBag)
    {

        // 记录当前使用的item
        curitm_ = itm;
        // 首先判断这个数据包是否是完整的
        if (!pBag->IsComplete())
        {
            // 加载数据包
            QUIMgr::PrepareSkinbag(pBag);
        }
        
        if (pBag->IsComplete())
        {
            QUIMgr::ApplyStylebag(pBag);
        }
    }
}

void QSkinUI::OnSldOpacityChanged(HELEMENT he) 
{
    int nAlpha = 255 - ESlider(he).GetValue();

    QUIMgr::ApplyOpacity(nAlpha);
}

BOOL QSkinUI::AddItem(__in _Skinbag* skin) 
{
    if (nullptr == skin)
    {
        ATLASSERT(FALSE);
        return FALSE;
    }

    if (!IsWindow())
    {
        skins_.push_back(skin);
        return TRUE;
    }

    ECtrl box = _ThumbBox();
    ETable itm = box.create("table");
    box.append(itm);
#ifdef _DEBUG
//     QString s;
//     s.Format(L"<tr><td .chk></tr><tr><td><progress /></td></tr><tr><td "
//         L"style=\"overflow:hidden;color:white;\">%s</td></tr>",
//         CPath(skin->GetThumb()).GetFileTitlex());
//     itm.SetHtml(s);
#else
    itm.SetHtml(L"<tr><td .chk></tr><tr><td><progress /></td></tr>");
#endif

    itm.SetBkgndImage(skin->GetThumb());
    itm.SetName(L"item_skin");
    itm.SetData(skin);
    itm.set_attribute("thumb_src", skin->GetThumb());

    if (!skin->IsVisible())
    {
        // 不可见？
        itm.ShowCtrl(SHOW_MODE_COLLAPSE);
    }

    // 记录之
    skins_.push_back(skin);

    return TRUE;
}

BOOL QSkinUI::ShowAt(CPoint pt) 
{
    QSkinUI * pThis = GetInstance();
    if (!pThis->Create())
    {


        ATLASSERT(FALSE);
        return FALSE;

    }

    pThis->SetWindowPos(HWND_TOPMOST, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

    return TRUE;
}

BOOL QSkinUI::ShowAt(__in HELEMENT he)
{
    ECtrl c(he);
    CRect rc = c.get_location();
    CPoint ptBR = rc.BottomRight();
    CPoint pt = rc.CenterPoint();
    ::ClientToScreen(c.get_element_hwnd(true), &pt);
    ::ClientToScreen(c.get_element_hwnd(true), &ptBR);
    pt.x -= 165;
    pt.y = ptBR.y + 1;

    return ShowAt(pt);
}

void QSkinUI::SetSkins(__in std::vector<_Skinbag*> *p) 
{
    ATLASSERT(nullptr != p);

    // 清除掉UIMgr现在使用的数据
    QUIMgr::ApplyStylebag(nullptr);

    if (nullptr != p)
    {
        _ThumbBox().DeleteAllChild();

        ClearAll();

        for (auto i = p->begin(); i != p->end(); ++i) 
        {

            AddItem(*i);
        }
    }
}

void QSkinUI::ClearAll() 
{
    _Skinbag *p = nullptr;
    for (auto i = skins_.begin(); i != skins_.end(); ++i) 
    {
        p = *i;
        if (nullptr != p)
        {
            p->Destroy();
        }
    }
    skins_.clear();


}

LRESULT QSkinUI::OnDocumentComplete() 
{
    std::vector<_Skinbag*> tmp = skins_;
    skins_.clear();

    for (auto i = tmp.begin(); i != tmp.end(); ++i) 
    {

        AddItem(*i);
    }

    //    quibase::TransparentWindow(GetSafeHwnd(), 250);

    return 0;
}

void QSkinUI::OnClose() 
{
    if (QUIIsQuiting())
    {
        // 现在不允许使用换肤功能了
        QUIMgr::ApplyStylebag(nullptr);
        // 销毁所有的数据
        ClearAll();
        // 不是退出就不关闭
        SetMsgHandled(FALSE);
    }
    else
    {
        // 隐藏就行了
        ShowWindow(SW_HIDE);
    }
}

void QSkinUI::OnKillFocus(HWND h) 
{
    SetMsgHandled(FALSE);

    SendMessage(WM_CLOSE);
}

LRESULT QSkinUI::OnMSG_SKINBAGREADY(UINT uMsg, WPARAM w, LPARAM l)
{
    int nPos = int(w);
    CStdString* t = reinterpret_cast<CStdString*>(l);
    CStdString thumb = *t;
    delete t;

    // 更新下载进度显示
    // EProgress prg();

    if (nPos == 100)
    {
        _Skinbag *pBag = nullptr;
        auto iEnd = skins_.end();
        for (auto i = skins_.begin(); i != iEnd; ++i)
        {
            pBag = *i;
            if (pBag->thumb_ == thumb)
            {
                QUIMgr::SkinbagComplete(pBag);
                QUIMgr::ApplyStylebag(pBag);

                break;
            }
        }
    }
    return 0;
}

BOOL QSkinUI::UseSkin(__in LPCWSTR szThumb, __in int nAlpha) 
{
    if (Create(NULL, WS_POPUP))
    {
        for (auto i = skins_.begin(); i != skins_.end(); ++i) 
        {
            if (0 == (*i)->GetThumb().CompareNoCase(szThumb))
            {
                ESlider(GetCtrl("#sld_opacity")).SetValue(255 - nAlpha);
                QUIMgr::ApplyOpacity(nAlpha);

                ETable tbl = _Data2Item(*i);
                if (tbl.is_valid())
                {
                    tbl.SetCheck(TRUE, TRUE);
                    // 是这个皮肤
                    OnItemSkinChanged(tbl);

                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

