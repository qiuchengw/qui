#pragma once

// // #include "QUI.h"
#include "stdstring.h"

class QTimeSpan
{
	// Constructors
public:
	QTimeSpan() throw();
    QTimeSpan(DWORD dwSeconds) throw();
	QTimeSpan(__in double dblSpanSrc) throw();
	QTimeSpan(
		__in LONG lDays, 
		__in int nHours, 
		__in int nMins, 
		__in int nSecs) throw();

	// Attributes
	enum DateTimeSpanStatus
	{
		valid = 0,
		invalid = 1,    // Invalid span (out of range, etc.)
		null = 2,       // Literally has no value
	};

	double m_span;
	DateTimeSpanStatus m_status;

	void SetStatus(__in DateTimeSpanStatus status) throw();
	DateTimeSpanStatus GetStatus() const throw();

	double GetTotalDays() const throw();    // span in days (about -3.65e6 to 3.65e6)
	double GetTotalHours() const throw();   // span in hours (about -8.77e7 to 8.77e6)
	double GetTotalMinutes() const throw(); // span in minutes (about -5.26e9 to 5.26e9)
	double GetTotalSeconds() const throw(); // span in seconds (about -3.16e11 to 3.16e11)

	LONG GetDays() const throw();       // component days in span
	LONG GetHours() const throw();      // component hours in span (-23 to 23)
	LONG GetMinutes() const throw();    // component minutes in span (-59 to 59)
	LONG GetSeconds() const throw();    // component seconds in span (-59 to 59)

	// Operations
	QTimeSpan& operator=(__in double dblSpanSrc) throw();

	bool operator==(__in const QTimeSpan& dateSpan) const throw();
	bool operator!=(__in const QTimeSpan& dateSpan) const throw();
	bool operator<(__in const QTimeSpan& dateSpan) const throw();
	bool operator>(__in const QTimeSpan& dateSpan) const throw();
	bool operator<=(__in const QTimeSpan& dateSpan) const throw();
	bool operator>=(__in const QTimeSpan& dateSpan) const throw();

	// DateTimeSpan math
	QTimeSpan operator+(__in const QTimeSpan& dateSpan) const throw();
	QTimeSpan operator-(__in const QTimeSpan& dateSpan) const throw();
	QTimeSpan& operator+=(__in const QTimeSpan dateSpan) throw();
	QTimeSpan& operator-=(__in const QTimeSpan dateSpan) throw();
	QTimeSpan operator-() const throw();

	operator double() const throw();

	void SetDateTimeSpan(
		__in LONG lDays, 
		__in int nHours, 
		__in int nMins, 
		__in int nSecs) throw();
	void SetDateTimeSpan( __in DWORD nSecs) throw();
	// formatting
	CStdString Format(__in LPCTSTR pFormat) const;

	// Implementation
	void CheckRange();

	static const double OLE_DATETIME_HALFSECOND;
};

class QTime
{
	// Constructors
public:
	static QTime WINAPI GetCurrentTime() throw();
	static inline BOOL IsLeapYear(int year) 
	{ return (((year%100!=0)&&(year%4==0))||(year%400==0));}

	BOOL IsLastDayOfMonth()const;

	// 日期 DWORD 最低8位为日，次低8位为月，最高16位为年
	// 时间 DWORD 最低10位为毫秒，次低6位为秒，次高8位为分,最高8位为时
	static QTime ParseDate( DWORD dwDate,const QTime&tTime = QTime::GetCurrentTime() );
	DWORD MakeDate()const;
	static QTime ParseTime( DWORD wTime ,const QTime&tDate = QTime::GetCurrentTime()) ;
	DWORD MakeTime() const;
	static QTime CombineTime(DWORD dwDate,DWORD wTime);

	QTime IgnoreMillSecond();

	QTime() throw();

	QTime(__in const VARIANT& varSrc) throw();
	QTime(__in DATE dtSrc) throw();

	QTime(__in __time32_t timeSrc) throw();
	QTime(__in __time64_t timeSrc);

	QTime(__in const SYSTEMTIME& systimeSrc) throw();
	QTime(__in const FILETIME& filetimeSrc) throw();

	QTime(
		__in int nYear, 
		__in int nMonth, 
		__in int nDay,
		__in int nHour, 
		__in int nMin, 
		__in int nSec) throw();
	QTime(__in WORD wDosDate, __in WORD wDosTime) throw();

	// Attributes
	enum DateTimeStatus
	{
		error = -1,
		valid = 0,
		invalid = 1,    // Invalid date (out of range, etc.)
		null = 2,       // Literally has no value
	};

    BOOL IsValid()const
    {
        return m_status == valid;
    }

	DATE m_dt;
	DateTimeStatus m_status;

	void SetStatus(__in DateTimeStatus status) throw();
	DateTimeStatus GetStatus() const throw();

	bool GetAsSystemTime(_Out_ SYSTEMTIME& sysTime) const throw();
	bool GetAsUDATE(_Out_ UDATE& udate) const throw();

	int GetYear() const throw();
	// Month of year (1 = January)
	int GetMonth() const throw();
	// Day of month (1-31)
	int GetDay() const throw();
	// Hour in day (0-23)
	int GetHour() const throw();
	// Minute in hour (0-59)
	int GetMinute() const throw();
	// Second in minute (0-59)
	int GetSecond() const throw();
	// Day of week (1 = Sunday, 2 = Monday, ..., 7 = Saturday)
	int GetDayOfWeek() const throw();
	// Days since start of year (1 = January 1)
	int GetDayOfYear() const throw();

	// Operations
	QTime& operator=(__in const VARIANT& varSrc) throw();
	QTime& operator=(__in DATE dtSrc) throw();

	QTime& operator=(__in const __time32_t& timeSrc) throw();
	QTime& operator=(__in const __time64_t& timeSrc) throw();

	QTime& operator=(__in const SYSTEMTIME& systimeSrc) throw();
	QTime& operator=(__in const FILETIME& filetimeSrc) throw();
	QTime& operator=(__in const UDATE& udate) throw();

	bool operator==(__in const QTime& date) const throw();
	bool operator!=(__in const QTime& date) const throw();
	bool operator<(__in const QTime& date) const throw();
	bool operator>(__in const QTime& date) const throw();
	bool operator<=(__in const QTime& date) const throw();
	bool operator>=(__in const QTime& date) const throw();
	// >0  >
	// = 0 =
	// <0 <
	int CompareDate(const QTime &date) const throw();
	int CompareTime(const QTime& date)const throw();
	// DateTime math
	QTime operator+(__in QTimeSpan dateSpan) const throw();
	QTime operator-(__in QTimeSpan dateSpan) const throw();
	QTime& operator+=(__in QTimeSpan dateSpan) throw();
	QTime& operator-=(__in QTimeSpan dateSpan) throw();

	// DateTimeSpan math
	QTimeSpan operator-(__in const QTime& date) const throw();

	operator DATE() const throw();

	int SetDateTime(const QTime& d,const QTime&t);
	int SetDateTime(
		__in int nYear, 
		__in int nMonth, 
		__in int nDay,
		__in int nHour, 
		__in int nMin, 
		__in int nSec) throw();
	int SetDate(__in int nYear, __in int nMonth, __in int nDay) throw();
	int SetDate(const QTime& t) throw();
	int SetTime(__in int nHour, __in int nMin, __in int nSec) throw();
	int SetTime(const QTime& t) throw();
	bool ParseDateTime(
		__in LPCTSTR lpszDate, 
		__in DWORD dwFlags = 0,
		__in LCID lcid = LANG_USER_DEFAULT) throw();

	// formatting
	CStdString Format(__in LPCTSTR lpszFormat) const;
	double SQLDateTime() const { return m_dt; }
	static double SQLDateTimeCurrent(){ return QTime::GetCurrentTime().SQLDateTime(); }
	static BOOL WINAPI IsValidFILETIME(__in const FILETIME& fileTime) throw();
	double GetTime() const throw();

protected:
	static double WINAPI DoubleFromDate(__in DATE date) throw();
	static DATE WINAPI DateFromDouble(__in double f) throw();

	void CheckRange();	
	BOOL ConvertSystemTimeToVariantTime(__in const SYSTEMTIME& systimeSrc);
};
