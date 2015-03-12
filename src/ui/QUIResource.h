#pragma once

#include <map>
#include "QBuffer.h"

class QUIResource
{
    struct _key_string
    {	// functor for operator<
        bool operator()(const CStdString& _Left, const CStdString& _Right) const
        {	// apply operator< to operands
            return _Left.CompareNoCase(_Right) < 0;
        }
    };
    typedef std::map<CStdString,QBuffer*,_key_string>::iterator MapStr2DataItr;

    // 空的资源
    static const int NULL_RES_DATA = 0;

public:
    virtual ~QUIResource();

    /**
     *	从zip文件中读取数据
     *		Load之前确保当前没有任何资源数据
     *	params
     *		-[in]
     *			szZip       Zip包路径
    **/
    template<typename T>
    BOOL LoadFromZip(__in T zip)
    {
        // TODO: 加入T类型判断
        ASSERT ( GetResourceCount() == 0);

        BOOL bOK = FALSE;
        CXUnzip theZip;
        if ( theZip.Open(zip) )
        {
            bOK = LoadZip(&theZip);
            theZip.Close();
        }
        return bOK;
    }

//     BOOL LoadFromZip(__in LPCWSTR szZip);
//     BOOL LoadFromZipBuffer(__in QBuffer& buf);

    // 当前已经存在的资源不会因为此次Load而释放
    // 但是已经存在的资源如果和此次加载的资源重名，则原来的资源被释放
    BOOL AddedLoadFromZip(__in LPCWSTR szZip);

    /**
     *	读取数据，返回NULL，则读取失败
     *		
     *	params
     *		-[in]
     *			szResName       资源名称，不区分大小写
    **/
    QBuffer* GetResource(__in LPCWSTR szResName);

    /**
     *	释放资源内存
     *		
     *	params
     *		-[in]
     *			szResName       资源名称，不区分大小写
     *          szResIndicator  资源标识符号，
     *                          如果不为NULL，释放资源的时候同时更新HL中此路径缓存
    **/
    void ReleaseResource(__in LPCWSTR szResName, 
        __in LPCWSTR szResIndicator = NULL);

    /**
     *	添加一个资源内存
     *		必须是堆内存指针，使用new申请，添加后不要自己释放内存
     *	params
     *		-[in]
     *			szResName       资源名称，不区分大小写
     *          pRes            数据资源指针
     *                          
    **/
    BOOL AddResource(__in LPCWSTR szResName, __in QBuffer* pRes);

    /**
     *	更新一个资源内存，原来的资源内存将被释放
     *		
     *	params
     *		-[in]
     *			szResName       资源名称，不区分大小写
     *          pRes            数据资源指针
     *                          ！！！必须是堆内存指针，使用new申请
    **/
    BOOL UpdateResource(__in LPCWSTR szResName, __in QBuffer* pNewRes);

    /**
     *	更新一个资源内存的名字，新的名字必须是不存在的
     *		
     *	params
     *		-[in]
     *			szResName       资源名称，不区分大小写
     *          szNewName       新的资源名字
    **/
    BOOL UpdateResourceName(__in LPCWSTR szResName, __in LPCWSTR szNewName);

    /**
     *	-释放所有的资源内存
     *      此处的内存资源虽然被释放，但是HL对于已经加载过的资源留有cache。
     *      这份cache内存只有当page reload、窗口被销毁的时候才会被释放掉。
     *      如果从内存开销优化方面考虑，必须要手动的销毁这份cache。
     *      使用HTMLayoutDataReadyAsync可以达到目的。
     *		
     *	params
     *		-[in]
     *			szResIndicator      正确清理资源，必须提供正确的资源路径标识符
     *                              对于自定义的资源，使用QUI_CUSTOM_INDICATOR
     *                              NULL则不清理HL的cache
     *			
    **/
    void ReleaseAll(__in LPCWSTR szResIndicator = NULL);

    inline int GetResourceCount()const
    {
        return m_mapData.size();
    }

protected:
    /**
     *	读取资源后，对资源进一步处理，比如数据加密
     *		返回FALSE，资源将被抛弃
     *	params
     *		-[in]
     *			sResName        资源名字
     *		-[inout]
     *          bufData         需要处理的资源
     *			
    **/
    virtual BOOL DecodeResource(__in const CStdString &sResName, __inout QBuffer* pData)
    {
        return TRUE;
    }

    // 加载资源的时候调用
    virtual BOOL OnLoadFromZip(CXUnzip *zip)
    {
        return TRUE;
    }

    BOOL LoadZip(__in CXUnzip* zip);

    inline MapStr2DataItr _Find(__in LPCWSTR szResName)
    {
        return m_mapData.find(szResName);
    }

    inline MapStr2DataItr _End()
    {
        return m_mapData.end();
    }

    inline BOOL IsExist(__in LPCWSTR szResName)
    {
        return _Find(szResName) != m_mapData.end();
    }

    virtual void OnReleaseResource(__in const CStdString& szResName, 
        __in const CStdString& szResIndicator = L"" );

protected:
    // 系统每次加载数据成功后都会缓存到此处
    std::map<CStdString,QBuffer*,_key_string>	m_mapData;
};

