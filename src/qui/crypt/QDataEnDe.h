#pragma once

// // #include "QUI.h"
#include "QBuffer.h"

class QDataEnDe
{
public:
	BOOL SetCodeData(LPCBYTE p,int len);
	BOOL SetCodeData( QBuffer& d );
	BOOL DecryptData(__inout QBuffer &buf);
	BOOL EncryptData(__inout QBuffer &buf);

private:
	BYTE		m_code[16];	
};

