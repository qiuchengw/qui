#pragma once

// // #include "QUI.h"

//#define USE_WTL
#include <atltransactionmanager.h>
#include "time/QTime.h"
//#include "TransactionMgr.h"


#ifndef __ATLCORE_H__
	/////////////////////////////////////////////////////////////////////////////
	// File status
	inline HRESULT AtlHresultFromLastError() throw()
	{
		DWORD dwErr = ::GetLastError();
		return HRESULT_FROM_WIN32(dwErr);
	}

	inline HRESULT AtlHresultFromWin32(__in DWORD nError) throw()
	{
		return( HRESULT_FROM_WIN32( nError ) );
	}

	/////////////////////////////////////////////////////////////////////////////
	// Verify that a null-terminated string points to valid memory
	inline BOOL AtlIsValidString(
		_In_z_count_(nMaxLength) LPCWSTR psz,
		__in size_t nMaxLength = INT_MAX)
	{
		(nMaxLength);
		return (psz != NULL);
	}

	// Verify that a null-terminated string points to valid memory
	inline BOOL AtlIsValidString(
		_In_z_count_(nMaxLength) LPCSTR psz,
		__in size_t nMaxLength = UINT_MAX)
	{
		(nMaxLength);
		return (psz != NULL);
	}

	// Verify that a pointer points to valid memory
	inline BOOL AtlIsValidAddress(
		_In_opt_bytecount_(nBytes) const void* p,
		__in size_t nBytes,
		__in BOOL bReadWrite = TRUE)
	{
		(bReadWrite);
		(nBytes);
		return (p != NULL);
	}
#endif

struct QFileStatus
{
	QTime m_ctime;          // creation date/time of file
	QTime m_mtime;          // last modification date/time of file
	QTime m_atime;          // last access date/time of file
	ULONGLONG m_size;            // logical size of file in bytes
	BYTE m_attribute;       // logical OR of CFile::Attribute enum values
	BYTE _m_padding;        // pad the structure to a WORD
	TCHAR m_szFullName[_MAX_PATH]; // absolute path name
};

class QException
{
public:
// Constructors
	QException();   // sets m_bAutoDelete = TRUE
	explicit QException(BOOL bAutoDelete);   // sets m_bAutoDelete = bAutoDelete

// Operations
	void Delete();  // use to delete exception in 'catch' block

	virtual BOOL GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR lpszError, __in UINT nMaxError,
		_Out_opt_ PUINT pnHelpContext = NULL) const ;
	virtual BOOL GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR lpszError, __in UINT nMaxError,
		_Out_opt_ PUINT pnHelpContext = NULL);

// Implementation (setting m_bAutoDelete to FALSE is advanced)
public:
	virtual ~QException(){};
	BOOL m_bAutoDelete;
};

class QFileException : public QException
{
public:
	enum {
		none,
		genericException,
		fileNotFound,
		badPath,
		tooManyOpenFiles,
		accessDenied,
		invalidFile,
		removeCurrentDir,
		directoryFull,
		badSeek,
		hardIO,
		sharingViolation,
		lockViolation,
		diskFull,
		endOfFile
	};

	 QFileException(int cause = QFileException::none, LONG lOsError = -1,LPCTSTR lpszArchiveName = NULL)
			:m_cause(cause),m_lOsError(lOsError),m_strFileName(lpszArchiveName){};

// Attributes
	int     m_cause;
	LONG    m_lOsError;
	CStdString m_strFileName;

// Operations
	// convert a OS dependent error code to a Cause
	static int PASCAL OsErrorToException(LONG lOsError);
	static int PASCAL ErrnoToException(int nErrno);

	// helper functions to throw exception after converting to a Cause
	static void PASCAL ThrowOsError(LONG lOsError, LPCTSTR lpszFileName = NULL);
	static void PASCAL ThrowErrno(int nErrno, LPCTSTR lpszFileName = NULL);

// Implementation
public:
	virtual ~QFileException(){};
	virtual BOOL GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR lpszError, 
		__in UINT nMaxError, _Out_opt_ PUINT pnHelpContext) const;
};

/////////////////////////////////////////////////////////////////////////////
// File - raw unbuffered disk file I/Ox

class QFile
{
public:
// Flag values
	enum OpenFlags {
		modeRead =         (int) 0x00000,
		modeWrite =        (int) 0x00001,
		modeReadWrite =    (int) 0x00002,
		shareCompat =      (int) 0x00000,
		shareExclusive =   (int) 0x00010,
		shareDenyWrite =   (int) 0x00020,
		shareDenyRead =    (int) 0x00030,
		shareDenyNone =    (int) 0x00040,
		modeNoInherit =    (int) 0x00080,
		modeCreate =       (int) 0x01000,
		modeNoTruncate =   (int) 0x02000,
		typeText =         (int) 0x04000, // typeText and typeBinary are
		typeBinary =       (int) 0x08000, // used in derived classes only
		osNoBuffer =       (int) 0x10000,
		osWriteThrough =   (int) 0x20000,
		osRandomAccess =   (int) 0x40000,
		osSequentialScan = (int) 0x80000,
		};

	enum Attribute {
		normal =    0x00,
		readOnly =  0x01,
		hidden =    0x02,
		system =    0x04,
		volume =    0x08,
		directory = 0x10,
		archive =   0x20
		};

	enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };

	static  const HANDLE hFileNull;

// Constructors
	QFile();
	QFile(HANDLE hFile);
	QFile(LPCTSTR lpszFileName, UINT nOpenFlags);

// Attributes
	HANDLE m_hFile;
	operator HANDLE() const;

	virtual ULONGLONG GetPosition() const;
	BOOL GetStatus(QFileStatus& rStatus) const;
	virtual CStdString GetFileName() const;
	virtual CStdString GetFilePath() const;
	virtual void SetFilePath(LPCTSTR lpszNewName);

// Operations
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags, QFileException* pError = NULL);

	/// <summary>
	/// This static function renames the specified file.</summary>
	/// <param name="lpszOldName">The old path.</param>
	/// <param name="lpszNewName">The new path.</param>
	/// <param name="pTM">Pointer to CAtlTransactionManager object</param>
	static void PASCAL Rename(LPCTSTR lpszOldName, LPCTSTR lpszNewName, CAtlTransactionManager* pTM=NULL);
	/// <summary>
	/// This static function deletes the file specified by the path.</summary>
	/// <param name="lpszFileName">A string that is the path to the desired file. The path can be relative or absolute.</param>
	/// <param name="pTM">Pointer to CAtlTransactionManager object</param>
	static void PASCAL Remove(LPCTSTR lpszFileName, CAtlTransactionManager* pTM=NULL);
	/// <summary>
	/// This method retrieves status information related to a given CFile object instance or a given file path.</summary>
	/// <returns> 
	/// TRUE if succeeds; otherwise FALSE.</returns>
	/// <param name="lpszFileName">A string that is the path to the desired file. The path can be relative or absolute.</param>
	/// <param name="rStatus">A reference to a user-supplied CFileStatus structure that will receive the status information.</param>
	/// <param name="pTM">Pointer to CAtlTransactionManager object</param>
	static BOOL PASCAL GetStatus(LPCTSTR lpszFileName, QFileStatus& rStatus, CAtlTransactionManager* pTM = NULL);

	/// <summary>
	/// Sets the status of the file associated with this file location.</summary>
	/// <param name="lpszFileName">A string that is the path to the desired file. The path can be relative or absolute.</param>
	/// <param name="rStatus">The buffer containing the new status information.</param>
	/// <param name="pTM">Pointer to CAtlTransactionManager object</param>
	static void PASCAL SetStatus(LPCTSTR lpszFileName, const QFileStatus& status, CAtlTransactionManager* pTM = NULL);
	ULONGLONG SeekToEnd();
	void SeekToBegin();

// Overridables
	virtual QFile* Duplicate() const;

	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom);
	virtual void SetLength(ULONGLONG dwNewLen);
	virtual ULONGLONG GetLength() const;

	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);

	virtual void LockRange(ULONGLONG dwPos, ULONGLONG dwCount);
	virtual void UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount);

	virtual void Abort();
	virtual void Flush();
	virtual void Close();

// Implementation
public:
	virtual ~QFile();
	enum BufferCommand { bufferRead, bufferWrite, bufferCommit, bufferCheck };
	enum BufferFlags 
	{ 
		bufferDirect = 0x01,
		bufferBlocking = 0x02
	};
	virtual UINT GetBufferPtr(UINT nCommand, UINT nCount = 0,
		void** ppBufStart = NULL, void** ppBufMax = NULL);

protected:
	void CommonBaseInit(HANDLE hFile);
	void CommonInit(LPCTSTR lpszFileName, UINT nOpenFlags);
	BOOL m_bCloseOnDelete;
	CStdString m_strFileName;
};

/////////////////////////////////////////////////////////////////////////////
// STDIO file implementation

class QStdioFile : public QFile
{
public:
// Constructors
	QStdioFile();

	/// <summary>
	/// CStdioFile constructor</summary>
	/// <param name="pTM">Pointer to CAtlTransactionManager object</param>
	QStdioFile(LPCTSTR lpszFileName, UINT nOpenFlags);
	
	/// <summary>
	/// CStdioFile constructor</summary>
	/// <param name="lpszFileName">A string that is the path to the desired file. The path can be relative or absolute.</param>
	/// <param name="nOpenFlags">Sharing and access mode. Specifies the action to take when opening the file. You can combine options listed below by using the bitwise-OR (|) operator. One access permission and one share option are required; the modeCreate and modeNoInherit modes are optional.</param>
	/// <param name="pTM">Pointer to CAtlTransactionManager object</param>
	QStdioFile(LPCTSTR lpszFileName, UINT nOpenFlags, CAtlTransactionManager* pTM);
	QStdioFile(FILE* p);
// Attributes
	FILE* m_pStream;    // stdio FILE
						// m_hFile from base class is _fileno(m_pStream)

// Operations
	// reading and writing strings
	virtual void WriteString(LPCTSTR lpsz);
	virtual LPTSTR ReadString(_Out_z_cap_(nMax) LPTSTR lpsz, __in UINT nMax);
	virtual BOOL ReadString(CStdString& rString);

// Implementation
public:
	virtual ~QStdioFile();
	virtual ULONGLONG GetPosition() const;
	virtual ULONGLONG GetLength() const;
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags, QFileException* pError = NULL);

	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);
	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom);
	virtual void Abort();
	virtual void Flush();
	virtual void Close();

	// Unsupported APIs
	virtual QFile* Duplicate() const;
	virtual void LockRange(ULONGLONG dwPos, ULONGLONG dwCount);
	virtual void UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount);

protected:
	void CommonInit(LPCTSTR lpszFileName, UINT nOpenFlags, CAtlTransactionManager* pTM);
};

//////////////////////////////////////////////////////////////////////////
// FileFind
class QFileFind
{
public:
	QFileFind();

	/// <summary>
	/// CFileFind constructor</summary>
	/// <param name="pTM">Pointer to CAtlTransactionManager object</param>
	QFileFind(CAtlTransactionManager* pTM);
	virtual ~QFileFind();

	// Attributes
public:
	ULONGLONG GetLength() const;
	virtual CStdString GetFileName() const;
	virtual CStdString GetFilePath() const;
	virtual CStdString GetFileURL() const;
	virtual CStdString GetRoot() const;

	virtual BOOL GetLastWriteTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastAccessTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetCreationTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastWriteTime(QTime& refTime) const;
	virtual BOOL GetLastAccessTime(QTime& refTime) const;
	virtual BOOL GetCreationTime(QTime& refTime) const;

	virtual BOOL MatchesMask(DWORD dwMask) const;

	virtual BOOL IsDots() const;
	// these aren't virtual because they all use MatchesMask(), which is
	BOOL IsReadOnly() const;
	BOOL IsDirectory() const;
	BOOL IsCompressed() const;
	BOOL IsSystem() const;
	BOOL IsHidden() const;
	BOOL IsTemporary() const;
	BOOL IsNormal() const;
	BOOL IsArchived() const;

	// Operations
	void Close();
	virtual BOOL FindFile(LPCTSTR pstrName = NULL, DWORD dwUnused = 0);
	virtual BOOL FindNextFile();

protected:
	virtual void CloseContext();

	// Implementation
protected:
	void* m_pFoundInfo;
	void* m_pNextInfo;
	HANDLE m_hContext;
	CStdString m_strRoot;
	TCHAR m_chDirSeparator;     // not '\\' for Internet classes

	/// <summary>
	/// Pointer to CAtlTransactionManager object</summary>
	CAtlTransactionManager* m_pTM;
};

////////////////////////////////////////////////////////////////////////////
// Memory based file implementation

// class QMemFile : public QFile
// {
// public:
// // Constructors
// 	/* explicit */ QMemFile(UINT nGrowBytes = 1024);
// 	QMemFile(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0);
// 
// // Operations
// 	void Attach(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0);
// 	BYTE* Detach();
// 
// // Advanced Overridables
// protected:
// 	virtual BYTE* Alloc(SIZE_T nBytes);
// 	virtual BYTE* Realloc(BYTE* lpMem, SIZE_T nBytes);
// 	virtual BYTE* Memcpy(BYTE* lpMemTarget, const BYTE* lpMemSource, SIZE_T nBytes);
// 	virtual void Free(BYTE* lpMem);
// 	virtual void GrowFile(SIZE_T dwNewLen);
// 
// // Implementation
// protected:
// 	SIZE_T m_nGrowBytes;
// 	SIZE_T m_nPosition;
// 	SIZE_T m_nBufferSize;
// 	SIZE_T m_nFileSize;
// 	BYTE* m_lpBuffer;
// 	BOOL m_bAutoDelete;
// 
// public:
// 	virtual ~QMemFile();
// 	virtual ULONGLONG GetPosition() const;
// 	BOOL GetStatus(QFileStatus& rStatus) const;
// 	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom);
// 	virtual void SetLength(ULONGLONG dwNewLen);
// 	virtual UINT Read(void* lpBuf, UINT nCount);
// 	virtual void Write(const void* lpBuf, UINT nCount);
// 	virtual void Abort();
// 	virtual void Flush();
// 	virtual void Close();
// 	virtual UINT GetBufferPtr(UINT nCommand, UINT nCount = 0,
// 		void** ppBufStart = NULL, void** ppBufMax = NULL);
// 	virtual ULONGLONG GetLength() const;
// 
// 	// Unsupported APIs
// 	virtual QFile* Duplicate() const;
// 	virtual void LockRange(ULONGLONG dwPos, ULONGLONG dwCount);
// 	virtual void UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount);
// };


// StdioFileEx.h: interface for the CStdioFileEx class.
//
// Version 1.1 23 August 2003. Incorporated fixes from Dennis Jeryd.
// Version 1.3 19 February 2005. Incorporated fixes Howard J Oh and some of my own.
//
// Copyright David Pritchard 2003-2004. davidpritchard@ctv.es
//
// You can use this class freely, but please keep my ego happy 
// by leaving this comment in place.
//
#define	nUNICODE_BOM			0xFEFF		// Unicode "byte order mark" which goes at start of file
#define	sNEWLINE			_T("\r\n")	// New line characters
#define	sDEFAULT_UNICODE_FILLER_CHAR	"#"			// Filler char used when no conversion from Unicode to local code page is possible

class CStdioFileEx: public QStdioFile
{
public:
	CStdioFileEx();
	CStdioFileEx( LPCTSTR lpszFileName, UINT nOpenFlags );

	virtual BOOL	Open( LPCTSTR lpszFileName, UINT nOpenFlags, QFileException* pError = NULL );
	virtual BOOL	ReadString(CStdString& rString);
	virtual void	WriteString( LPCTSTR lpsz );
	bool				IsFileUnicodeText()	{ return m_bIsUnicodeText; }	
	unsigned long	GetCharCount();

	// Additional flag to allow Unicode text writing
	static const UINT modeWriteUnicode;

	// static utility functions

	// --------------------------------------------------------------------------------------------
	//
	//	CStdioFileEx::GetUnicodeStringFromMultiByteString()
	//
	// --------------------------------------------------------------------------------------------
	// Returns:    int - num. of chars written (0 means error)
	// Parameters: char *		szMultiByteString		(IN)		Multi-byte input string
	//					wchar_t*		szUnicodeString		(OUT)		Unicode outputstring
	//					int&			nUnicodeBufferSize	(IN/OUT)	Size of Unicode output buffer(IN)
	//																			Actual bytes written to buffer (OUT)
	//					UINT			nCodePage				(IN)		Code page used to perform conversion
	//																			Default = -1 (Get local code page).
	//
	// Purpose:		Gets a Unicode string from a MultiByte string.
	// Notes:		None.
	// Exceptions:	None.
	//
	static int		GetUnicodeStringFromMultiByteString(IN char * szMultiByteString, OUT wchar_t* szUnicodeString, IN OUT int& nUnicodeBufferSize, IN UINT nCodePage=-1);

	// --------------------------------------------------------------------------------------------
	//
	//	CStdioFileEx::GetMultiByteStringFromUnicodeString()
	//
	// --------------------------------------------------------------------------------------------
	// Returns:    int - number of characters written. 0 means error
	// Parameters: wchar_t *	szUnicodeString			(IN)	Unicode input string
	//					char*			szMultiByteString			(OUT)	Multibyte output string
	//					short			nMultiByteBufferSize		(IN)	Multibyte buffer size
	//					UINT			nCodePage					(IN)	Code page used to perform conversion
	//																			Default = -1 (Get local code page).
	//
	// Purpose:		Gets a MultiByte string from a Unicode string.
	// Notes:		.
	// Exceptions:	None.
	//
	static int			GetMultiByteStringFromUnicodeString(wchar_t * szUnicodeString,char* szMultiByteString,
		short nMultiByteBufferSize,UINT nCodePage=-1);


	// --------------------------------------------------------------------------------------------
	//
	//	CStdioFileEx::IsFileUnicode()
	//
	// --------------------------------------------------------------------------------------------
	// Returns:    bool
	// Parameters: const CString& sFilePath
	//
	// Purpose:		Determines whether a file is Unicode by reading the first character and detecting
	//					whether it's the Unicode byte marker.
	// Notes:		None.
	// Exceptions:	None.
	//
	static bool IsFileUnicode(const CStdString& sFilePath);


protected:
	UINT ProcessFlags(const CStdString& sFilePath, UINT& nOpenFlags);

	bool		m_bIsUnicodeText;
	UINT		m_nFlags;
};


/** 
 * This class adapts the CStdioFile MFC class to read and write to UTF8  
 * encoded files. If the application is compiled with unicode support, all 
 * UTF8 character sequences are handled, else only the normal ASCII7 subset. 
 */ 
class CStdioFile_UTF8 : public QStdioFile 
{ 
public: 
	// Duplicate the standard constructors 
	CStdioFile_UTF8() : QStdioFile() {} 
	CStdioFile_UTF8(FILE* pOpenStream) : QStdioFile(pOpenStream) {} 
	CStdioFile_UTF8(LPCTSTR lpszFileName, UINT nOpenFlags) : QStdioFile(lpszFileName, nOpenFlags) {} 
 
    // Override these methods for UTF8 conversion 
	virtual LPTSTR ReadString(LPTSTR lpsz, UINT nMax); 
	virtual BOOL ReadString(CStdString& rString); 
	virtual void WriteString(LPCTSTR lpsz); 
 
    // Read and write an optional Byte Order Mark 
	BOOL ReadBOM(); 
	void WriteBOM(); 
};
