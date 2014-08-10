#pragma once

#include "QBuffer.h"

class  ZlibData
{
public:
    ZlibData(LPCBYTE pData=NULL,DWORD nLen=0);
    // 如果两个函数调用成功,需要使用delete 删除pout指针指向的内存
    static BOOL CompressData(__in QBuffer &in,__out QBuffer &out);
    static BOOL CompressData( __in LPCBYTE pBufIn,__in DWORD nByteIn, __out QBuffer& bufOut );
    static BOOL DecompressData(__in QBuffer &in,__in DWORD nOrgSize, __out QBuffer &out);
    static BOOL DecompressData(__in LPCBYTE pin,__in DWORD nByteIn,__in DWORD nOrgSize, __out QBuffer &out);
    static BOOL DecompressData(__inout QBuffer &inout,__inout DWORD& nOrgSize);
public:
    BOOL ReadFile(LPCTSTR pszFile);
    // 数据为经过压缩的数据，前四个字节为其原始大小
    BOOL WriteDataFile(LPCTSTR pszFile);
    BOOL Compress();
    // param : 压缩前的数据大小
    BOOL Decompress(DWORD nOrignDataLen);

    void SetData(LPCBYTE pData,DWORD nLen);
    void SetData(QBuffer &buf);
    void ClearData(){ m_buf.ClearBuffer();};
    QBuffer& GetData(){ return m_buf;};

private:
    QBuffer		m_buf;
    DWORD		m_dwOrignBufferLen;	// 压缩前的数据大小，用于解压
};
