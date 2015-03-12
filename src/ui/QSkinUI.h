#ifndef QSkinUI_h__
#define QSkinUI_h__

#pragma once

#include "QFrame.h"

class QSkinUI;

/**
 *	用于HL窗口的换肤
 *			
**/
struct _SkinItem
{
    // method 在选择器选中的DOM中是必须定义的，否则会assert
    _SkinItem(LPCWSTR s, LPCSTR a, LPCWSTR v, LPCSTR method = "activate")
        :selector_(s), atti_(a), val_(v), xcall_(method)
    {
        selector_.Trim();
        atti_.Trim();
        val_.Trim();
        xcall_.Trim();
    }

    inline BOOL IsValid()const
    {
        return !selector_.IsEmpty() && !atti_.IsEmpty();
    }

    bool operator==(const _SkinItem& r)
    {
        return r.selector_.CompareNoCase(selector_) == 0;
    }

    bool operator==(LPCWSTR selctor)
    {
        return selector_.CompareNoCase(CStdString(selctor).Trim()) == 0;
    }

    // <div #box skin="skin1" />
    // 换肤方法：
    //      select(div#box).set_attribute(atti_, val_)
    CStdString     selector_;  // 如 div#box_1
    CStdStringA     atti_;  // 属性 如： name/id/自定义的字段
    CStdString     val_;   // 属性值
    CStdStringA     xcall_; // 设置此属性后，需要调用的behavior_method,为空不调用
                        // 比如<style>重新设置css路径后，必须调用“activate”才会生效
};
typedef std::vector<_SkinItem*>  SkinItems;

/// 用于换肤的数据
///     1> 换肤，就是换css
///     2> 在一个bag中，同一个selector是不应该出现多次的！
class _Skinbag
{
    friend class QSkinUI;

protected:
    // 必须使用工厂方法创建之
    ~_Skinbag()
    {
        auto iEnd = _End();
        for (auto i = itms_.begin(); i != iEnd; ++i)
        {
            delete *i;
        }
    }

    _Skinbag(LPCWSTR thumb)
    {
        ATLASSERT(nullptr != thumb);
        ATLASSERT(wcslen(thumb) > 0); 

        thumb_ = thumb;
        complete_ = FALSE;
        visible_ = TRUE;
    }

public:
    /**
     *	工厂方法
     *		
     *	params
     *		-[in]
     *			szThumb     预览图像地址
    **/
    static _Skinbag* Create(__in LPCWSTR szThumb)
    {
        return new _Skinbag(szThumb);
    }

    void Destroy()
    {
        delete this;
    }

    inline CStdString GetThumb()const
    {
        return thumb_;
    }

    /**
     *	在应用这个皮肤包之前会判断皮肤数据是否是完整的
     *		如果不完整，需要外部提供这个包的数据，准备完成后需要通知QSkinUI
     *      此时再进行应用皮肤
     *	params
     *		-[in]
     *			
     *		-[out]
     *			
     *		-[inout]
     *			
    **/
    inline BOOL IsComplete()const
    {
        return complete_;
    }

    inline void SetComplete(__in BOOL bComplete)
    {
        complete_ = bComplete;
    }

    inline BOOL IsVisible()const
    {
        return visible_;
    }

    inline void SetVisible(__in BOOL bVisible)
    {
        visible_ = bVisible;
    }
	
    /**
     *	在skin数据包中加上一个需要执行的项目
     *		
     *	params
     *		-[in]
     *			selector    需要被应用的选择器
     *          atti/val    选择器 【属性-值】
     *          method      DOM设置属性值后需要调用的behavior——method
     *                      对于<style>设置src后必须调用 “activate”使其生效
     *                      method 在选择器选中的DOM中是必须定义的，否则会assert
    **/
    BOOL AddItem(__in LPCWSTR selector, __in LPCSTR atti, 
        __in LPCWSTR val, __in LPCSTR method = "activate")
    {
        _SkinItem* p = new _SkinItem(selector, atti, val, method);
        if (!p->IsValid())
        {
            delete p;

            return FALSE;
        }

        itms_.push_back(p);

        return TRUE;
    }

    // <style #theme>
    inline BOOL AddStyleThemeItem(__in LPCWSTR val)
    {
        return AddItem(L"style#theme", "src", val);
    }

    BOOL RemoveItem(__in LPCWSTR selector)
    {
        auto i = _Find(selector);
        if (_End() != i)
        {
            delete *i;
            itms_.erase(i);

            return TRUE;
        }
        return FALSE;
    }

    inline SkinItems* GetAll()
    {
        return &itms_;
    }

    inline int GetItemCount()const
    {
        return itms_.size();
    }

private:
    inline SkinItems::iterator _Find(__in LPCWSTR selector)
    {
        auto iEnd = _End();
        for (auto i = itms_.begin(); i != iEnd; ++i)
        {
            if (*(*i) == selector)
            {
                return i;
            }
        }
        return iEnd;
    }

    inline SkinItems::iterator _End()
    {
        return itms_.end();
    }

private:
    SkinItems   itms_;
    CStdString     thumb_; // 预览图片路径
    BOOL        complete_;  // 数据包是否是完整的
    BOOL        visible_;   // 是否是用户可见的
};

/**
 *  当用户点击一个皮肤预览项目的时候，QSkinUI会先在本地查看当前项目关联的皮肤数据
 *  如果本地没有关联的皮肤数据，将回调此函数以请求皮肤数据。
 *  
 *	-return
 *	    NULL    皮肤更换进程终止
 *      返回的皮肤包数据指针必须是堆内存使用new生成，而且不要自己删除。
 *	params
 *		-[in]
 *			pszSkinThumb        预览项目设置的预览图标路径
 *			
**/
typedef _Skinbag* (__stdcall *PFN_REQUEST_SKINBAG)(__in LPCWSTR pszSkinThumb);

// 皮肤库下载通知
DECLARE_USER_MESSAGE(UIBASE_MSG_SKINBAGDOWN);

/**
 *	HL 换肤窗口
 *      只要生成窗口，就直到进程结束才会销毁窗口
 *      要么被呼到前台，要么是隐藏状态
**/
class QSkinUI : public QFrame
{
    QUI_DECLARE_EVENT_MAP;

    SINGLETON_ON_DESTRUCTOR(QSkinUI)
    {
        ClearAll();
    }

    BEGIN_MSG_MAP_EX(QSkinUI)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_KILLFOCUS(OnKillFocus)
        MESSAGE_HANDLER_EX(UIBASE_MSG_SKINBAGDOWN, OnMSG_SKINBAGREADY)
        CHAIN_MSG_MAP(QFrame)
    END_MSG_MAP()

public:
    QSkinUI() ;

    /**
     *	必须确保 skin 指针在换肤期间都是有效的
     *		通过此方法加入进来的数据会被自动销毁。
     *      所以调用者不要自己调用_Skinbag::Destroy
     *	params
     *		-[in]

     *			skin    皮肤包
    **/ 
    BOOL AddItem(__in _Skinbag* skin);

    // 显示之
    static BOOL ShowAt(CPoint pt);
    static BOOL ShowAt(__in HELEMENT he);

    // 设置一系列皮肤数据
    void SetSkins(__in std::vector<_Skinbag*> *p );
    
    inline void SetSkinRequest(__in PFN_REQUEST_SKINBAG pRequest)
    {
        request_ = pRequest;
    }

    // 使用这个皮肤
    BOOL UseSkin(__in LPCWSTR szThumb, __in int nAlpha);
protected:
    BOOL Create(HWND hWndParent = NULL, UINT nStyle = WS_POPUP | WS_VISIBLE)
    {
        if (!IsWindow())
        {
            return QFrame::Create(hWndParent, nStyle,
                WS_EX_LAYERED|WS_EX_TOOLWINDOW, 0);
        }
        return TRUE;
    }

    virtual LRESULT OnDocumentComplete();

    void OnItemSkinChanged(HELEMENT he);

    void OnSldOpacityChanged(HELEMENT he);

    ECtrl _ThumbBox()
    {
        return GetCtrl("#box_themes");
    }

    void ClearAll();
    
    BOOL ApplySkinbagEx( __in QView* pView, __in _Skinbag* pBag );

    void OnClose();

    void OnKillFocus(HWND h);

    inline CStdString _ThumbSrc(__in ECtrl& itm)
    {
        return itm.get_attribute("thumb_src");
    }

    inline _Skinbag* _ItemData(__in ECtrl& itm)
    {
        return reinterpret_cast<_Skinbag*>(itm.GetData()); 
    }

    inline ETable _Data2Item(__in _Skinbag* p)
    {
        return _ThumbBox().FindFirstWithData(p);
    }

    LRESULT OnMSG_SKINBAGREADY(UINT uMsg, WPARAM w, LPARAM l);

private:
    ECtrl       curitm_;    // 当前使用的皮肤项目
    std::vector<_Skinbag*>  skins_;
    PFN_REQUEST_SKINBAG     request_;
};

#endif // QSkinUI_h__
