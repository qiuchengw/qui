#include "stdafx.h"
#include "QFile.h"
#include <io.h>
#include <shlobj.h>
#include <shellapi.h>
#include <Strsafe.h>
#include <errno.h>
#include <fcntl.h>
#include <mbstring.h> 
#include <atldef.h>
#include "XTrace.h"

/////////////////////////////////////////////////////////////////////////////
// CFileException helpers

#ifndef UNUSED
#	define UNUSED(x) x
#endif

void __declspec(noreturn) AfxThrowFileException(int cause, LONG lOsError,
	LPCTSTR lpszFileName /* == NULL */)
{
	throw(new QFileException(cause, lOsError, lpszFileName));
}


UINT AfxGetFileName(LPCTSTR lpszPathName, _Out_opt_cap_(nMax) LPTSTR lpszTitle, UINT nMax)
{
	// always capture the complete file name including extension (if present)
	LPTSTR lpszTemp = ::PathFindFileName(lpszPathName);

	// lpszTitle can be NULL which just returns the number of bytes
	if (lpszTitle == NULL)
		return lstrlen(lpszTemp)+1;

	// otherwise copy it into the buffer provided
	_tcsncpy_s(lpszTitle, nMax, lpszTemp, _TRUNCATE);
	return 0;
}

QException::QException()
{
	// most exceptions are deleted when not needed
	m_bAutoDelete = TRUE;
}

QException::QException(BOOL bAutoDelete)
{
	// for exceptions which are not auto-delete (usually)
	m_bAutoDelete = bAutoDelete;
}

void QException::Delete()
{
	// delete exception if it is auto-deleting
	if (m_bAutoDelete > 0)
	{
		delete this;
	}
}

BOOL QException::GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR lpszError, __in UINT nMaxError,
	_Out_opt_ PUINT pnHelpContext /* = NULL */ ) const
{
	if (pnHelpContext != NULL)
		*pnHelpContext = 0;

	if (nMaxError != 0 && lpszError != NULL)
		*lpszError = '\0';

	return FALSE;
}

BOOL QException::GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR lpszError, __in UINT nMaxError,
	_Out_opt_ PUINT pnHelpContext /* = NULL */ )
{
	// Call the const version of GetErrorMessage
	return const_cast<const QException*>(this)->GetErrorMessage(lpszError, nMaxError, pnHelpContext);
}

BOOL QFileException::GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR lpszError, __in UINT nMaxError,
	_Out_opt_ PUINT pnHelpContext) const
{
	if (lpszError == NULL || nMaxError == 0) 
	{
		return FALSE;
	}

	if (pnHelpContext != NULL)
		*pnHelpContext = m_cause;

	CStdString str;
	str.Format(L"%d:%s",m_cause,m_strFileName);
	_tcsncpy_s(lpszError, nMaxError, str, _TRUNCATE);

	return TRUE;
}


#ifdef _DEBUG
static const LPCSTR rgszCFileExceptionCause[] =
{
	"none",
	"generic",
	"fileNotFound",
	"badPath",
	"tooManyOpenFiles",
	"accessDenied",
	"invalidFile",
	"removeCurrentDir",
	"directoryFull",
	"badSeek",
	"hardIO",
	"sharingViolation",
	"lockViolation",
	"diskFull",
	"endOfFile",
};
static const char szUnknown[] = "unknown";
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileException

void PASCAL QFileException::ThrowOsError(LONG lOsError,
	LPCTSTR lpszFileName /* = NULL */)
{
	if (lOsError != 0)
		AfxThrowFileException(QFileException::OsErrorToException(lOsError),
		lOsError, lpszFileName);
}

void PASCAL QFileException::ThrowErrno(int nErrno,
	LPCTSTR lpszFileName /* = NULL */)
{
	if (nErrno != 0)
		AfxThrowFileException(QFileException::ErrnoToException(nErrno),
		_doserrno, lpszFileName);
}


/////////////////////////////////////////////////////////////////////////////
// CFileException diagnostics
int PASCAL QFileException::ErrnoToException(int nErrno)
{
	switch(nErrno)
	{
	case EPERM:
	case EACCES:
		return QFileException::accessDenied;
	case EBADF:
		return QFileException::invalidFile;
	case EDEADLOCK:
		return QFileException::sharingViolation;
	case EMFILE:
		return QFileException::tooManyOpenFiles;
	case ENOENT:
	case ENFILE:
		return QFileException::fileNotFound;
	case ENOSPC:
		return QFileException::diskFull;
	case EINVAL:
	case EIO:
		return QFileException::hardIO;
	default:
		return QFileException::genericException;
	}
}

int PASCAL QFileException::OsErrorToException(LONG lOsErr)
{
	// NT Error codes
	switch ((UINT)lOsErr)
	{
	case NO_ERROR:
		return QFileException::none;
	case ERROR_FILE_NOT_FOUND:
		return QFileException::fileNotFound;
	case ERROR_PATH_NOT_FOUND:
		return QFileException::badPath;
	case ERROR_TOO_MANY_OPEN_FILES:
		return QFileException::tooManyOpenFiles;
	case ERROR_ACCESS_DENIED:
		return QFileException::accessDenied;
	case ERROR_INVALID_HANDLE:
		return QFileException::fileNotFound;
	case ERROR_BAD_FORMAT:
		return QFileException::invalidFile;
	case ERROR_INVALID_ACCESS:
		return QFileException::accessDenied;
	case ERROR_INVALID_DRIVE:
		return QFileException::badPath;
	case ERROR_CURRENT_DIRECTORY:
		return QFileException::removeCurrentDir;
	case ERROR_NOT_SAME_DEVICE:
		return QFileException::badPath;
	case ERROR_NO_MORE_FILES:
		return QFileException::fileNotFound;
	case ERROR_WRITE_PROTECT:
		return QFileException::accessDenied;
	case ERROR_BAD_UNIT:
		return QFileException::hardIO;
	case ERROR_NOT_READY:
		return QFileException::hardIO;
	case ERROR_BAD_COMMAND:
		return QFileException::hardIO;
	case ERROR_CRC:
		return QFileException::hardIO;
	case ERROR_BAD_LENGTH:
		return QFileException::badSeek;
	case ERROR_SEEK:
		return QFileException::badSeek;
	case ERROR_NOT_DOS_DISK:
		return QFileException::invalidFile;
	case ERROR_SECTOR_NOT_FOUND:
		return QFileException::badSeek;
	case ERROR_WRITE_FAULT:
		return QFileException::accessDenied;
	case ERROR_READ_FAULT:
		return QFileException::badSeek;
	case ERROR_SHARING_VIOLATION:
		return QFileException::sharingViolation;
	case ERROR_LOCK_VIOLATION:
		return QFileException::lockViolation;
	case ERROR_WRONG_DISK:
		return QFileException::badPath;
	case ERROR_SHARING_BUFFER_EXCEEDED:
		return QFileException::tooManyOpenFiles;
	case ERROR_HANDLE_EOF:
		return QFileException::endOfFile;
	case ERROR_HANDLE_DISK_FULL:
		return QFileException::diskFull;
	case ERROR_DUP_NAME:
		return QFileException::badPath;
	case ERROR_BAD_NETPATH:
		return QFileException::badPath;
	case ERROR_NETWORK_BUSY:
		return QFileException::accessDenied;
	case ERROR_DEV_NOT_EXIST:
		return QFileException::badPath;
	case ERROR_ADAP_HDW_ERR:
		return QFileException::hardIO;
	case ERROR_BAD_NET_RESP:
		return QFileException::accessDenied;
	case ERROR_UNEXP_NET_ERR:
		return QFileException::hardIO;
	case ERROR_BAD_REM_ADAP:
		return QFileException::invalidFile;
	case ERROR_NO_SPOOL_SPACE:
		return QFileException::directoryFull;
	case ERROR_NETNAME_DELETED:
		return QFileException::accessDenied;
	case ERROR_NETWORK_ACCESS_DENIED:
		return QFileException::accessDenied;
	case ERROR_BAD_DEV_TYPE:
		return QFileException::invalidFile;
	case ERROR_BAD_NET_NAME:
		return QFileException::badPath;
	case ERROR_TOO_MANY_NAMES:
		return QFileException::tooManyOpenFiles;
	case ERROR_SHARING_PAUSED:
		return QFileException::badPath;
	case ERROR_REQ_NOT_ACCEP:
		return QFileException::accessDenied;
	case ERROR_FILE_EXISTS:
		return QFileException::accessDenied;
	case ERROR_CANNOT_MAKE:
		return QFileException::accessDenied;
	case ERROR_ALREADY_ASSIGNED:
		return QFileException::badPath;
	case ERROR_INVALID_PASSWORD:
		return QFileException::accessDenied;
	case ERROR_NET_WRITE_FAULT:
		return QFileException::hardIO;
	case ERROR_DISK_CHANGE:
		return QFileException::fileNotFound;
	case ERROR_DRIVE_LOCKED:
		return QFileException::lockViolation;
	case ERROR_BUFFER_OVERFLOW:
		return QFileException::badPath;
	case ERROR_DISK_FULL:
		return QFileException::diskFull;
	case ERROR_NO_MORE_SEARCH_HANDLES:
		return QFileException::tooManyOpenFiles;
	case ERROR_INVALID_TARGET_HANDLE:
		return QFileException::invalidFile;
	case ERROR_INVALID_CATEGORY:
		return QFileException::hardIO;
	case ERROR_INVALID_NAME:
		return QFileException::badPath;
	case ERROR_INVALID_LEVEL:
		return QFileException::badPath;
	case ERROR_NO_VOLUME_LABEL:
		return QFileException::badPath;
	case ERROR_NEGATIVE_SEEK:
		return QFileException::badSeek;
	case ERROR_SEEK_ON_DEVICE:
		return QFileException::badSeek;
	case ERROR_DIR_NOT_ROOT:
		return QFileException::badPath;
	case ERROR_DIR_NOT_EMPTY:
		return QFileException::removeCurrentDir;
	case ERROR_LABEL_TOO_LONG:
		return QFileException::badPath;
	case ERROR_BAD_PATHNAME:
		return QFileException::badPath;
	case ERROR_LOCK_FAILED:
		return QFileException::lockViolation;
	case ERROR_BUSY:
		return QFileException::accessDenied;
	case ERROR_INVALID_ORDINAL:
		return QFileException::invalidFile;
	case ERROR_ALREADY_EXISTS:
		return QFileException::accessDenied;
	case ERROR_INVALID_EXE_SIGNATURE:
		return QFileException::invalidFile;
	case ERROR_BAD_EXE_FORMAT:
		return QFileException::invalidFile;
	case ERROR_FILENAME_EXCED_RANGE:
		return QFileException::badPath;
	case ERROR_META_EXPANSION_TOO_LONG:
		return QFileException::badPath;
	case ERROR_DIRECTORY:
		return QFileException::badPath;
	case ERROR_OPERATION_ABORTED:
		return QFileException::hardIO;
	case ERROR_IO_INCOMPLETE:
		return QFileException::hardIO;
	case ERROR_IO_PENDING:
		return QFileException::hardIO;
	case ERROR_SWAPERROR:
		return QFileException::accessDenied;
	default:
		return QFileException::genericException;
	}
}



void  AfxTimeToFileTime(const QTime& time, LPFILETIME pFileTime)
{
	ATLASSERT(pFileTime != NULL);

	if (pFileTime == NULL) 
	{
		throw L"";
	}

	SYSTEMTIME sysTime;
	sysTime.wYear = (WORD)time.GetYear();
	sysTime.wMonth = (WORD)time.GetMonth();
	sysTime.wDay = (WORD)time.GetDay();
	sysTime.wHour = (WORD)time.GetHour();
	sysTime.wMinute = (WORD)time.GetMinute();
	sysTime.wSecond = (WORD)time.GetSecond();
	sysTime.wMilliseconds = 0;

	// convert system time to local file time
	FILETIME localTime;
	if (!SystemTimeToFileTime((LPSYSTEMTIME)&sysTime, &localTime))
		QFileException::ThrowOsError((LONG)::GetLastError());

	// convert local file time to UTC file time
	if (!LocalFileTimeToFileTime(&localTime, pFileTime))
		QFileException::ThrowOsError((LONG)::GetLastError());
}

///////////////////////////////////////////////////////////////////////////////
// CMemFile::GetStatus implementation

// BOOL QMemFile::GetStatus(QFileStatus& rStatus) const
// {
// 	ASSERT(FALSE);
// 	return FALSE;
// }

/////////////////////////////////////////////////////////////////////////////
// CFile implementation helpers

#ifdef AfxGetFileName
#undef AfxGetFileName
#endif


void  AfxGetRoot(LPCTSTR lpszPath, CStdString& strRoot)
{
	ATLASSERT(lpszPath != NULL);

	LPTSTR lpszRoot = strRoot.GetBuffer(_MAX_PATH);
	memset(lpszRoot, 0, _MAX_PATH);
	_tcsncpy_s(lpszRoot, _MAX_PATH, lpszPath, _TRUNCATE);
	PathStripToRoot(lpszRoot);
	strRoot.ReleaseBuffer();
}
// turn a file, relative path or other into an absolute path
BOOL  _AfxFullPath2(_Out_z_cap_c_(_MAX_PATH) LPTSTR lpszPathOut, LPCTSTR lpszFileIn, QFileException* pException)
	// lpszPathOut = buffer of _MAX_PATH
	// lpszFileIn = file, relative path or absolute path
	// (both in ANSI character set)
	// pException - pointer to exception object - can be NULL.
{
	pException;
	ATLASSERT(AtlIsValidAddress(lpszPathOut, _MAX_PATH));

	// first, fully qualify the path name
	LPTSTR lpszFilePart;
	DWORD dwRet = GetFullPathName(lpszFileIn, _MAX_PATH, lpszPathOut, &lpszFilePart);
	if (dwRet == 0)
	{
		_tcsncpy_s(lpszPathOut, _MAX_PATH, lpszFileIn, _TRUNCATE); // take it literally
		return FALSE;
	}
	else if (dwRet >= _MAX_PATH)
	{
		// GetFullPathName() returned a path greater than _MAX_PATH
		if (pException != NULL)
		{
			pException->m_cause = QFileException::badPath;
			pException->m_strFileName = lpszFileIn;
		}
		return FALSE; // long path won't fit in buffer
	}

	CStdString strRoot;
	// determine the root name of the volume
	AfxGetRoot(lpszPathOut, strRoot);

	if (!::PathIsUNC( strRoot ))
	{
		// get file system information for the volume
		DWORD dwFlags, dwDummy;
		if (!GetVolumeInformation(strRoot, NULL, 0, NULL, &dwDummy, &dwFlags,
			NULL, 0))
		{
			return FALSE;   // preserving case may not be correct
		}

		// not all characters have complete uppercase/lowercase
		if (!(dwFlags & FS_CASE_IS_PRESERVED))
			CharUpper(lpszPathOut);

		// assume non-UNICODE file systems, use OEM character set
		if (!(dwFlags & FS_UNICODE_STORED_ON_DISK))
		{
			WIN32_FIND_DATA data;
			HANDLE h = FindFirstFile(lpszFileIn, &data);
			if (h != INVALID_HANDLE_VALUE)
			{
				FindClose(h);
				if(lpszFilePart != NULL && lpszFilePart > lpszPathOut)
				{
					int nFileNameLen = lstrlen(data.cFileName);
					int nIndexOfPart = (int)(lpszFilePart - lpszPathOut);
					if ((nFileNameLen + nIndexOfPart) < _MAX_PATH)
					{
						_tcscpy_s(lpszFilePart, _MAX_PATH - nIndexOfPart, data.cFileName);
					}
					else
					{
						// the path+filename of the file is too long
						if (pException != NULL)
						{
							pException->m_cause = QFileException::badPath;
							pException->m_strFileName = lpszFileIn;
						}
						return FALSE; // Path doesn't fit in the buffer.
					}
				}
				else
				{
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}
//Backward compatible wrapper that calls the new exception throwing func.
BOOL  AfxFullPath(_Pre_notnull_ _Post_z_ LPTSTR lpszPathOut, LPCTSTR lpszFileIn)
{
	return _AfxFullPath2(lpszPathOut, lpszFileIn,NULL);
}


BOOL  AfxComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2)
{
#pragma warning(push)
#pragma warning(disable:4068)
#pragma prefast(push)
#pragma prefast(disable:400, "lstrcmpi is ok here as we are backing it up with further comparison")
	// use case insensitive compare as a starter
	if (lstrcmpi(lpszPath1, lpszPath2) != 0)
	{
		return FALSE;
	}
#pragma prefast(pop)
#pragma warning(pop)

	// on non-DBCS systems, we are done
	if (!GetSystemMetrics(SM_DBCSENABLED))
		return TRUE;

	// on DBCS systems, the file name may not actually be the same
	// in particular, the file system is case sensitive with respect to
	// "full width" roman characters.
	// (ie. fullwidth-R is different from fullwidth-r).
	int nLen = lstrlen(lpszPath1);
	if (nLen != lstrlen(lpszPath2))
		return FALSE;
	ATLASSERT(nLen < _MAX_PATH);

	// need to get both CT_CTYPE1 and CT_CTYPE3 for each filename
	LCID lcid = GetThreadLocale();
	WORD aCharType11[_MAX_PATH];
	ATLVERIFY(GetStringTypeEx(lcid, CT_CTYPE1, lpszPath1, -1, aCharType11));
	WORD aCharType13[_MAX_PATH];
	ATLVERIFY(GetStringTypeEx(lcid, CT_CTYPE3, lpszPath1, -1, aCharType13));
	WORD aCharType21[_MAX_PATH];
	ATLVERIFY(GetStringTypeEx(lcid, CT_CTYPE1, lpszPath2, -1, aCharType21));
#ifdef _DEBUG
	WORD aCharType23[_MAX_PATH];
	ATLVERIFY(GetStringTypeEx(lcid, CT_CTYPE3, lpszPath2, -1, aCharType23));
#endif

	// for every C3_FULLWIDTH character, make sure it has same C1 value
	int i = 0;
	for (LPCTSTR lpsz = lpszPath1; *lpsz != 0; lpsz = _tcsinc(lpsz))
	{
		// check for C3_FULLWIDTH characters only
		if (aCharType13[i] & C3_FULLWIDTH)
		{
#ifdef _DEBUG
			ATLASSERT(aCharType23[i] & C3_FULLWIDTH); // should always match!
#endif

			// if CT_CTYPE1 is different then file system considers these
			// file names different.
			if (aCharType11[i] != aCharType21[i])
				return FALSE;
		}
		++i; // look at next character type
	}
	return TRUE; // otherwise file name is truly the same
}

void  AfxGetModuleFileName(HINSTANCE hInst, CStdString& strFileName)
{
	TCHAR szLongPathName[_MAX_PATH];
	::GetModuleFileName(hInst, szLongPathName, _MAX_PATH);
	strFileName = szLongPathName;
}

void  AfxGetModuleShortFileName(HINSTANCE hInst, CStdString& strShortName)
{
	TCHAR szLongPathName[_MAX_PATH];
	::GetModuleFileName(hInst, szLongPathName, _MAX_PATH);
	if (::GetShortPathName(szLongPathName,
		strShortName.GetBuffer(_MAX_PATH), _MAX_PATH) == 0)
	{
		// rare failure case (especially on not-so-modern file systems)
		strShortName = szLongPathName;
	}
	strShortName.ReleaseBuffer();
}

////////////////////////////////////////////////////////////////////////////
// CFile implementation

const HANDLE QFile::hFileNull = INVALID_HANDLE_VALUE;

QFile::QFile()
{
	CommonBaseInit(INVALID_HANDLE_VALUE);
}

QFile::QFile(HANDLE hFile)
{
	ATLASSERT(hFile != INVALID_HANDLE_VALUE);
#ifdef _DEBUG
	DWORD dwFlags = 0;
	ATLASSERT(GetHandleInformation(hFile, &dwFlags) != 0);
#endif
	CommonBaseInit(hFile);
}

QFile::QFile(LPCTSTR lpszFileName, UINT nOpenFlags)
{
	CommonInit(lpszFileName, nOpenFlags);
}

void QFile::CommonBaseInit(HANDLE hFile)
{
	m_hFile = hFile;
	m_bCloseOnDelete = FALSE;
}

void QFile::CommonInit(LPCTSTR lpszFileName, UINT nOpenFlags)
{
	if (lpszFileName == NULL)
	{
		return ;
	}

	CommonBaseInit(INVALID_HANDLE_VALUE);

	QFileException e;
	if (!Open(lpszFileName, nOpenFlags, &e))
	{
	}
}

QFile::~QFile()
{
	if (m_hFile != INVALID_HANDLE_VALUE && m_bCloseOnDelete)
	{
		Close();
	}
}

// CFile
QFile::operator HANDLE() const
{ return m_hFile; }

ULONGLONG QFile::SeekToEnd()
{ return Seek(0, QFile::end); }

void QFile::SeekToBegin()
{ Seek(0, QFile::begin); }

void QFile::SetFilePath(LPCTSTR lpszNewName)
{
	if(lpszNewName != NULL)
		m_strFileName = lpszNewName;
	else
		throw L"";
}

QFile* QFile::Duplicate() const
{
	ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);

	QFile* pFile = new QFile();
	HANDLE hFile;
	if (!::DuplicateHandle(::GetCurrentProcess(), m_hFile,
		::GetCurrentProcess(), &hFile, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		delete pFile;
		QFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
	}
	pFile->m_hFile = hFile;
	ATLASSERT(pFile->m_hFile != INVALID_HANDLE_VALUE);
	pFile->m_bCloseOnDelete = m_bCloseOnDelete;

	return pFile;
}

void PASCAL QFile::SetStatus(LPCTSTR lpszFileName, const QFileStatus& status, CAtlTransactionManager* pTM)
{
	FILETIME creationTime;
	FILETIME lastAccessTime;
	FILETIME lastWriteTime;
	LPFILETIME lpCreationTime = NULL;
	LPFILETIME lpLastAccessTime = NULL;
	LPFILETIME lpLastWriteTime = NULL;

	DWORD wAttr = (pTM != NULL) ? pTM->GetFileAttributes((LPTSTR)lpszFileName) : GetFileAttributes((LPTSTR)lpszFileName);
	if (wAttr == (DWORD)-1L)
		QFileException::ThrowOsError((LONG)GetLastError(), lpszFileName);

	if ((DWORD)status.m_attribute != wAttr && (wAttr & readOnly))
	{
		// Set file attribute, only if currently readonly.
		// This way we will be able to modify the time assuming the
		// caller changed the file from readonly.

		BOOL bRes = (pTM != NULL) ? pTM->SetFileAttributes((LPTSTR)lpszFileName, (DWORD)status.m_attribute) : SetFileAttributes((LPTSTR)lpszFileName, (DWORD)status.m_attribute);
		if (!bRes)
			QFileException::ThrowOsError((LONG)GetLastError(), lpszFileName);
	}

	// last modification time
	if (status.m_mtime.GetTime() != 0)
	{
		AfxTimeToFileTime(status.m_mtime, &lastWriteTime);
		lpLastWriteTime = &lastWriteTime;

		// last access time
		if (status.m_atime.GetTime() != 0)
		{
			AfxTimeToFileTime(status.m_atime, &lastAccessTime);
			lpLastAccessTime = &lastAccessTime;
		}

		// create time
		if (status.m_ctime.GetTime() != 0)
		{
			AfxTimeToFileTime(status.m_ctime, &creationTime);
			lpCreationTime = &creationTime;
		}

		HANDLE hFile = pTM != NULL ?
			pTM->CreateFile (lpszFileName, GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) :
		::CreateFile(lpszFileName, GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			QFileException::ThrowOsError((LONG)::GetLastError(), lpszFileName);

		if (!SetFileTime((HANDLE)hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime))
		{
			LONG sc=(LONG)::GetLastError();
			::CloseHandle(hFile);
			QFileException::ThrowOsError(sc, lpszFileName);
		}

		if (!::CloseHandle(hFile))
			QFileException::ThrowOsError((LONG)::GetLastError(), lpszFileName);
	}

	if ((DWORD)status.m_attribute != wAttr && !(wAttr & readOnly))
	{
		BOOL bRes = (pTM != NULL) ?
			pTM->SetFileAttributes((LPTSTR)lpszFileName, (DWORD)status.m_attribute) :
		SetFileAttributes((LPTSTR)lpszFileName, (DWORD)status.m_attribute);

		if (!bRes)
			QFileException::ThrowOsError((LONG)GetLastError(), lpszFileName);
	}
}

CStdString QFile::GetFileName() const
{


	QFileStatus status;
	GetStatus(status);
	CStdString strResult;
	AfxGetFileName(status.m_szFullName, strResult.GetBuffer(_MAX_FNAME),
		_MAX_FNAME);
	strResult.ReleaseBuffer();
	return strResult;
}

CStdString QFile::GetFilePath() const
{


	QFileStatus status;
	GetStatus(status);
	return status.m_szFullName;
}

/////////////////////////////////////////////////////////////////////////////
// CFile Status implementation

BOOL QFile::GetStatus(QFileStatus& rStatus) const
{
	memset(&rStatus, 0, sizeof(QFileStatus));

	// copy file name from cached m_strFileName
	_tcsncpy_s(rStatus.m_szFullName, _countof(rStatus.m_szFullName), m_strFileName, _TRUNCATE);

	if (m_hFile != hFileNull)
	{
		// get time current file size
		FILETIME ftCreate, ftAccess, ftModify;
		if (!::GetFileTime(m_hFile, &ftCreate, &ftAccess, &ftModify))
			return FALSE;


		LARGE_INTEGER li;

		if (::GetFileSizeEx(m_hFile, &li) == 0)
			return FALSE;

		rStatus.m_size = li.QuadPart;


		if (m_strFileName.IsEmpty())
			rStatus.m_attribute = 0;
		else
		{
			DWORD dwAttribute = ::GetFileAttributes(m_strFileName);

			// don't return an error for this because previous versions of MFC didn't
			if (dwAttribute == 0xFFFFFFFF)
				rStatus.m_attribute = 0;
			else
			{
				rStatus.m_attribute = (BYTE) dwAttribute;
			}
		}

		// convert times as appropriate
		// some file systems may not record file creation time, file access time etc
		if (QTime::IsValidFILETIME(ftCreate))
		{
			rStatus.m_ctime = QTime(ftCreate);
		}
		else
		{
			rStatus.m_ctime = QTime();
		}

		if (QTime::IsValidFILETIME(ftAccess))
		{
			rStatus.m_atime = QTime(ftAccess);
		}
		else
		{
			rStatus.m_atime = QTime();
		}

		if (QTime::IsValidFILETIME(ftModify))
		{
			rStatus.m_mtime = QTime(ftModify);
		}
		else
		{
			rStatus.m_mtime = QTime();
		}

		if (rStatus.m_ctime.GetTime() == 0)
			rStatus.m_ctime = rStatus.m_mtime;

		if (rStatus.m_atime.GetTime() == 0)
			rStatus.m_atime = rStatus.m_mtime;
	}
	return TRUE;
}

BOOL PASCAL QFile::GetStatus(LPCTSTR lpszFileName, QFileStatus& rStatus, CAtlTransactionManager* pTM)
{
	ATLASSERT( lpszFileName != NULL );

	if ( lpszFileName == NULL ) 
	{
		return FALSE;
	}

	if ( lstrlen(lpszFileName) >= _MAX_PATH )
	{
		ATLASSERT(FALSE); // MFC requires paths with length < _MAX_PATH
		return FALSE;
	}

	// attempt to fully qualify path first
	if (!AfxFullPath(rStatus.m_szFullName, lpszFileName))
	{
		rStatus.m_szFullName[0] = '\0';
		return FALSE;
	}

	WIN32_FILE_ATTRIBUTE_DATA fileAttributeData;

	if (pTM != NULL)
	{
		if (!pTM->GetFileAttributesEx(lpszFileName, GetFileExInfoStandard, &fileAttributeData))
			return FALSE;
	}
	else
	{
		if (!GetFileAttributesEx(lpszFileName, GetFileExInfoStandard, &fileAttributeData))
			return FALSE;
	}

	// strip attribute of NORMAL bit, our API doesn't have a "normal" bit.
	rStatus.m_attribute = (BYTE)
		(fileAttributeData.dwFileAttributes & ~FILE_ATTRIBUTE_NORMAL);

	rStatus.m_size = fileAttributeData.nFileSizeHigh;
	rStatus.m_size <<= 32;
	rStatus.m_size |= fileAttributeData.nFileSizeLow;

	// convert times as appropriate
	if (QTime::IsValidFILETIME(fileAttributeData.ftCreationTime))
	{
		rStatus.m_ctime = QTime(fileAttributeData.ftCreationTime);
	}
	else
	{
		rStatus.m_ctime = QTime();
	}

	if (QTime::IsValidFILETIME(fileAttributeData.ftLastAccessTime))
	{
		rStatus.m_atime = QTime(fileAttributeData.ftLastAccessTime);
	}
	else
	{
		rStatus.m_atime = QTime();
	}

	if (QTime::IsValidFILETIME(fileAttributeData.ftLastWriteTime))
	{
		rStatus.m_mtime = QTime(fileAttributeData.ftLastWriteTime);
	}
	else
	{
		rStatus.m_mtime = QTime();
	}

	if (rStatus.m_ctime.GetTime() == 0)
		rStatus.m_ctime = rStatus.m_mtime;

	if (rStatus.m_atime.GetTime() == 0)
		rStatus.m_atime = rStatus.m_mtime;

	return TRUE;
}

BOOL QFile::Open(LPCTSTR lpszFileName, UINT nOpenFlags, QFileException* pException)
{
	ATLASSERT(pException == NULL ||
		AtlIsValidAddress(pException, sizeof(QFileException)));
	ATLASSERT((nOpenFlags & typeText) == 0);   // text mode not supported

	// shouldn't open an already open file (it will leak)
	ATLASSERT(m_hFile == INVALID_HANDLE_VALUE);

	// CFile objects are always binary and CreateFile does not need flag
	nOpenFlags &= ~(UINT)typeBinary;

	m_bCloseOnDelete = FALSE;

	m_hFile = INVALID_HANDLE_VALUE;
	m_strFileName.Empty();

	TCHAR szTemp[_MAX_PATH];
	if (lpszFileName != NULL && SUCCEEDED(StringCchLength(lpszFileName, _MAX_PATH, NULL)) )
	{
		if( _AfxFullPath2(szTemp, lpszFileName,pException) == FALSE )
			return FALSE;
	}
	else
	{
		// user passed in a buffer greater then _MAX_PATH
		if (pException != NULL)
		{
			pException->m_cause = QFileException::badPath;
			pException->m_strFileName = lpszFileName;
		}
		return FALSE; // path is too long
	}
		
	m_strFileName = szTemp;
	ATLASSERT(shareCompat == 0);

	// map read/write mode
	ATLASSERT((modeRead|modeWrite|modeReadWrite) == 3);
	DWORD dwAccess = 0;
	switch (nOpenFlags & 3)
	{
	case modeRead:
		dwAccess = GENERIC_READ;
		break;
	case modeWrite:
		dwAccess = GENERIC_WRITE;
		break;
	case modeReadWrite:
		dwAccess = GENERIC_READ | GENERIC_WRITE;
		break;
	default:
		ATLASSERT(FALSE);  // invalid share mode
	}

	// map share mode
	DWORD dwShareMode = 0;
	switch (nOpenFlags & 0x70)    // map compatibility mode to exclusive
	{
	default:
		ATLASSERT(FALSE);  // invalid share mode?
	case shareCompat:
	case shareExclusive:
		dwShareMode = 0;
		break;
	case shareDenyWrite:
		dwShareMode = FILE_SHARE_READ;
		break;
	case shareDenyRead:
		dwShareMode = FILE_SHARE_WRITE;
		break;
	case shareDenyNone:
		dwShareMode = FILE_SHARE_WRITE | FILE_SHARE_READ;
		break;
	}

	// Note: typeText and typeBinary are used in derived classes only.

	// map modeNoInherit flag
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = (nOpenFlags & modeNoInherit) == 0;

	// map creation flags
	DWORD dwCreateFlag;
	if (nOpenFlags & modeCreate)
	{
		if (nOpenFlags & modeNoTruncate)
			dwCreateFlag = OPEN_ALWAYS;
		else
			dwCreateFlag = CREATE_ALWAYS;
	}
	else
		dwCreateFlag = OPEN_EXISTING;

	// special system-level access flags

	// Random access and sequential scan should be mutually exclusive
	ATLASSERT((nOpenFlags&(osRandomAccess|osSequentialScan)) != (osRandomAccess|
		osSequentialScan) );

	DWORD dwFlags = FILE_ATTRIBUTE_NORMAL;
	if (nOpenFlags & osNoBuffer)
		dwFlags |= FILE_FLAG_NO_BUFFERING;
	if (nOpenFlags & osWriteThrough)
		dwFlags |= FILE_FLAG_WRITE_THROUGH;
	if (nOpenFlags & osRandomAccess)
		dwFlags |= FILE_FLAG_RANDOM_ACCESS;
	if (nOpenFlags & osSequentialScan)
		dwFlags |= FILE_FLAG_SEQUENTIAL_SCAN;

	// attempt file creation
	HANDLE hFile = ::CreateFile(lpszFileName, dwAccess, dwShareMode, &sa, dwCreateFlag, dwFlags, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	m_hFile = hFile;
	m_bCloseOnDelete = TRUE;

	return TRUE;
}

UINT QFile::Read(void* lpBuf, UINT nCount)
{
	
	ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);

	if (nCount == 0)
		return 0;   // avoid Win32 "null-read"

	ATLASSERT(lpBuf != NULL);
	ATLASSERT(AtlIsValidAddress(lpBuf, nCount));

	DWORD dwRead;
	if (!::ReadFile(m_hFile, lpBuf, nCount, &dwRead, NULL))
		QFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);

	return (UINT)dwRead;
}

void QFile::Write(const void* lpBuf, UINT nCount)
{
	
	ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);

	if (nCount == 0)
		return;     // avoid Win32 "null-write" option

	ATLASSERT(lpBuf != NULL);
	ATLASSERT(AtlIsValidAddress(lpBuf, nCount, FALSE));

	DWORD nWritten;
	if (!::WriteFile(m_hFile, lpBuf, nCount, &nWritten, NULL))
		QFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);

	if (nWritten != nCount)
		AfxThrowFileException(QFileException::diskFull, -1, m_strFileName);
}

ULONGLONG QFile::Seek(LONGLONG lOff, UINT nFrom)
{
	
	ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
	ATLASSERT(nFrom == begin || nFrom == end || nFrom == current);
	ATLASSERT(begin == FILE_BEGIN && end == FILE_END && current == FILE_CURRENT);

   LARGE_INTEGER liOff;

   liOff.QuadPart = lOff;
	liOff.LowPart = ::SetFilePointer(m_hFile, liOff.LowPart, &liOff.HighPart,
	  (DWORD)nFrom);
	if (liOff.LowPart  == (DWORD)-1)
	  if (::GetLastError() != NO_ERROR)
		   QFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);

	return liOff.QuadPart;
}

ULONGLONG QFile::GetPosition() const
{
	
	ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);

   LARGE_INTEGER liPos;
   liPos.QuadPart = 0;
	liPos.LowPart = ::SetFilePointer(m_hFile, liPos.LowPart, &liPos.HighPart , FILE_CURRENT);
	if (liPos.LowPart == (DWORD)-1)
	  if (::GetLastError() != NO_ERROR)
		   QFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);

	return liPos.QuadPart;
}

void QFile::Flush()
{
	

	if (m_hFile == INVALID_HANDLE_VALUE)
		return;

	if (!::FlushFileBuffers(m_hFile))
		QFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
}

void QFile::Close()
{
	
	ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
	BOOL bError = FALSE;
	if (m_hFile != INVALID_HANDLE_VALUE)
		bError = !::CloseHandle(m_hFile);

	m_hFile = INVALID_HANDLE_VALUE;
	m_bCloseOnDelete = FALSE;
	m_strFileName.Empty();

	if (bError)
		QFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
}

void QFile::Abort()
{
	
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		// close but ignore errors
		::CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
	m_strFileName.Empty();
}

void QFile::LockRange(ULONGLONG dwPos, ULONGLONG dwCount)
{
	
	ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);

   ULARGE_INTEGER liPos;
   ULARGE_INTEGER liCount;

   liPos.QuadPart = dwPos;
   liCount.QuadPart = dwCount;
	if (!::LockFile(m_hFile, liPos.LowPart, liPos.HighPart, liCount.LowPart, 
	  liCount.HighPart))
   {
		QFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
   }
}

void QFile::UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount)
{
	
	ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);

   ULARGE_INTEGER liPos;
   ULARGE_INTEGER liCount;

   liPos.QuadPart = dwPos;
   liCount.QuadPart = dwCount;
	if (!::UnlockFile(m_hFile, liPos.LowPart, liPos.HighPart, liCount.LowPart,
	  liCount.HighPart))
   {
		QFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
   }
}

void QFile::SetLength(ULONGLONG dwNewLen)
{
	
	ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);

	Seek(dwNewLen, (UINT)begin);

	if (!::SetEndOfFile(m_hFile))
		QFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
}

ULONGLONG QFile::GetLength() const
{
	

   ULARGE_INTEGER liSize;
   liSize.LowPart = ::GetFileSize(m_hFile, &liSize.HighPart);
   if (liSize.LowPart == INVALID_FILE_SIZE)
	  if (::GetLastError() != NO_ERROR)
		 QFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);

	return liSize.QuadPart;
}

// CFile does not support direct buffering (CMemFile does)
UINT QFile::GetBufferPtr(UINT nCommand, UINT /*nCount*/,
	void** /*ppBufStart*/, void** /*ppBufMax*/)
{
	ATLASSERT(nCommand == bufferCheck);
	UNUSED(nCommand);    // not used in retail build

	return 0;   // no support
}

void PASCAL QFile::Rename(LPCTSTR lpszOldName, LPCTSTR lpszNewName, CAtlTransactionManager* pTM)
{
	BOOL bRes = (pTM != NULL) ? pTM->MoveFile(lpszOldName, lpszNewName) : ::MoveFile((LPTSTR)lpszOldName, (LPTSTR)lpszNewName);
	if (!bRes)
		QFileException::ThrowOsError((LONG)::GetLastError(), lpszOldName);
}

void PASCAL QFile::Remove(LPCTSTR lpszFileName, CAtlTransactionManager* pTM)
{
	BOOL bRes = (pTM != NULL) ? pTM->DeleteFile(lpszFileName) : ::DeleteFile((LPTSTR)lpszFileName);
	if (!bRes)
		QFileException::ThrowOsError((LONG)::GetLastError(), lpszFileName);
}

////////////////////////////////////////////////////////////////////////////
// CFileFind implementation

QFileFind::QFileFind()
{
	m_pFoundInfo = NULL;
	m_pNextInfo = NULL;
	m_hContext = NULL;
	m_chDirSeparator = '\\';
	m_pTM = NULL;
}

QFileFind::QFileFind(CAtlTransactionManager* pTM)
{
	m_pFoundInfo = NULL;
	m_pNextInfo = NULL;
	m_hContext = NULL;
	m_chDirSeparator = '\\';
	m_pTM = pTM;
}

QFileFind::~QFileFind()
{
	Close();
}

void QFileFind::Close()
{
	if (m_pFoundInfo != NULL)
	{
		delete m_pFoundInfo;
		m_pFoundInfo = NULL;
	}

	if (m_pNextInfo != NULL)
	{
		delete m_pNextInfo;
		m_pNextInfo = NULL;
	}

	if (m_hContext != NULL)
	{
		if (m_hContext != INVALID_HANDLE_VALUE)
			CloseContext();
		m_hContext = NULL;
	}
}

void QFileFind::CloseContext()
{
	::FindClose(m_hContext);
	return;
}

BOOL QFileFind::FindFile(LPCTSTR pstrName /* = NULL */,
	DWORD dwUnused /* = 0 */)
{
	Close();

	if (pstrName == NULL)
		pstrName = _T("*.*");
	else if (lstrlen(pstrName) >= (_countof(((WIN32_FIND_DATA*) m_pNextInfo)->cFileName)))
	{
		::SetLastError(ERROR_BAD_ARGUMENTS);
		return FALSE;		
	}

	m_pNextInfo = new WIN32_FIND_DATA;

	WIN32_FIND_DATA *pFindData = (WIN32_FIND_DATA *)m_pNextInfo;

	Checked::tcscpy_s(pFindData->cFileName, _countof(pFindData->cFileName), pstrName);

	m_hContext = m_pTM != NULL ?
		m_pTM->FindFirstFile(pstrName, (WIN32_FIND_DATA*) m_pNextInfo) :
	::FindFirstFile(pstrName, (WIN32_FIND_DATA*) m_pNextInfo);

	if (m_hContext == INVALID_HANDLE_VALUE)
	{
		DWORD dwTemp = ::GetLastError();
		Close();
		::SetLastError(dwTemp);
		return FALSE;
	}

	LPTSTR pstrRoot = m_strRoot.GetBufferSetLength(_MAX_PATH);
	LPCTSTR pstr = _tfullpath(pstrRoot, pstrName, _MAX_PATH);

	// passed name isn't a valid path but was found by the API
	ATLASSERT(pstr != NULL);
	if (pstr == NULL)
	{
		m_strRoot.ReleaseBuffer(0);
		Close();
		::SetLastError(ERROR_INVALID_NAME);
		return FALSE;
	}
	else
	{
		TCHAR strDrive[_MAX_DRIVE], strDir[_MAX_DIR];
		Checked::tsplitpath_s(pstrRoot, strDrive, _MAX_DRIVE, strDir, _MAX_DIR, NULL, 0, NULL, 0);
		Checked::tmakepath_s(pstrRoot, _MAX_PATH, strDrive, strDir, NULL, NULL);
		m_strRoot.ReleaseBuffer(-1);
	}
	return TRUE;
}

BOOL QFileFind::MatchesMask(DWORD dwMask) const
{
	ATLASSERT(m_hContext != NULL);
	ATLASSERT(this);

	if (m_pFoundInfo != NULL)
		return (!!(((LPWIN32_FIND_DATA) m_pFoundInfo)->dwFileAttributes & dwMask));
	else
		return FALSE;
}

BOOL QFileFind::GetLastAccessTime(FILETIME* pTimeStamp) const
{
	ATLASSERT(m_hContext != NULL);
	ATLASSERT(pTimeStamp != NULL);
	ATLASSERT(this);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastAccessTime;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL QFileFind::GetLastWriteTime(FILETIME* pTimeStamp) const
{
	ATLASSERT(m_hContext != NULL);
	ATLASSERT(pTimeStamp != NULL);
	ATLASSERT(this);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastWriteTime;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL QFileFind::GetCreationTime(FILETIME* pTimeStamp) const
{
	ATLASSERT(m_hContext != NULL);
	ATLASSERT(this);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPWIN32_FIND_DATA) m_pFoundInfo)->ftCreationTime;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL QFileFind::GetLastAccessTime(QTime& refTime) const
{
	ATLASSERT(m_hContext != NULL);
	ATLASSERT(this);

	if (m_pFoundInfo != NULL)
	{
		if (QTime::IsValidFILETIME(((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastAccessTime))
		{
			refTime = QTime(((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastAccessTime);
		}
		else
		{
			refTime = QTime();
		}
		return TRUE;
	}
	else
		return FALSE;
}

BOOL QFileFind::GetLastWriteTime(QTime& refTime) const
{
	ATLASSERT(m_hContext != NULL);
	ATLASSERT(this);

	if (m_pFoundInfo != NULL)
	{
		if (QTime::IsValidFILETIME(((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastWriteTime))
		{
			refTime = QTime(((LPWIN32_FIND_DATA) m_pFoundInfo)->ftLastWriteTime);
		}
		else
		{
			refTime = QTime();
		}
		return TRUE;
	}
	else
		return FALSE;
}

BOOL QFileFind::GetCreationTime(QTime& refTime) const
{
	ATLASSERT(m_hContext != NULL);
	ATLASSERT(this);

	if (m_pFoundInfo != NULL)
	{
		if (QTime::IsValidFILETIME(((LPWIN32_FIND_DATA) m_pFoundInfo)->ftCreationTime))
		{
			refTime = QTime(((LPWIN32_FIND_DATA) m_pFoundInfo)->ftCreationTime);
		}
		else
		{
			refTime = QTime();
		}
		return TRUE;
	}
	else
		return FALSE;
}

BOOL QFileFind::IsDots() const
{
	ATLASSERT(m_hContext != NULL);
	ATLASSERT(this);

	// return TRUE if the file name is "." or ".." and
	// the file is a directory

	BOOL bResult = FALSE;
	if (m_pFoundInfo != NULL && IsDirectory())
	{
		LPWIN32_FIND_DATA pFindData = (LPWIN32_FIND_DATA) m_pFoundInfo;
		if (pFindData->cFileName[0] == '.')
		{
			if (pFindData->cFileName[1] == '\0' ||
				(pFindData->cFileName[1] == '.' &&
				pFindData->cFileName[2] == '\0'))
			{
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

BOOL QFileFind::FindNextFile()
{
	ATLASSERT(m_hContext != NULL);

	if (m_hContext == NULL)
		return FALSE;
	if (m_pFoundInfo == NULL)
		m_pFoundInfo = new WIN32_FIND_DATA;

	ATLASSERT(this);

	void* pTemp = m_pFoundInfo;
	m_pFoundInfo = m_pNextInfo;
	m_pNextInfo = pTemp;

	return ::FindNextFile(m_hContext, (LPWIN32_FIND_DATA) m_pNextInfo);
}

CStdString QFileFind::GetFileURL() const
{
	ATLASSERT(m_hContext != NULL);
	ATLASSERT(this);

	CStdString strResult("file://");
	strResult += GetFilePath();
	return strResult;
}

CStdString QFileFind::GetRoot() const
{
	ATLASSERT(m_hContext != NULL);
	ATLASSERT(this);

	return m_strRoot;
}

CStdString QFileFind::GetFilePath() const
{
	ATLASSERT(m_hContext != NULL);
	ATLASSERT(this);

	CStdString strResult = m_strRoot;
	LPCTSTR pszResult;
	LPCTSTR pchLast;
	pszResult = strResult;
	pchLast = _tcsdec( pszResult, pszResult+strResult.GetLength() );
	ATLASSERT(pchLast!=NULL);
	if ((*pchLast != _T('\\')) && (*pchLast != _T('/')))
		strResult += m_chDirSeparator;
	strResult += GetFileName();
	return strResult;
}

CStdString QFileFind::GetFileName() const
{
	ATLASSERT(m_hContext != NULL);
	ATLASSERT(this);

	CStdString ret;

	if (m_pFoundInfo != NULL)
		ret = ((LPWIN32_FIND_DATA) m_pFoundInfo)->cFileName;
	return ret;
}

ULONGLONG QFileFind::GetLength() const
{
	ATLASSERT(m_hContext != NULL);
	ATLASSERT(this);

	ULARGE_INTEGER nFileSize;

	if (m_pFoundInfo != NULL)
	{
		nFileSize.LowPart = ((LPWIN32_FIND_DATA) m_pFoundInfo)->nFileSizeLow;
		nFileSize.HighPart = ((LPWIN32_FIND_DATA) m_pFoundInfo)->nFileSizeHigh;
	}
	else
	{
		nFileSize.QuadPart = 0;
	}

	return nFileSize.QuadPart;
}

// CFileFind
BOOL QFileFind::IsReadOnly() const
{ return MatchesMask(FILE_ATTRIBUTE_READONLY); }
BOOL QFileFind::IsDirectory() const
{ return MatchesMask(FILE_ATTRIBUTE_DIRECTORY); }
BOOL QFileFind::IsCompressed() const
{ return MatchesMask(FILE_ATTRIBUTE_COMPRESSED); }
BOOL QFileFind::IsSystem() const
{ return MatchesMask(FILE_ATTRIBUTE_SYSTEM); }
BOOL QFileFind::IsHidden() const
{ return MatchesMask(FILE_ATTRIBUTE_HIDDEN); }
BOOL QFileFind::IsTemporary() const
{ return MatchesMask(FILE_ATTRIBUTE_TEMPORARY); }
BOOL QFileFind::IsNormal() const
{ return MatchesMask(FILE_ATTRIBUTE_NORMAL); }
BOOL QFileFind::IsArchived() const
{ return MatchesMask(FILE_ATTRIBUTE_ARCHIVE); }

//////////////////////////////////////////////////////////////////////////
// QStdioFile

void ThrowInvalidArgException() { throw L"Invalid Argument!"; }
void AfxThrowNotSupportedException() { throw L""; }

QStdioFile::QStdioFile()
{
	m_pStream = NULL;
	m_strFileName = L"";
}

QStdioFile::QStdioFile(LPCTSTR lpszFileName, UINT nOpenFlags)
{
	CommonInit(lpszFileName, nOpenFlags, NULL);
}

QStdioFile::QStdioFile(LPCTSTR lpszFileName, UINT nOpenFlags, CAtlTransactionManager* pTM)
{
	CommonInit(lpszFileName, nOpenFlags, pTM);
}

QStdioFile::QStdioFile( FILE* p )
{
	ATLASSERT(FALSE);
}

void QStdioFile::CommonInit(LPCTSTR lpszFileName, UINT nOpenFlags, CAtlTransactionManager* pTM)
{
	QFileException e;
	if (!Open(lpszFileName, nOpenFlags, &e))
	{
		AfxThrowFileException(e.m_cause, e.m_lOsError, e.m_strFileName);
	}
}

QStdioFile::~QStdioFile()
{
	if (m_pStream != NULL && m_bCloseOnDelete)
	{
		Close();
	}
}
BOOL QStdioFile::Open(LPCTSTR lpszFileName, UINT nOpenFlags, QFileException* pException)
{
	ATLASSERT(lpszFileName != NULL);

	if (lpszFileName == NULL)
	{
		return FALSE;
	}

	m_pStream = NULL;
	if (!QFile::Open(lpszFileName, (nOpenFlags & ~typeText), pException))
		return FALSE;

	ATLASSERT(m_hFile != hFileNull);
	ATLASSERT(m_bCloseOnDelete);

	char szMode[4]; // C-runtime open string
	int nMode = 0;

	// determine read/write mode depending on CFile mode
	if (nOpenFlags & modeCreate)
	{
		if (nOpenFlags & modeNoTruncate)
			szMode[nMode++] = 'a';
		else
			szMode[nMode++] = 'w';
	}
	else if (nOpenFlags & modeWrite)
		szMode[nMode++] = 'a';
	else
		szMode[nMode++] = 'r';

	// add '+' if necessary (when read/write modes mismatched)
	if (szMode[0] == 'r' && (nOpenFlags & modeReadWrite) ||
		szMode[0] != 'r' && !(nOpenFlags & modeWrite))
	{
		// current szMode mismatched, need to add '+' to fix
		szMode[nMode++] = '+';
	}

	// will be inverted if not necessary
	int nFlags = _O_RDONLY|_O_TEXT;
	if (nOpenFlags & (modeWrite|modeReadWrite))
		nFlags ^= _O_RDONLY;

	if (nOpenFlags & typeBinary)
		szMode[nMode++] = 'b', nFlags ^= _O_TEXT;
	else
		szMode[nMode++] = 't';
	szMode[nMode++] = '\0';

	// open a C-runtime low-level file handle
	int nHandle = _open_osfhandle((UINT_PTR) m_hFile, nFlags);

	// open a C-runtime stream from that handle
	if (nHandle != -1)
		m_pStream = _fdopen(nHandle, szMode);

	if (m_pStream == NULL)
	{
		// an error somewhere along the way...
		if (pException != NULL)
		{
			pException->m_lOsError = _doserrno;
			pException->m_cause = QFileException::OsErrorToException(_doserrno);
		}

		QFile::Abort(); // close m_hFile
		return FALSE;
	}

	return TRUE;
}

UINT QStdioFile::Read(void* lpBuf, UINT nCount)
{
	
	ATLASSERT(m_pStream != NULL);

	if (nCount == 0)
		return 0;   // avoid Win32 "null-read"

	if (lpBuf == NULL)
	{
		ThrowInvalidArgException();
	}

	UINT nRead = 0;

	if ((nRead = (UINT)fread(lpBuf, sizeof(BYTE), nCount, m_pStream)) == 0 && !feof(m_pStream))
		AfxThrowFileException(QFileException::genericException, _doserrno, m_strFileName);
	if (ferror(m_pStream))
	{
		::clearerr_s(m_pStream);
		AfxThrowFileException(QFileException::genericException, _doserrno, m_strFileName);
	}
	return nRead;
}

void QStdioFile::Write(const void* lpBuf, UINT nCount)
{
	
	ATLASSERT(m_pStream != NULL);

	if (lpBuf == NULL)
	{
		ThrowInvalidArgException();
	}

	if (fwrite(lpBuf, sizeof(BYTE), nCount, m_pStream) != nCount)
		AfxThrowFileException(QFileException::genericException, _doserrno, m_strFileName);
}

void QStdioFile::WriteString(LPCTSTR lpsz)
{
	ATLASSERT(lpsz != NULL);
	ATLASSERT(m_pStream != NULL);

	if (lpsz == NULL)
	{
		ThrowInvalidArgException();
	}

	if (_fputts(lpsz, m_pStream) == _TEOF)
		AfxThrowFileException(QFileException::diskFull, _doserrno, m_strFileName);
}

LPTSTR QStdioFile::ReadString(_Out_z_cap_(nMax) LPTSTR lpsz, __in UINT nMax)
{
	ATLASSERT(lpsz != NULL);
	
	ATLASSERT(m_pStream != NULL);

	if (lpsz == NULL)
	{
		ThrowInvalidArgException();
	}

	LPTSTR lpszResult = _fgetts(lpsz, nMax, m_pStream);
	if (lpszResult == NULL && !feof(m_pStream))
	{
		::clearerr_s(m_pStream);
		AfxThrowFileException(QFileException::genericException, _doserrno, m_strFileName);
	}
	return lpszResult;
}

BOOL QStdioFile::ReadString(CStdString& rString)
{
	

	rString = _T("");    // empty string without deallocating
	const int nMaxSize = 128;
	LPTSTR lpsz = rString.GetBuffer(nMaxSize);
	LPTSTR lpszResult;
	int nLen = 0;
	for (;;)
	{
		lpszResult = _fgetts(lpsz, nMaxSize+1, m_pStream);
		rString.ReleaseBuffer();

		// handle error/eof case
		if (lpszResult == NULL && !feof(m_pStream))
		{
			::clearerr_s(m_pStream);
			AfxThrowFileException(QFileException::genericException, _doserrno,
				m_strFileName);
		}

		// if string is read completely or EOF
		if (lpszResult == NULL ||
			(nLen = (int)lstrlen(lpsz)) < nMaxSize ||
			lpsz[nLen-1] == '\n')
			break;

		nLen = rString.GetLength();
		lpsz = rString.GetBuffer(nMaxSize + nLen) + nLen;
	}

	// remove '\n' from end of string if present
	lpsz = rString.GetBuffer(0);
	nLen = rString.GetLength();
	if (nLen != 0 && lpsz[nLen-1] == '\n')
		rString.GetBufferSetLength(nLen-1);

	return nLen != 0;
}

ULONGLONG QStdioFile::Seek(LONGLONG lOff, UINT nFrom)
{
	
	ATLASSERT(nFrom == begin || nFrom == end || nFrom == current);
	ATLASSERT(m_pStream != NULL);

	LONG lOff32;

	if ((lOff < LONG_MIN) || (lOff > LONG_MAX))
	{
		AfxThrowFileException(QFileException::badSeek, -1, m_strFileName);
	}

	lOff32 = (LONG)lOff;
	if (fseek(m_pStream, lOff32, nFrom) != 0)
		AfxThrowFileException(QFileException::badSeek, _doserrno,
		m_strFileName);

	long pos = ftell(m_pStream);
	return pos;
}

ULONGLONG QStdioFile::GetLength() const
{
	

	LONG nCurrent;
	LONG nLength;
	LONG nResult;

	nCurrent = ftell(m_pStream);
	if (nCurrent == -1)
		AfxThrowFileException(QFileException::invalidFile, _doserrno,
		m_strFileName);

	nResult = fseek(m_pStream, 0, SEEK_END);
	if (nResult != 0)
		AfxThrowFileException(QFileException::badSeek, _doserrno,
		m_strFileName);

	nLength = ftell(m_pStream);
	if (nLength == -1)
		AfxThrowFileException(QFileException::invalidFile, _doserrno,
		m_strFileName);
	nResult = fseek(m_pStream, nCurrent, SEEK_SET);
	if (nResult != 0)
		AfxThrowFileException(QFileException::badSeek, _doserrno,
		m_strFileName);

	return nLength;
}

ULONGLONG QStdioFile::GetPosition() const
{
	
	ATLASSERT(m_pStream != NULL);

	long pos = ftell(m_pStream);
	if (pos == -1)
		AfxThrowFileException(QFileException::invalidFile, _doserrno,
		m_strFileName);
	return pos;
}

void QStdioFile::Flush()
{
	

	if (m_pStream != NULL && fflush(m_pStream) != 0)
		AfxThrowFileException(QFileException::diskFull, _doserrno,
		m_strFileName);
}

void QStdioFile::Close()
{
	
	ATLASSERT(m_pStream != NULL);

	int nErr = 0;

	if (m_pStream != NULL)
		nErr = fclose(m_pStream);

	m_hFile = hFileNull;
	m_bCloseOnDelete = FALSE;
	m_pStream = NULL;

	if (nErr != 0)
		AfxThrowFileException(QFileException::diskFull, _doserrno,
		m_strFileName);
}

void QStdioFile::Abort()
{
	

	if (m_pStream != NULL && m_bCloseOnDelete)
		fclose(m_pStream);  // close but ignore errors
	m_hFile = hFileNull;
	m_pStream = NULL;
	m_bCloseOnDelete = FALSE;
}

QFile* QStdioFile::Duplicate() const
{
	
	ATLASSERT(m_pStream != NULL);

	AfxThrowNotSupportedException();

	return NULL;
}

void QStdioFile::LockRange(ULONGLONG /* dwPos */, ULONGLONG /* dwCount */)
{
	
	ATLASSERT(m_pStream != NULL);

	AfxThrowNotSupportedException();
}

void QStdioFile::UnlockRange(ULONGLONG /* dwPos */, ULONGLONG /* dwCount */)
{
	
	ATLASSERT(m_pStream != NULL);

	AfxThrowNotSupportedException();
}


//////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------
// StdioFileEx
//--------------------------------------------------------------------

// StdioFileEx.cpp: implementation of the CStdioFileEx class.
//
// Version 1.1 23 August 2003. Incorporated fixes from Dennis Jeryd.
// Version 1.3 19 February 2005. Incorporated fixes Howard J Oh and some of my own.
//
// Copyright David Pritchard 2003-2005. davidpritchard@ctv.es
//
// You can use this class freely, but please keep my ego happy 
// by leaving this comment in place.
//
//////////////////////////////////////////////////////////////////////

/*static*/ const UINT CStdioFileEx::modeWriteUnicode = 0x20000; // Add this flag to write in Unicode

CStdioFileEx::CStdioFileEx(): QStdioFile()
{
	m_bIsUnicodeText = false;
}

CStdioFileEx::CStdioFileEx(LPCTSTR lpszFileName,UINT nOpenFlags)
	:QStdioFile(lpszFileName, ProcessFlags(lpszFileName, nOpenFlags))
{
}

BOOL CStdioFileEx::Open(LPCTSTR lpszFileName,UINT nOpenFlags,
	QFileException* pError /*=NULL*/)
{
	// Process any Unicode stuff
	ProcessFlags(lpszFileName, nOpenFlags);

	return QStdioFile::Open(lpszFileName, nOpenFlags, pError);
}

BOOL CStdioFileEx::ReadString(CStdString& rString)
{
	BOOL		bReadData;
	LPTSTR		lpsz;
	int		nLen = 0; //, nMultiByteBufferLength = 0, nChars = 0;
	CStdString		sTemp;

	// If at position 0, discard byte-order mark before reading
	if (!m_pStream || (GetPosition() == 0 && m_bIsUnicodeText))
	{
		wchar_t	cDummy;
		Read(&cDummy, sizeof(wchar_t));
	}

	// If compiled for Unicode
#ifdef _UNICODE
	// Do standard stuff -- both ANSI and Unicode cases seem to work OK
	bReadData = QStdioFile::ReadString(rString);
#else

	if (!m_bIsUnicodeText)
	{
		// Do standard stuff -- read ANSI in ANSI
		bReadData = QStdioFile::ReadString(rString);
	}
	else
	{
		const int nMAX_LINE_CHARS = 4096;
		wchar_t* pszUnicodeString = new wchar_t[nMAX_LINE_CHARS]; 
		char* pszMultiByteString= new char[nMAX_LINE_CHARS];  

		// Read as Unicode, convert to ANSI; fixed by Dennis Jeryd 6/8/03
		bReadData = (NULL != fgetws(pszUnicodeString, nMAX_LINE_CHARS, m_pStream));
		if (GetMultiByteStringFromUnicodeString(pszUnicodeString, pszMultiByteString, nMAX_LINE_CHARS))
		{
			rString = (CStdString)pszMultiByteString;
		}

		if (pszUnicodeString)
		{
			delete pszUnicodeString;
		}

		if (pszMultiByteString)
		{
			delete pszMultiByteString;
		}
	}
#endif

	// Then remove end-of-line character if in Unicode text mode
	if (bReadData)
	{
		// Copied from FileTxt.cpp but adapted to Unicode and then adapted for end-of-line being just '\r'. 
		nLen = rString.GetLength();
		if (nLen > 1 && rString.Mid(nLen-2) == sNEWLINE)
		{
			rString.GetBufferSetLength(nLen-2);
		}
		else
		{
			lpsz = rString.GetBuffer(0);
			if (nLen != 0 && (lpsz[nLen-1] == _T('\r') || lpsz[nLen-1] == _T('\n')))
			{
				rString.GetBufferSetLength(nLen-1);
			}
		}
	}

	return bReadData;
}

// --------------------------------------------------------------------------------------------
//
//	CStdioFileEx::WriteString()
//
// --------------------------------------------------------------------------------------------
// Returns:    void
// Parameters: LPCTSTR lpsz
//
// Purpose:		Writes string to file either in Unicode or multibyte, depending on whether the caller specified the
//					CStdioFileEx::modeWriteUnicode flag. Override of base class function.
// Notes:		If writing in Unicode we need to:
//						a) Write the Byte-order-mark at the beginning of the file
//						b) Write all strings in byte-mode
//					-	If we were compiled in Unicode, we need to convert Unicode to multibyte if 
//						we want to write in multibyte
//					-	If we were compiled in multi-byte, we need to convert multibyte to Unicode if 
//						we want to write in Unicode.
// Exceptions:	None.
//
void CStdioFileEx::WriteString(LPCTSTR lpsz)
{
	// If writing Unicode and at the start of the file, need to write byte mark
	if (m_nFlags & CStdioFileEx::modeWriteUnicode)
	{
		// If at position 0, write byte-order mark before writing anything else
		if (!m_pStream || GetPosition() == 0)
		{
			wchar_t cBOM = (wchar_t)nUNICODE_BOM;
			QFile::Write(&cBOM, sizeof(wchar_t));
		}
	}

	// If compiled in Unicode...
#ifdef _UNICODE

	// If writing Unicode, no conversion needed
	if (m_nFlags & CStdioFileEx::modeWriteUnicode)
	{
		// Write in byte mode
		QFile::Write(lpsz, lstrlen(lpsz) * sizeof(wchar_t));
	}
	// Else if we don't want to write Unicode, need to convert
	else
	{
		int		nChars = lstrlen(lpsz) + 1;				// Why plus 1? Because yes
		int		nBufferSize = nChars * sizeof(char);
		wchar_t*	pszUnicodeString	= new wchar_t[nChars]; 
		char	*	pszMultiByteString= new char[nChars];  
		int		nCharsWritten = 0;

		// Copy string to Unicode buffer
		StringCchCopy(pszUnicodeString,nChars, lpsz);
		// Get multibyte string
		nCharsWritten = 
			GetMultiByteStringFromUnicodeString(pszUnicodeString, pszMultiByteString, ( short ) nBufferSize, GetACP());

		if (nCharsWritten > 0)
		{
			//   CFile::Write((const void*)pszMultiByteString, lstrlen(lpsz));

			// Do byte-mode write using actual chars written (fix by Howard J Oh)
			QFile::Write((const void*)pszMultiByteString,
				nCharsWritten*sizeof(char));
		}

		if (pszUnicodeString && pszMultiByteString)
		{
			delete [] pszUnicodeString;
			delete [] pszMultiByteString;
		}
	}
	// Else if *not* compiled in Unicode
#else
	// If writing Unicode, need to convert
	if (m_nFlags & CStdioFileEx::modeWriteUnicode)
	{
		int		nChars = lstrlen(lpsz) + 1;	 // Why plus 1? Because yes
		wchar_t*	pszUnicodeString	= new wchar_t[nChars];
		char	*	pszMultiByteString= new char[nChars]; 
		int		nCharsWritten = 0;

		// Copy string to multibyte buffer
		StringCchCopy(pszMultiByteString, lpsz);

		nCharsWritten =
			GetUnicodeStringFromMultiByteString(pszMultiByteString,
			pszUnicodeString, nChars, GetACP());

		if (nCharsWritten > 0)
		{
			//   CFile::Write(pszUnicodeString, lstrlen(lpsz) * sizeof(wchar_t));

			// Write in byte mode. Write actual number of chars written * bytes (fix by Howard J Oh)
			QFile::Write(pszUnicodeString, nCharsWritten*sizeof(wchar_t));
		}
		else
		{
			ATLASSERT(false);
		}

		if (pszUnicodeString && pszMultiByteString)
		{
			delete [] pszUnicodeString;
			delete [] pszMultiByteString;
		}
	}
	// Else if we don't want to write Unicode, no conversion needed
	else
	{
		// Do standard stuff
		//CStdioFile::WriteString(lpsz);

		// Do byte-mode write. This avoids annoying "interpretation" of \n's as	\r\n
		QFile::Write((const void*)lpsz, lstrlen(lpsz)*sizeof(char));
	}

#endif
}

UINT CStdioFileEx::ProcessFlags(const CStdString& sFilePath, UINT& nOpenFlags)
{
	m_bIsUnicodeText = false;

	// If we have writeUnicode we must have write or writeRead as well
#ifdef _DEBUG
	if (nOpenFlags & CStdioFileEx::modeWriteUnicode)
	{
		ATLASSERT(nOpenFlags & QFile::modeWrite || nOpenFlags & QFile::modeReadWrite);
	}
#endif

	// If reading in text mode and not creating... ; fixed by Dennis Jeryd 6/8/03
	if (nOpenFlags & QFile::typeText && !(nOpenFlags & QFile::modeCreate) && !(nOpenFlags & QFile::modeWrite ))
	{
		m_bIsUnicodeText = IsFileUnicode(sFilePath);

		// If it's Unicode, switch to binary mode
		if (m_bIsUnicodeText)
		{
			nOpenFlags ^= QFile::typeText;
			nOpenFlags |= QFile::typeBinary;
		}
	}

	m_nFlags = nOpenFlags;

	return nOpenFlags;
}

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
/*static*/ bool CStdioFileEx::IsFileUnicode(const CStdString& sFilePath)
{
	QFile				file;
	bool				bIsUnicode = false;
	wchar_t			cFirstChar;
	QFileException	exFile;

	// Open file in binary mode and read first character
	if (file.Open(sFilePath, QFile::typeBinary | QFile::modeRead, &exFile))
	{
		// If byte is Unicode byte-order marker, let's say it's Unicode
		if (file.Read(&cFirstChar, sizeof(wchar_t)) > 0 && cFirstChar == (wchar_t)nUNICODE_BOM)
		{
			bIsUnicode = true;
		}

		file.Close();
	}
	else
	{
		// Handle error here if you like
	}

	return bIsUnicode;
}

unsigned long CStdioFileEx::GetCharCount()
{
	int				nCharSize;
	unsigned long	nByteCount, nCharCount = 0;

	if (m_pStream)
	{
		// Get size of chars in file
		nCharSize = m_bIsUnicodeText ? sizeof(wchar_t): sizeof(char);

		// If Unicode, remove byte order mark from count
		nByteCount = GetLength();

		if (m_bIsUnicodeText)
		{
			nByteCount = nByteCount - sizeof(wchar_t);
		}

		// Calc chars
		nCharCount = (nByteCount / nCharSize);
	}

	return nCharCount;
}

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
int CStdioFileEx::GetUnicodeStringFromMultiByteString(IN char * szMultiByteString, OUT wchar_t* szUnicodeString, IN OUT int& nUnicodeBufferSize, IN UINT nCodePage)
{
	int		nCharsWritten = 0;

	if (szUnicodeString && szMultiByteString)
	{
		// If no code page specified, take default for system
		if (nCodePage == -1)
		{
			nCodePage = GetACP();
		}

		try 
		{
			// Zero out buffer first. NB: nUnicodeBufferSize is NUMBER OF CHARS, NOT BYTES!
			memset((void*)szUnicodeString, '\0', sizeof(wchar_t) *
				nUnicodeBufferSize);

			nCharsWritten = MultiByteToWideChar(nCodePage,MB_PRECOMPOSED,szMultiByteString,-1,szUnicodeString,nUnicodeBufferSize);
		}
		catch(...)
		{
			TRACE(_T("Controlled exception in MultiByteToWideChar!\n"));
		}
	}

	// Now fix nCharsWritten
	if (nCharsWritten > 0)
	{
		nCharsWritten--;
	}

	ATLASSERT(nCharsWritten > 0);
	return nCharsWritten;
}

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
// Purpose:		Gets a MultiByte string from a Unicode string
// Notes:		None.
// Exceptions:	None.
//
int CStdioFileEx::GetMultiByteStringFromUnicodeString(wchar_t * szUnicodeString, char* szMultiByteString, 
	short nMultiByteBufferSize, UINT nCodePage)
{
	BOOL		bUsedDefChar	= FALSE;
	int		nCharsWritten = 0;

	if (szUnicodeString && szMultiByteString) 
	{
		// Zero out buffer first
		memset((void*)szMultiByteString, '\0', nMultiByteBufferSize);

		// If no code page specified, take default for system
		if (nCodePage == -1)
		{
			nCodePage = GetACP();
		}

		try 
		{
			nCharsWritten = WideCharToMultiByte(nCodePage, WC_COMPOSITECHECK | WC_SEPCHARS,
				szUnicodeString,-1, szMultiByteString, nMultiByteBufferSize, sDEFAULT_UNICODE_FILLER_CHAR, &bUsedDefChar);
		}
		catch(...) 
		{
			TRACE(_T("Controlled exception in WideCharToMultiByte!\n"));
		}
	} 

	// Now fix nCharsWritten 
	if (nCharsWritten > 0)
	{
		nCharsWritten--;
	}

	return nCharsWritten;
}

//////////////////////////////////////////////////////////////////
// CStdioFile_UTF8
//////////////////////////////////////////////////////////////////
/**  
* Read an UTF8 encoded string from storage and convert it as appropriate.  
* The code is snipped from the MFC implementation of CStdioFile::ReadString  
* and adapted as appropriate.  
*/   
LPTSTR CStdioFile_UTF8::ReadString(LPTSTR lpsz, UINT nMax)   
{   
	ATLASSERT(lpsz != NULL);   
	ATLASSERT(m_pStream != NULL);   

	if (lpsz == NULL) {   
		ThrowInvalidArgException();   
	}   

	char *buf = new char[nMax];    
	BOOL read_failed = fgets(buf, nMax, m_pStream) == NULL;   
	if (read_failed && !feof(m_pStream)) {   
		clearerr(m_pStream);   
		AfxThrowFileException(QFileException::genericException, _doserrno, m_strFileName);   
	}   
	if (!read_failed) {   
#if defined(_UNICODE)      
		WCHAR *wbuf = new WCHAR[nMax];   
		MultiByteToWideChar(CP_UTF8, 0, buf, strlen(buf)+1, wbuf, nMax);   
		wcsncpy_s(lpsz,nMax, wbuf, nMax);   
		delete [] wbuf;   
#else   
		strncpy_s_s(lpsz, buf, nMax);   
#endif   
		delete [] buf;   
	}   

	return read_failed ? NULL : lpsz;   
}   

/**  
* Read an UTF8 encoded string from storage and convert it as appropriate.  
* The code is snipped from the MFC implementation of CStdioFile::ReadString  
* and adapted as appropriate.  
*/   
BOOL CStdioFile_UTF8::ReadString(CStdString& rString)   
{   
	rString = _T("");    // empty string without deallocating   
	const int nMaxSize = 128;   
	LPTSTR lpsz = rString.GetBuffer(nMaxSize);   
	LPTSTR lpszResult;   
	int nLen = 0;   
	for (;;)   
	{   
		// Required conversions are done here. If the MFC implementation had   
		// called ReadString here as well, instead of _fgetts directly, I   
		// wouldn't have to do this override at all.   
		lpszResult = ReadString(lpsz, nMaxSize+1);   
		rString.ReleaseBuffer();   

		// handle error/eof case   
		if (lpszResult == NULL && !feof(m_pStream))   
		{   
			clearerr(m_pStream);   
			AfxThrowFileException(QFileException::genericException, _doserrno,   
				m_strFileName);   
		}   

		// if string is read completely or EOF   
		if (lpszResult == NULL ||   
			(nLen = (int)lstrlen(lpsz)) < nMaxSize ||   
			lpsz[nLen-1] == '\n')   
			break;   

		nLen = rString.GetLength();   
		lpsz = rString.GetBuffer(nMaxSize + nLen) + nLen;   
	}   

	// remove '\n' from end of string if present   
	lpsz = rString.GetBuffer(0);   
	nLen = rString.GetLength();   
	if (nLen != 0 && lpsz[nLen-1] == '\n')   
		rString.GetBufferSetLength(nLen-1);   

	return nLen != 0;   
}   

/**  
* Write the given string in UTF8 format to storage.  
* The code is snipped from the MFC implementation of CStdioFile::WriteString  
* and adapted as appropriate.  
*/   
void CStdioFile_UTF8::WriteString(LPCTSTR lpsz)   
{   
	ATLASSERT(lpsz != NULL);   
	ATLASSERT(m_pStream != NULL);   

	if (lpsz == NULL) {   
		ThrowInvalidArgException();   
	}   

#if defined(_UNICODE)   
	int lpsz_len = lstrlen(lpsz);   
	int buf_len = WideCharToMultiByte(CP_UTF8, 0, lpsz, lpsz_len, 0, 0, 0, 0)+1;   
	char *buf = new char[buf_len+1];   
	WideCharToMultiByte(CP_UTF8, 0, lpsz, lpsz_len, buf, buf_len, 0, 0);   
	buf[buf_len] = 0;   
	if (fputs(buf, m_pStream) == EOF) {   
		AfxThrowFileException(QFileException::diskFull, _doserrno, m_strFileName);   
	}   
	delete [] buf;   
#else   
	if (fputs(lpsz, m_pStream) == EOF) {   
		AfxThrowFileException(QFileException::diskFull, _doserrno, m_strFileName);   
	}   
#endif   
}   

/**  
* Checks for a valid UTF8 Byte Order Mark at the beginning of the file.  
* Resets the file position os side effect.  
*/   
BOOL CStdioFile_UTF8::ReadBOM()   
{   
	const char utf8_bom[] = {'\xef', '\xbb', '\xbf'};   
	BOOL ok = FALSE;   

	SeekToBegin();   
	char buf[3];   
	if (Read(buf, 3) == 3) {   
		ok = strncmp(buf, utf8_bom, 3) == 0;   
	}   

	if (!ok) SeekToBegin(); // Reset to beginning if BOM not present   

	return ok;   
}   

/**  
* Writes an UTF8 Byte order Mark at the beginning of the file.  
* Resets the file position os side effect.  
*/   
void CStdioFile_UTF8::WriteBOM()   
{   
	const char utf8_bom[] = {'\xef', '\xbb', '\xbf'};   

	SeekToBegin();   
	Write(utf8_bom, sizeof(utf8_bom));   
}   

