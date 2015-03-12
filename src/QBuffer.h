#pragma once

// Buffer And Container
// #include "QUI.h"
#include <Windows.h>
#include <WinSCard.h>
#include <algorithm>

class  QBuffer  
{
	// Attributes
protected:
	PBYTE	m_pBase;
	PBYTE	m_pPtr;
	UINT	m_nSize;

	// Methods
protected:
	UINT ReAllocateBuffer(UINT nRequestedSize);
	UINT DeAllocateBuffer(UINT nRequestedSize);

public:
	UINT GetMemSize();
	void ClearBuffer();
	void ZeroBuffer();
	BOOL AllocBuffer(int nSize);

	UINT Delete(UINT nSize);
	UINT Read(PBYTE pData, UINT nSize);
	// 读取最后面的nSize个字节
	UINT ReadLast(PBYTE pData,UINT nSize);
	BOOL Write(LPCTSTR strData);
	BOOL Write(const BYTE* pData, UINT nSize);
	UINT GetBufferLen();
	BOOL SetBufferLen(int nLen);
	int Scan(PBYTE pScan,UINT nPos);
	BOOL Insert(PBYTE pData, UINT nSize);
	BOOL Insert(LPCTSTR strData);

	void Copy(QBuffer& buffer);	

	PBYTE GetBuffer(UINT nPos=0);

	QBuffer();
	virtual ~QBuffer();

	BOOL FileWrite( LPCTSTR strFileName );
	BOOL FileRead( LPCTSTR strFileName );

	void WriteByte(BYTE b) { Write(&b,1); }
	UINT ReadByte(BYTE &b) { return Read(&b,1); }
	void WriteInt(int n) {Write((LPBYTE) &n,sizeof(int)); }
	UINT ReadInt(int &n) { return Read((LPBYTE)&n,sizeof(int)); }
	void WriteDouble(double d) { Write((LPBYTE) &d,sizeof(double));}
	UINT ReadDouble( double &d ) { return Read((LPBYTE) &d,sizeof(double)); }
	void WriteFloat(float f) { Write((LPBYTE) &f,sizeof(float)); }
	UINT ReadFloat(float &f) { return Read((LPBYTE)&f,sizeof(float)); }
};
