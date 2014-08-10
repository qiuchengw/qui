#include "stdafx.h"
#include "QDataEnDe.h"
#include "crypt/crijndael.h"
#include "crypt/MD5.h"

BOOL QDataEnDe::SetCodeData( QBuffer& d )
{
	return SetCodeData(d.GetBuffer(0), d.GetBufferLen());	
}

BOOL QDataEnDe::SetCodeData( LPCBYTE p,int len )
{
	// 获得密码
	MD5 md5;
    md5.update(const_cast<PBYTE>(p), len);
    md5.finalize();
    PBYTE pData = md5.raw_digest(); // 16 bytes
    if (NULL != pData)
    {
        CopyMemory(m_code, pData, 16);

        return TRUE;
    }
	return FALSE;
}

BOOL QDataEnDe::EncryptData( __inout QBuffer &buf )
{
	// 加密
	CRijndael rjd;
	if (RIJNDAEL_SUCCESS != rjd.init(CRijndael::ECB,
		CRijndael::Encrypt,m_code,CRijndael::Key16Bytes))
	{
		return FALSE;
	}
	QBuffer inBuf;
	inBuf.AllocBuffer(buf.GetBufferLen() + 16);
	int len = rjd.padEncrypt(buf.GetBuffer(),buf.GetBufferLen(),inBuf.GetBuffer(0));
	if (len > 0)
	{
		inBuf.SetBufferLen(len);
		buf.Copy(inBuf);
		return TRUE;
	}
	return FALSE;
}

BOOL QDataEnDe::DecryptData( __inout QBuffer &buf )
{
	// 解密
	CRijndael rjd;
	if (RIJNDAEL_SUCCESS != rjd.init(CRijndael::ECB,
		CRijndael::Decrypt,m_code,CRijndael::Key16Bytes))
	{
		return FALSE;
	}
	QBuffer inBuf;
	inBuf.AllocBuffer(buf.GetBufferLen() + 16);
	int len = rjd.padDecrypt(buf.GetBuffer(),buf.GetBufferLen(),inBuf.GetBuffer(0));
	if (len > 0)
	{
		inBuf.SetBufferLen(len);
		buf.Copy(inBuf);
		return TRUE;
	}
	return FALSE;
}
