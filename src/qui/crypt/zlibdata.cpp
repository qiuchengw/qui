#include "stdafx.h"
#include "zlibdata.h"
#include "zlib/zlib.h"
#include "file/QFile.h"

#pragma comment(lib,"zlib")

ZlibData::ZlibData( LPCBYTE pData/*=NULL*/,DWORD nLen/*=0*/ )
{
    SetData(pData,nLen);
}

void ZlibData::SetData( LPCBYTE pData,DWORD nLen )
{
    if ((pData != NULL) && (nLen > 0) )
    {
        m_buf.Write(pData,nLen);
    }
}

void ZlibData::SetData( QBuffer &buf )
{
    m_buf.Copy(buf);
}

BOOL ZlibData::CompressData(__in QBuffer &in,__out QBuffer &out)
{
    return CompressData(in.GetBuffer(),in.GetBufferLen(),out);
}

BOOL ZlibData::CompressData( __in LPCBYTE pBufIn,__in DWORD nByteIn, __out QBuffer& bufOut )
{
    DWORD nBytesOut = compressBound(nByteIn);
    bufOut.AllocBuffer(nBytesOut);
    nBytesOut = bufOut.GetMemSize();
    if (Z_OK == compress(bufOut.GetBuffer(0),(uLongf*)&nBytesOut,pBufIn,nByteIn))
    {
        bufOut.SetBufferLen(nBytesOut);
        return TRUE;
    }
    bufOut.ClearBuffer();
    return FALSE;
}

BOOL ZlibData::DecompressData(__in QBuffer &in,__in DWORD nOrgSize, __out QBuffer &bufOut)
{
    return DecompressData(in.GetBuffer(0),in.GetBufferLen(),nOrgSize,bufOut);
}

BOOL ZlibData::DecompressData(__inout QBuffer &inout,__inout DWORD& nOrgSize)
{
    QBuffer out;
    out.AllocBuffer(nOrgSize);
    if (Z_OK == uncompress(out.GetBuffer(0),(uLongf*)&nOrgSize,
        inout.GetBuffer(0),inout.GetBufferLen()))
    {
        out.SetBufferLen(nOrgSize);
        inout.Copy(out);
        return TRUE;
    }
    return FALSE;
}

BOOL ZlibData::DecompressData( __in LPCBYTE pin,__in DWORD nByteIn,__in DWORD nOrgSize, __out QBuffer &bufOut )
{
    DWORD nDecpressedSize = nOrgSize;
    bufOut.AllocBuffer(nOrgSize);
    if ((Z_OK == uncompress(bufOut.GetBuffer(0),(uLongf*)&nDecpressedSize,pin,nByteIn)) 
        && (nDecpressedSize == nOrgSize))
    {
        bufOut.SetBufferLen(nOrgSize);
        return TRUE;
    }
    bufOut.ClearBuffer();
    return FALSE;
}

BOOL ZlibData::ReadFile( LPCTSTR pszFile )
{
    ClearData();

    QFile file;
    if (!file.Open(pszFile,QFile::modeRead))
        return FALSE;
    BYTE buf[10240];
    DWORD nRead;
    while((nRead = file.Read(buf,10240)) > 0)
    {
        m_buf.Write(buf,nRead);
    }
    file.Close();
    return TRUE;
}

BOOL ZlibData::Compress()
{
    if (m_buf.GetBufferLen() > 0)
    {
        DWORD nsi = m_buf.GetBufferLen();
        QBuffer bufOut;
        if (CompressData(m_buf,bufOut))
        {
            m_buf.Copy(bufOut);
            return TRUE;
        }
    }
    return FALSE;
}

BOOL ZlibData::Decompress( DWORD nOrignDataLen )
{
    // 前四个字节为源数据大小
    if (nOrignDataLen > 0)
    {
        PBYTE pOut = NULL;
        if (DecompressData(m_buf,nOrignDataLen))
        {
            return TRUE;
        }
    }
    return FALSE;
}
