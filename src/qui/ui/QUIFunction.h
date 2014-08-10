#pragma once

/**
 *	QUIBase 的一些功能性配置
 *		
**/
class QUIFunction
{
public:
    virtual ~QUIFunction()
    {

    }

    /**
     *  在使用皮肤数据包之前，应该确保数据被正确加载了
     *      以下两个调用中的任何一个错误，应用皮肤进程终止
     *      1，PrepareSkinbag
     *      2，LoadSkinbag
    **/

    // 准备数据 （还没有被加载进资源管理器）
    virtual BOOL PrepareSkinbag(__inout _Skinbag* p)
    {
        return TRUE;
    }

    // 数据已经完成准备，响应此通知的时候，可以将数据加载进资源管理器。
    virtual BOOL SkinbagComplete(__inout _Skinbag* p)
    {
        ASSERT(nullptr != p);

        if (nullptr != p)
        {
            p->SetComplete(TRUE);
        }

        return TRUE;
    }

    // 加载数据（需要被加载仅资源管理器）
    virtual BOOL LoadSkinbag(__inout _Skinbag* p)
    {
        return TRUE;
    }

    /**
     *	应用程序需要在pView上应用透明
     *		
     *  return
     *      true        可以应用透明
     *	params
     *		-[in]
     *			pView   需要应用透明的窗口指针
     *          nAlpha  透明度 [0, 255]
    **/
    virtual BOOL OnApplyOpacity(__in QView* pView, __in int nAlpha);

    /**
     *  应用程序需要在pView上应用皮肤
     *	-return
     *		TRUE        可以应用
     *	params
     *		-[in]
     *			pView       需要应用皮肤的窗口
     *          pBag        皮肤数据
    **/
    virtual BOOL OnApplySkinbag(__in QView* pView, __in _Skinbag* pBag)
    {
        return TRUE;
    }

	/**
     *  应用程序在pView上应用皮肤数据已经完成
     *	params
     *		-[in]
     *			pView       应用了皮肤的窗口
     *          pBag        皮肤数据
    **/
    virtual void ApplySkinbagComplete(__in QView* pView, __in _Skinbag* pBag)
    {
    }
	
    /**
     *  手动更新HL窗口的特定uri资源内容
     *	-return
     *		FALSE       不要更新        
     *	params
     *		-[in]
     *			pView       需要应用更改的窗口
     *          uri         资源索引
    **/
    virtual BOOL OnUpdateResData(__in QView* pView, __in LPCWSTR uri);
    
    /**
     *	-读取自定义的数据
     *		当QUIMgr不能解析这些指定资源数据的时候，会从此处取
     *	params
     *		-[in]
     *			szURI       路径
     *          he          哪个DOM请求的资源
     *		-[out]
     *			pBuf        数据
    **/
    virtual BOOL LoadResData(__in LPCWSTR szURI ,__in HELEMENT he, __out QBuffer* &pBuf)
    {
        pBuf = nullptr;
        return FALSE;
    }
};

