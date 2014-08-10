#pragma once

#include "../QBuffer.h"
#include "XUnzip.h"
#include "XZip.h"

#define ZIP_BUFFERSIZE 20480

class CXUnzip
{
public:
	CXUnzip();
	~CXUnzip();
	BOOL Open(LPCWSTR psZip);
	BOOL Open(QBuffer &bufData);
	BOOL IsOpen()const { return m_hZip != NULL; }
	BOOL Close();

	// unzip current item
	BOOL UnzipToBuffer(__out QBuffer& d);
	// unzip sFile in zipfile to d
	BOOL UnzipToBuffer(__in LPCWSTR sItem,__out QBuffer& d);
	BOOL UnzipToBuffer( int idx,__out QBuffer& d );
	BOOL UnzipToFile(__in LPCWSTR sItem,__in LPCWSTR sFileTo);
	BOOL GetItemInfo(int idx,ZIPENTRYW *pze );
	int FindItem(LPCWSTR sFile);
	int GetItemCount();

private:
	HZIP		m_hZip;
};

class CXZip
{
public:
	CXZip();
	~CXZip();
	
	BOOL Create(LPCWSTR sFile);
	BOOL Open(LPCWSTR sFile);
	BOOL AddFile(LPCWSTR sName,LPBYTE pData,int nDataLen);
	void Close();

private:
	HZIP		m_hZip;
};