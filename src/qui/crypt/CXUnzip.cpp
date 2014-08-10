#include "stdafx.h"
#include "CXUnzip.h"

CXUnzip::CXUnzip()
{
	m_hZip = NULL;
}

CXUnzip::~CXUnzip()
{
	Close();
}

BOOL CXUnzip::Close()
{
	if (NULL != m_hZip)
	{
		CloseZip(m_hZip);
		m_hZip = NULL;
	}
	return TRUE;
}

BOOL CXUnzip::Open( LPCWSTR psZip )
{
	if (NULL != m_hZip)
	{
		CloseZip(m_hZip);
		m_hZip = NULL;
	}
	m_hZip = OpenZip((void*)psZip,0,ZIP_FILENAME);
	return NULL != m_hZip;
}

BOOL CXUnzip::Open( QBuffer &bufData )
{
	if (NULL != m_hZip)
	{
		CloseZip(m_hZip);
		m_hZip = NULL;
	}
	m_hZip = OpenZip((void*)bufData.GetBuffer(),
		bufData.GetBufferLen(),ZIP_MEMORY);
	return NULL != m_hZip;
}

BOOL CXUnzip::UnzipToBuffer( int idx,__out QBuffer& d )
{
	ZIPENTRYW zInfo;
	if (ZR_OK == GetZipItem(m_hZip,idx,&zInfo))
	{
		d.AllocBuffer(zInfo.unc_size+16);
		int nRet = UnzipItem(m_hZip,idx,d.GetBuffer(0),zInfo.unc_size+16,ZIP_MEMORY);
		if ((ZR_OK == nRet) || (ZR_MORE == nRet))
		{
			d.SetBufferLen(zInfo.unc_size);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CXUnzip::UnzipToBuffer( __in LPCWSTR sFile,__out QBuffer& d )
{
	int idx;
	ZIPENTRYW zInfo;
	if (ZR_OK == FindZipItem(m_hZip,sFile,true,&idx,&zInfo))
	{
		d.AllocBuffer(zInfo.unc_size+16);
		int nRet = UnzipItem(m_hZip,idx,d.GetBuffer(0),zInfo.unc_size+16,ZIP_MEMORY);
		if ((ZR_OK == nRet) || (ZR_MORE == nRet))
		{
			d.SetBufferLen(zInfo.unc_size);
			return TRUE;
		}
	}
//	ASSERT(FALSE);
	return FALSE;
}

BOOL CXUnzip::UnzipToFile( __in LPCWSTR sItem,__in LPCWSTR sFileTo )
{
	QBuffer buf;
	if (UnzipToBuffer(sItem,buf))
	{
		return buf.FileWrite(sFileTo);
	}
	return FALSE;
}

int CXUnzip::FindItem( LPCWSTR sFile )
{
	ZIPENTRYW zInfo;
	int idx;
	if (ZR_OK != FindZipItem(m_hZip,sFile,false,&idx,&zInfo))
		return -1;
	return idx;
}

int CXUnzip::GetItemCount()
{
	ZIPENTRYW ze;
	if (ZR_OK == GetZipItem(m_hZip,-1,&ze))
	{
		return ze.index;
	}
	return 0;
}

BOOL CXUnzip::GetItemInfo( int idx,ZIPENTRYW *pze )
{
	return ZR_OK == GetZipItem(m_hZip,idx,pze);
}

//////////////////////////////////////////////////////////////////////////
CXZip::CXZip()
{
	m_hZip = NULL;
}

CXZip::~CXZip()
{
	Close();
}

void CXZip::Close()
{
	if (m_hZip != NULL)
	{
		CloseZip(m_hZip);
		m_hZip = NULL;
	}
}

BOOL CXZip::Create(LPCWSTR sFile)
{
	Close();
	m_hZip = CreateZip((void*)sFile,0,ZIP_FILENAME);
	return m_hZip != NULL;
}

BOOL CXZip::Open( LPCWSTR sFile )
{
	Close();
	m_hZip = OpenZip((void*)sFile,0,ZIP_FILENAME);
	return NULL != m_hZip;
}

BOOL CXZip::AddFile( LPCWSTR sName,LPBYTE pData,int nDataLen )
{
	char szDest[MAX_PATH*2];
	memset(szDest, 0, sizeof(szDest));
	// need to convert Unicode dest to ANSI
	int nActualChars = WideCharToMultiByte(CP_ACP,	// code page
		0,						// performance and mapping flags
		(LPCWSTR) sName,		// wide-character string
		-1,						// number of chars in string
		szDest,					// buffer for new string
		MAX_PATH*2-2,			// size of buffer
		NULL,					// default for unmappable chars
		NULL);					// set when default char used
	if (nActualChars == 0)
		return FALSE; 
	szDest[nActualChars] = 0;

	return ZR_OK == ZipAdd(m_hZip,(TCHAR*)szDest,pData,nDataLen,ZIP_MEMORY);
}


