#include "stdafx.h"
#include "QTime.h"
#include <ctime>
#include <math.h>
#include <atldef.h>

const int maxTimeBufferSize = 128;
const long maxDaysInSpan  =	3615897L;

BOOL WINAPI QTime::IsValidFILETIME(__in const FILETIME& fileTime) throw()
{
    FILETIME localTime;
    if (!FileTimeToLocalFileTime(&fileTime, &localTime))
    {
        return FALSE;
    }

    // then convert that time to system time
    SYSTEMTIME sysTime;
    if (!FileTimeToSystemTime(&localTime, &sysTime))
    {
        return FALSE;
    }

    return TRUE;
}

double QTime::GetTime() const throw()
{
    return m_dt;
}

CStdString QTime::Format(__in LPCTSTR pFormat) const
{
    // If null, return empty string
    if(GetStatus() == null)
        return _T("");

    // If invalid, return DateTime global string
    if(GetStatus() == invalid)
    {
        return L"Invalid DateTime";
    }

    UDATE ud;
    if (S_OK != VarUdateFromDate(m_dt, 0, &ud))
    {
        return L"Invalid DateTime";
    }

    struct tm tmTemp;
    tmTemp.tm_sec	= ud.st.wSecond;
    tmTemp.tm_min	= ud.st.wMinute;
    tmTemp.tm_hour	= ud.st.wHour;
    tmTemp.tm_mday	= ud.st.wDay;
    tmTemp.tm_mon	= ud.st.wMonth - 1;
    tmTemp.tm_year	= ud.st.wYear - 1900;
    tmTemp.tm_wday	= ud.st.wDayOfWeek;
    tmTemp.tm_yday	= ud.wDayOfYear - 1;
    tmTemp.tm_isdst	= 0;

    TCHAR szBuf[256];
    _tcsftime(szBuf, 255, pFormat, &tmTemp);
    return szBuf;
}

QTimeSpan::QTimeSpan() throw() : m_span(0), m_status(valid)
{
}

QTimeSpan::QTimeSpan(__in double dblSpanSrc) throw() :
    m_span(dblSpanSrc), m_status(valid)
{
    CheckRange();
}

QTimeSpan::QTimeSpan(
    __in LONG lDays,
    __in int nHours,
    __in int nMins,
    __in int nSecs) throw()
{
    SetDateTimeSpan(lDays, nHours, nMins, nSecs);
}

QTimeSpan::QTimeSpan( DWORD dwSeconds ) throw()
    :m_span( ((double)dwSeconds) / 86400.0f ), m_status(valid)
{

}

void QTimeSpan::SetStatus(__in DateTimeSpanStatus status) throw()
{
    m_status = status;
}

QTimeSpan::DateTimeSpanStatus QTimeSpan::GetStatus() const throw()
{
    return m_status;
}

__declspec(selectany) const double
QTimeSpan::OLE_DATETIME_HALFSECOND =
    1.0 / (2.0 * (60.0 * 60.0 * 24.0));

double QTimeSpan::GetTotalDays() const throw()
{
    ATLASSERT(GetStatus() == valid);
    return (double)LONGLONG(m_span + (m_span < 0 ?
                                      -OLE_DATETIME_HALFSECOND : OLE_DATETIME_HALFSECOND));
}

double QTimeSpan::GetTotalHours() const throw()
{
    ATLASSERT(GetStatus() == valid);
    return (double)LONGLONG((m_span + (m_span < 0 ?
                                       -OLE_DATETIME_HALFSECOND : OLE_DATETIME_HALFSECOND)) * 24);
}

double QTimeSpan::GetTotalMinutes() const throw()
{
    ATLASSERT(GetStatus() == valid);
    return (double)LONGLONG((m_span + (m_span < 0 ?
                                       -OLE_DATETIME_HALFSECOND : OLE_DATETIME_HALFSECOND)) * (24 * 60));
}

double QTimeSpan::GetTotalSeconds() const throw()
{
    ATLASSERT(GetStatus() == valid);
    return (double)LONGLONG((m_span + (m_span < 0 ?
                                       -OLE_DATETIME_HALFSECOND : OLE_DATETIME_HALFSECOND)) * (24 * 60 * 60));
}

LONG QTimeSpan::GetDays() const throw()
{
    ATLASSERT(GetStatus() == valid);
    return LONG(GetTotalDays());
}

LONG QTimeSpan::GetHours() const throw()
{
    return LONG(GetTotalHours()) % 24;
}

LONG QTimeSpan::GetMinutes() const throw()
{
    return LONG(GetTotalMinutes()) % 60;
}

LONG QTimeSpan::GetSeconds() const throw()
{
    return LONG(GetTotalSeconds()) % 60;
}

QTimeSpan& QTimeSpan::operator=(__in double dblSpanSrc) throw()
{
    m_span = dblSpanSrc;
    m_status = valid;
    CheckRange();
    return *this;
}

bool QTimeSpan::operator==(
    __in const QTimeSpan& dateSpan) const throw()
{
    if(GetStatus() == dateSpan.GetStatus())
    {
        if(GetStatus() == valid)
        {
            // it has to be in precision range to say that it as equal
            if (m_span + OLE_DATETIME_HALFSECOND > dateSpan.m_span &&
                    m_span - OLE_DATETIME_HALFSECOND < dateSpan.m_span)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        return (GetStatus() == null);
    }

    return false;
}

bool QTimeSpan::operator!=(
    __in const QTimeSpan& dateSpan) const throw()
{
    return !operator==(dateSpan);
}

bool QTimeSpan::operator<(
    __in const QTimeSpan& dateSpan) const throw()
{
    ATLASSERT(GetStatus() == valid);
    ATLASSERT(dateSpan.GetStatus() == valid);
    if( (GetStatus() == valid) && (GetStatus() == dateSpan.GetStatus()) )
        return m_span < dateSpan.m_span;

    return false;
}

bool QTimeSpan::operator>(
    __in const QTimeSpan& dateSpan) const throw()
{
    ATLASSERT(GetStatus() == valid);
    ATLASSERT(dateSpan.GetStatus() == valid);
    if( (GetStatus() == valid) && (GetStatus() == dateSpan.GetStatus()) )
        return m_span > dateSpan.m_span ;

    return false;
}

bool QTimeSpan::operator<=(
    __in const QTimeSpan& dateSpan) const throw()
{
    return operator<(dateSpan) || operator==(dateSpan);
}

bool QTimeSpan::operator>=(
    __in const QTimeSpan& dateSpan) const throw()
{
    return operator>(dateSpan) || operator==(dateSpan);
}

QTimeSpan QTimeSpan::operator+(
    __in const QTimeSpan& dateSpan) const throw()
{
    QTimeSpan dateSpanTemp;

    // If either operand Null, result Null
    if (GetStatus() == null || dateSpan.GetStatus() == null)
    {
        dateSpanTemp.SetStatus(null);
        return dateSpanTemp;
    }

    // If either operand Invalid, result Invalid
    if (GetStatus() == invalid || dateSpan.GetStatus() == invalid)
    {
        dateSpanTemp.SetStatus(invalid);
        return dateSpanTemp;
    }

    // Add spans and validate within legal range
    dateSpanTemp.m_span = m_span + dateSpan.m_span;
    dateSpanTemp.CheckRange();

    return dateSpanTemp;
}

QTimeSpan QTimeSpan::operator-(
    __in const QTimeSpan& dateSpan) const throw()
{
    QTimeSpan dateSpanTemp;

    // If either operand Null, result Null
    if (GetStatus() == null || dateSpan.GetStatus() == null)
    {
        dateSpanTemp.SetStatus(null);
        return dateSpanTemp;
    }

    // If either operand Invalid, result Invalid
    if (GetStatus() == invalid || dateSpan.GetStatus() == invalid)
    {
        dateSpanTemp.SetStatus(invalid);
        return dateSpanTemp;
    }

    // Subtract spans and validate within legal range
    dateSpanTemp.m_span = m_span - dateSpan.m_span;
    dateSpanTemp.CheckRange();

    return dateSpanTemp;
}

QTimeSpan& QTimeSpan::operator+=(
    __in const QTimeSpan dateSpan) throw()
{
    ATLASSERT(GetStatus() == valid);
    ATLASSERT(dateSpan.GetStatus() == valid);
    *this = *this + dateSpan;
    CheckRange();
    return *this;
}

QTimeSpan& QTimeSpan::operator-=(
    __in const QTimeSpan dateSpan) throw()
{
    ATLASSERT(GetStatus() == valid);
    ATLASSERT(dateSpan.GetStatus() == valid);
    *this = *this - dateSpan;
    CheckRange();
    return *this;
}

QTimeSpan QTimeSpan::operator-() const throw()
{
    return -this->m_span;
}

QTimeSpan::operator double() const throw()
{
    return m_span;
}

void QTimeSpan::SetDateTimeSpan( __in DWORD nSecs) throw()
{
    // Set date span by breaking into fractional days (all input ranges valid)
    m_span = (double)nSecs / 86400.0f;
    m_status = valid;
    CheckRange();
}

void QTimeSpan::SetDateTimeSpan(
    __in LONG lDays,
    __in int nHours,
    __in int nMins,
    __in int nSecs) throw()
{
    // Set date span by breaking into fractional days (all input ranges valid)
    m_span = lDays + ((double)nHours)/24 + ((double)nMins)/(24*60) +
             ((double)nSecs)/(24*60*60);
    m_status = valid;
    CheckRange();
}

void QTimeSpan::CheckRange()
{
    if(m_span < -maxDaysInSpan || m_span > maxDaysInSpan)
    {
        m_status = invalid;
    }
}

enum _CTIMESPANFORMATSTEP
{
    _CTFS_NONE   = 0,
    _CTFS_FORMAT = 1,
    _CTFS_NZ     = 2
};

#define _CTIMESPANFORMATS 3

CStdString QTimeSpan::Format(__in LPCTSTR pFormat) const
// formatting timespans is a little trickier than formatting CTimes
//  * we are only interested in relative time formats, ie. it is illegal
//      to format anything dealing with absolute time (i.e. years, months,
//         day of week, day of year, timezones, ...)
//  * the only valid formats:
//      %D - # of days
//      %H - hour in 24 hour format
//      %M - minute (0-59)
//      %S - seconds (0-59)
//      %% - percent sign
//	%#<any_of_mods> - skip leading zeros
{
    ATLASSERT( pFormat != NULL );
    if( pFormat == NULL )
        AtlThrow( E_INVALIDARG );

    CStdString strBuffer;
    CStdString hmsFormats [_CTIMESPANFORMATS] = {_T("%c"),_T("%02ld"),_T("%d")};
    CStdString dayFormats [_CTIMESPANFORMATS] = {_T("%c"),_T("%I64d"),_T("%I64d")};
//	strBuffer.GetBuffer(maxTimeBufferSize);
    TCHAR ch;
    while ((ch = *pFormat++) != _T('\0'))
    {
        enum _CTIMESPANFORMATSTEP formatstep = _CTFS_NONE;
        if(ch == _T('%'))
        {
            formatstep = _CTFS_FORMAT;
            ch = *pFormat++;
            if(ch == _T('#'))
            {
                formatstep = _CTFS_NZ;
                ch = *pFormat++;
            }
        }
        switch (ch)
        {
        case '%':
            strBuffer += ch;
            break;
        case 'D':
            strBuffer.AppendFormat(dayFormats[formatstep], formatstep ? GetDays()    : ch);
            break;
        case 'H':
            strBuffer.AppendFormat(hmsFormats[formatstep], formatstep ? GetHours()   : ch);
            break;
        case 'M':
            strBuffer.AppendFormat(hmsFormats[formatstep], formatstep ? GetMinutes() : ch);
            break;
        case 'S':
            strBuffer.AppendFormat(hmsFormats[formatstep], formatstep ? GetSeconds() : ch);
            break;
        default:
            if(formatstep)
            {
#pragma warning (push)
#pragma warning (disable: 4127)  // conditional expression constant
                ATLENSURE(FALSE);      // probably a bad format character
#pragma warning (pop)
            }
            else
            {
                strBuffer += ch;
#ifdef _MBCS
                if (_istlead(ch))
                {
                    strBuffer += *pFormat++;
                }
#endif
            }
            break;
        }
    }
//	strBuffer.ReleaseBuffer();
    return strBuffer;
}


/////////////////////////////////////////////////////////////////////////////
// COleDateTime
/////////////////////////////////////////////////////////////////////////////

QTime WINAPI QTime::GetCurrentTime() throw()
{
    return QTime(_time64(NULL));
}

QTime::QTime() throw() : m_status(valid)
{
    *this = GetCurrentTime();
}

QTime::QTime(__in const VARIANT& varSrc) throw() :
    m_dt( 0 ), m_status(valid)
{
    *this = varSrc;
}

QTime::QTime(__in DATE dtSrc) throw() :
    m_dt( dtSrc ), m_status(valid)
{
}

QTime::QTime(__in __time32_t timeSrc) throw() :
    m_dt( 0 ), m_status(valid)
{
    *this = timeSrc;
}

QTime::QTime(__in __time64_t timeSrc) throw() :
    m_dt( 0 ), m_status(valid)
{
    *this = timeSrc;
}

QTime::QTime(__in const SYSTEMTIME& systimeSrc) throw() :
    m_dt( 0 ), m_status(valid)
{
    *this = systimeSrc;
}

QTime::QTime(__in const FILETIME& filetimeSrc) throw() :
    m_dt( 0 ), m_status(valid)
{
    *this = filetimeSrc;
}

QTime::QTime(
    __in int nYear,
    __in int nMonth,
    __in int nDay,
    __in int nHour,
    __in int nMin,
    __in int nSec) throw()
{
    SetDateTime(nYear, nMonth, nDay, nHour, nMin, nSec);
}

QTime::QTime(
    __in WORD wDosDate,
    __in WORD wDosTime) throw()
{
    m_status = ::DosDateTimeToVariantTime(wDosDate, wDosTime, &m_dt) ?
               valid : invalid;
}

void QTime::SetStatus(__in DateTimeStatus status) throw()
{
    m_status = status;
}

QTime::DateTimeStatus QTime::GetStatus() const throw()
{
    return m_status;
}

bool QTime::GetAsSystemTime(_Out_ SYSTEMTIME& sysTime) const throw()
{
    return GetStatus() == valid && ::VariantTimeToSystemTime(m_dt, &sysTime);
}

bool QTime::GetAsUDATE(_Out_ UDATE &udate) const throw()
{
    return SUCCEEDED(::VarUdateFromDate(m_dt, 0, &udate));
}

int QTime::GetYear() const throw()
{
    SYSTEMTIME st;
    return GetAsSystemTime(st) ? st.wYear : error;
}

int QTime::GetMonth() const throw()
{
    SYSTEMTIME st;
    return GetAsSystemTime(st) ? st.wMonth : error;
}

int QTime::GetDay() const throw()
{
    SYSTEMTIME st;
    return GetAsSystemTime(st) ? st.wDay : error;
}

int QTime::GetHour() const throw()
{
    SYSTEMTIME st;
    return GetAsSystemTime(st) ? st.wHour : error;
}

int QTime::GetMinute() const throw()
{
    SYSTEMTIME st;
    return GetAsSystemTime(st) ? st.wMinute : error;
}

int QTime::GetSecond() const throw()
{
    SYSTEMTIME st;
    return GetAsSystemTime(st) ? st.wSecond : error;
}

int QTime::GetDayOfWeek() const throw()
{
    SYSTEMTIME st;
    return GetAsSystemTime(st) ? st.wDayOfWeek + 1 : error;
}

int QTime::GetDayOfYear() const throw()
{
    UDATE udate;
    return GetAsUDATE(udate) ? udate.wDayOfYear : error;
}

QTime& QTime::operator=(__in const VARIANT& varSrc) throw()
{
    if (varSrc.vt != VT_DATE)
    {
        VARIANT varDest;
        varDest.vt = VT_EMPTY;
        if(SUCCEEDED(::VariantChangeType(&varDest, const_cast<VARIANT *>(&varSrc), 0, VT_DATE)))
        {
            m_dt = varDest.date;
            m_status = valid;
        }
        else
            m_status = invalid;
    }
    else
    {
        m_dt = varSrc.date;
        m_status = valid;
    }

    return *this;
}

QTime& QTime::operator=(__in DATE dtSrc) throw()
{
    m_dt = dtSrc;
    m_status = valid;
    return *this;
}

QTime& QTime::operator=(__in const __time32_t& timeSrc) throw()
{
    return operator=(static_cast<__time64_t>(timeSrc));
}

QTime& QTime::operator=(__in const __time64_t& timeSrc)
{
    SYSTEMTIME st;
    struct tm ttm;

    if (_localtime64_s(&ttm, &timeSrc) != 0)
    {
        m_status = invalid;
        return *this;
    }

    st.wYear = (WORD) (1900 + ttm.tm_year);
    st.wMonth = (WORD) (1 + ttm.tm_mon);
    st.wDayOfWeek = (WORD) ttm.tm_wday;
    st.wDay = (WORD) ttm.tm_mday;
    st.wHour = (WORD) ttm.tm_hour;
    st.wMinute = (WORD) ttm.tm_min;
    st.wSecond = (WORD) ttm.tm_sec;
    st.wMilliseconds = 0;

    m_status = ConvertSystemTimeToVariantTime(st) ? valid : invalid;

    return *this;
}

QTime &QTime::operator=(__in const SYSTEMTIME &systimeSrc) throw()
{
    m_status = ConvertSystemTimeToVariantTime(systimeSrc) ?	valid : invalid;
    return *this;
}

QTime &QTime::operator=(__in const FILETIME &filetimeSrc) throw()
{
    FILETIME ftl;
    SYSTEMTIME st;

    m_status =  ::FileTimeToLocalFileTime(&filetimeSrc, &ftl) &&
                ::FileTimeToSystemTime(&ftl, &st) &&
                ConvertSystemTimeToVariantTime(st) ? valid : invalid;

    return *this;
}

BOOL QTime::ConvertSystemTimeToVariantTime(__in const SYSTEMTIME& systimeSrc)
{
    return AtlConvertSystemTimeToVariantTime(systimeSrc,&m_dt);
}
QTime &QTime::operator=(const UDATE &udate) throw()
{
    m_status = (S_OK == VarDateFromUdate((UDATE*)&udate, 0, &m_dt)) ? valid : invalid;

    return *this;
}

bool QTime::operator==(__in const QTime& date) const throw()
{
    if(GetStatus() == date.GetStatus())
    {
        if(GetStatus() == valid)
        {
            // it has to be in precision range to say that it as equal
            if (m_dt + QTimeSpan::OLE_DATETIME_HALFSECOND > date.m_dt &&
                    m_dt - QTimeSpan::OLE_DATETIME_HALFSECOND < date.m_dt)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        return (GetStatus() == null);
    }
    return false;

}

bool QTime::operator!=(__in const QTime& date) const throw()
{
    return !operator==(date);
}

bool QTime::operator<(__in const QTime& date) const throw()
{
    ATLASSERT(GetStatus() == valid);
    ATLASSERT(date.GetStatus() == valid);
    if( (GetStatus() == valid) && (GetStatus() == date.GetStatus()) )
        return( DoubleFromDate( m_dt ) < DoubleFromDate( date.m_dt ) );

    return false;
}

bool QTime::operator>(__in const QTime& date) const throw()
{
    ATLASSERT(GetStatus() == valid);
    ATLASSERT(date.GetStatus() == valid);
    if( (GetStatus() == valid) && (GetStatus() == date.GetStatus()) )
        return( DoubleFromDate( m_dt ) > DoubleFromDate( date.m_dt ) );

    return false;
}

bool QTime::operator<=(__in const QTime& date) const throw()
{
    return operator<(date) || operator==(date);
}

bool QTime::operator>=(__in const QTime& date) const throw()
{
    return operator>(date) || operator==(date);
}

QTime QTime::operator+(__in QTimeSpan dateSpan) const throw()
{
    ATLASSERT(GetStatus() == valid);
    ATLASSERT(dateSpan.GetStatus() == valid);
    return( QTime( DateFromDouble( DoubleFromDate( m_dt )+(double)dateSpan ) ) );
}

QTime QTime::operator-(__in QTimeSpan dateSpan) const throw()
{
    ATLASSERT(GetStatus() == valid);
    ATLASSERT(dateSpan.GetStatus() == valid);
    return( QTime( DateFromDouble( DoubleFromDate( m_dt )-(double)dateSpan ) ) );
}

QTime& QTime::operator+=(__in QTimeSpan dateSpan) throw()
{
    ATLASSERT(GetStatus() == valid);
    ATLASSERT(dateSpan.GetStatus() == valid);
    m_dt = DateFromDouble( DoubleFromDate( m_dt )+(double)dateSpan );
    return( *this );
}

QTime& QTime::operator-=(__in QTimeSpan dateSpan) throw()
{
    ATLASSERT(GetStatus() == valid);
    ATLASSERT(dateSpan.GetStatus() == valid);
    m_dt = DateFromDouble( DoubleFromDate( m_dt )-(double)dateSpan );
    return( *this );
}

QTimeSpan QTime::operator-(__in const QTime& date) const throw()
{
    ATLASSERT(GetStatus() == valid);
    ATLASSERT(date.GetStatus() == valid);
    return DoubleFromDate(m_dt) - DoubleFromDate(date.m_dt);
}

QTime::operator DATE() const throw()
{
    ATLASSERT(GetStatus() == valid);
    return( m_dt );
}

int QTime::SetDateTime(
    __in int nYear,
    __in int nMonth,
    __in int nDay,
    __in int nHour,
    __in int nMin,
    __in int nSec) throw()
{
    SYSTEMTIME st;
    ::ZeroMemory(&st, sizeof(SYSTEMTIME));

    st.wYear = WORD(nYear);
    st.wMonth = WORD(nMonth);
    st.wDay = WORD(nDay);
    st.wHour = WORD(nHour);
    st.wMinute = WORD(nMin);
    st.wSecond = WORD(nSec);

    m_status = ConvertSystemTimeToVariantTime(st) ? valid : invalid;
    return m_status;
}

int QTime::SetDateTime( const QTime& d,const QTime&t )
{
    return SetDateTime(d.GetYear(),d.GetMonth(),d.GetDay(),
                       t.GetHour(),t.GetMinute(),t.GetSecond());
}

int QTime::SetDate(__in int nYear, __in int nMonth, __in int nDay) throw()
{
    SYSTEMTIME st;
    if (GetAsSystemTime(st))
        return SetDateTime(nYear, nMonth, nDay,st.wHour,st.wMinute,st.wSecond);
    else
        return SetDateTime(nYear, nMonth, nDay,0,0,0);
}

int QTime::SetDate( const QTime& t ) throw()
{
    return SetDate(t.GetYear(),t.GetMonth(),t.GetDay());
}

int QTime::SetTime(__in int nHour, __in int nMin, __in int nSec) throw()
{
    // Set date to zero date - 12/30/1899
    SYSTEMTIME st;
    if (GetAsSystemTime(st))
        return SetDateTime(st.wYear, st.wMonth, st.wDay, nHour, nMin, nSec);
    else
        return SetDateTime(1899, 12, 30, nHour, nMin, nSec);
}

int QTime::SetTime( const QTime& t ) throw()
{
    return SetTime(t.GetHour(),t.GetMinute(),t.GetSecond());
}

BOOL QTime::IsLastDayOfMonth()const
{
    SYSTEMTIME st;
    if (!GetAsSystemTime(st))
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
    switch (st.wMonth)
    {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
    {
        return ( st.wDay == 31);
    }
    case 4:
    case 6:
    case 9:
    case 11:
    {
        return (st.wDay == 30);
    }
    case 2:
    {
        return IsLeapYear(st.wYear)?(29==st.wDay):(28==st.wDay);
    }
    default:
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
    }
}

double WINAPI QTime::DoubleFromDate(__in DATE date) throw()
{
    // We treat it as positive from -OLE_DATETIME_HALFSECOND because of numeric errors
    // If value is positive it doesn't need conversion
    if(date > -QTimeSpan::OLE_DATETIME_HALFSECOND)
    {
        return date;
    }

    // If negative, must convert since negative dates not continuous
    // (examples: -1.25 to -.75, -1.50 to -.50, -1.75 to -.25)
    double fTemp = ceil(date);

    return fTemp - (date - fTemp);
}

DATE WINAPI QTime::DateFromDouble(__in double f) throw()
{
    // We treat it as positive from -OLE_DATETIME_HALFSECOND because of numeric errors
    // If value is positive it doesn't need conversion
    if(f > -QTimeSpan::OLE_DATETIME_HALFSECOND )
    {
        return f;
    }

    // If negative, must convert since negative dates not continuous
    // (examples: -.75 to -1.25, -.50 to -1.50, -.25 to -1.75)
    double fTemp = floor(f); // fTemp is now whole part

    return fTemp + (fTemp - f);
}

void QTime::CheckRange()
{
    // About year 100 to about 9999
    if(m_dt > VTDATEGRE_MAX || m_dt < VTDATEGRE_MIN)
    {
        SetStatus(invalid);
    }
}

bool QTime::ParseDateTime( __in LPCTSTR lpszDate,
                           __in DWORD dwFlags /*= 0*/, __in LCID lcid /*= LANG_USER_DEFAULT*/ ) throw()
{
    USES_CONVERSION_EX;
    LPCTSTR pszDate = ( lpszDate == NULL ) ? _T("") : lpszDate;

    HRESULT hr;
    LPOLESTR p = T2OLE_EX((LPTSTR)pszDate, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
    if( p == NULL )
    {
        m_dt = 0;
        m_status = invalid;
        return false;
    }
#endif // _UNICODE

    if (FAILED(hr = VarDateFromStr( p, lcid, dwFlags, &m_dt )))
    {
        if (hr == DISP_E_TYPEMISMATCH)
        {
            // Can't convert string to date, set 0 and invalidate
            m_dt = 0;
            m_status = invalid;
            return false;
        }
        else if (hr == DISP_E_OVERFLOW)
        {
            // Can't convert string to date, set -1 and invalidate
            m_dt = -1;
            m_status = invalid;
            return false;
        }
        else
        {
            ATLTRACE(atlTraceTime, 0, _T("\nCOleDateTime VarDateFromStr call failed.\n\t"));
            // Can't convert string to date, set -1 and invalidate
            m_dt = -1;
            m_status = invalid;
            return false;
        }
    }

    m_status = valid;
    return true;
//
// 	if (NULL == lpszTime)
// 		return false;
// 	if (SUCCEEDED(VarDateFromStr( LPOLESTR(lpszTime), dwFlags, VAR_CALENDAR_HIJRI , &m_dt )))
// 	{
// 		m_status = valid;
// 		return true;
// 	}
// 	return false;
}

int QTime::CompareDate( const QTime &date ) const throw()
{
    ATLASSERT(GetStatus() == valid);
    ATLASSERT(date.GetStatus() == valid);
    if( (GetStatus() == valid) && (GetStatus() == date.GetStatus()) )
    {
        // 只要比较其小数点之前的部分就可以了
        double d = floor(m_dt) - floor(date.m_dt);
        if (d > 0) return 1;
        else if (0 == d) return 0;
        else  return -1;
    }
    return 0;
}

int QTime::CompareTime( const QTime& date ) const throw()
{
    ATLASSERT(GetStatus() == valid);
    ATLASSERT(date.GetStatus() == valid);
    if( (GetStatus() == valid) && (GetStatus() == date.GetStatus()) )
    {
        // 只要比较其小数点之后的部分就可以了
        double d = (m_dt - floor(m_dt)) - (date.m_dt - floor(date.m_dt));
        if (d > 1e-8) return 1;
        else if (0 == d) return 0;
        else  return -1;
    }
    return 0;
}

QTime QTime::ParseDate( DWORD dwDate,const QTime&tTime /*= GetCurrentTime() */ )
{
    SYSTEMTIME st;
    tTime.GetAsSystemTime(st);
    st.wDay = dwDate & 0xFF;
    st.wMonth = (dwDate>>8) & 0xFF;
    st.wYear = (dwDate>>16);
    st.wDayOfWeek = 0;
    return QTime (st);
}

DWORD QTime::MakeDate()const
{
    SYSTEMTIME st;
    GetAsSystemTime(st);
    DWORD dwDate = st.wYear;
    dwDate <<= 8;
    dwDate |= st.wMonth;
    dwDate <<= 8;
    dwDate |= st.wDay;
    return dwDate;
}

QTime QTime::ParseTime( DWORD dwTime ,const QTime&tDate /*= GetCurrentTime()*/ )
{
    SYSTEMTIME st;
    tDate.GetAsSystemTime(st);
    st.wHour = ((dwTime>>24) & 0x1F);
    st.wMinute = ((dwTime>>16) & 0x3F);
    st.wSecond = ((dwTime>>10) & 0x3F); // 秒占用6位
    st.wMilliseconds = (dwTime & 0x3FF);
    st.wDayOfWeek = 0;
    return QTime (st);
}

DWORD QTime::MakeTime() const
{
    SYSTEMTIME st;
    GetAsSystemTime(st);
    DWORD dwTime = (st.wHour & 0x1F);
    dwTime <<= 8;
    dwTime |= (st.wMinute & 0x3F);
    dwTime <<= 8;
    dwTime |= (((st.wSecond & 0x3F) << 2));
    dwTime <<= 8;
    dwTime |= st.wMilliseconds;
    return dwTime;
}

QTime QTime::CombineTime(DWORD dwDate,DWORD dwTime)
{
    SYSTEMTIME st;
    st.wYear = (dwDate>>16) & 0xFFFF;
    st.wMonth = (dwDate>>8) & 0xFF;
    st.wDay = dwDate & 0xFF;
    st.wHour = ((dwTime>>24) & 0x1F);
    st.wMinute = ((dwTime>>16) & 0x3F);
    st.wSecond = ((dwTime>>10) & 0x3F); // 秒占用6位
    st.wMilliseconds = (dwTime & 0x3FF);
    st.wDayOfWeek = 0;

    return QTime(st);
}

QTime QTime::IgnoreMillSecond()
{
    SYSTEMTIME st;
    if ((GetStatus() == invalid) && GetAsSystemTime(st))
    {
        st.wMilliseconds = 0;
        m_status = ConvertSystemTimeToVariantTime(st) ? valid : invalid;
    }
    return *this;
}

