#ifndef qstrex_h__
#define qstrex_h__

#pragma once

#include <sstream>
#include <Shlwapi.h>

class QStrEx
{
public:
    QStrEx(void)
    {
        m_cB = m_cE = L'\0';
    }

    QStrEx(wchar_t cSep1,wchar_t cSep2,LPCTSTR pszStr)
    {
        SetSeperator(cSep1,cSep2);
        SetString(pszStr);
    }

    QStrEx(wchar_t cSep1,LPCTSTR pszStr)
    {
        m_cE = L'\0';
        SetSeperator(cSep1);
        SetString(pszStr);
    }

    ~QStrEx(void)
    {

    }

public:
    void SetSeperator(wchar_t cB,wchar_t cE) 
    { 
        m_cB = cB;
        m_cE = cE;
    }

    void SetSeperator(wchar_t c) 
    {
        m_cB = c; 
        m_cE = L'\0';
    }

    void SetString(LPCTSTR szStr)
    {
        ASSERT((m_cB != L'\0'));
        vstr_.clear();
        ParseStr(szStr);
    }

    void AddString(LPCTSTR szStr)
    {
        ASSERT((m_cB != L'\0'));

        ParseStr(szStr);
    }

    void RemoveItem(__in LPCWSTR szStr, __in BOOL bIgnoreCase = TRUE)
    {
        if (bIgnoreCase)
        {
            std::remove_if(vstr_.begin(), vstr_.end(), 
                [&szStr](const CStdString& s)->bool { return !s.CompareNoCase(szStr); });
        }
        else
        {
            std::remove(vstr_.begin(), vstr_.end(), szStr);
        }
    }

    void Unique(BOOL bIgnoreCase = TRUE)
    {
        auto itr = vstr_.begin();
        int nSize = vstr_.size();
        if (bIgnoreCase)
        {
            for (int i = 0; i < nSize; i++)
            {
                for (int j = i + 1; j < nSize; )
                {
                    if (vstr_[j] == vstr_[i])
                    {
                        itr = vstr_.begin();
                        std::advance(itr,j);
                        vstr_.erase(itr);
                        nSize--;
                    }
                    else
                    {
                        j++;
                    }
                }
            }
        }
        else
        {
            for (int i = 0; i < nSize; i++)
            {
                for (int j = i + 1; j < nSize; )
                {
                    if (vstr_[j].CompareNoCase(vstr_[i]) == 0)
                    {
                        itr = vstr_.begin();
                        std::advance(itr,j);
                        vstr_.erase(itr);
                        nSize--;
                    }
                    else
                    {
                        j++;
                    }
                }
            }
        }
    }

    inline void GetAll(std::vector<CStdString> &sV)
    {
        sV = vstr_;
    }

    void GetAll(std::vector<int>& vi)
    {
        int n = vstr_.size();
        for (int i = 0; i < n; i++)
        {
            vi.push_back(StrToInt(vstr_[i]));
        }
    }

    inline int GetCount()const
    {
        return vstr_.size();
    }

    CStdString operator[](int i) 
    { 
        return vstr_[i]; 
    };

    CStdString Commbine()
    {
        std::wostringstream os;
        for (auto i = vstr_.begin(); i != vstr_.end(); ++i)
        {
            if (IsHasEndSep())
            {
                os<<m_cB<<(LPCWSTR)(*i)<<m_cE;
            }
            else
            {
                os<<(LPCWSTR)(*i)<<m_cB;
            }
        }
        return os.str();
    }

    BOOL IsExist(LPCWSTR szStr, BOOL bIgnoreCase = FALSE)const
    {
        for (auto i = vstr_.begin(); i != vstr_.end(); ++i)
        {
            if (bIgnoreCase)
            {
                if ((*i).CompareNoCase(szStr) == 0)
                {
                    return TRUE;
                }
            }
            else
            {
                if ((*i).Compare(szStr) == 0)
                {
                    return TRUE;
                }
            }
        }
        return FALSE;
    }

    void Clear()
    {
        vstr_.clear();
    }

protected:
    inline BOOL IsHasEndSep()const
    {
        return m_cE != L'\0';
    }

    void ParseStr(LPCTSTR szStr)
    {
        if (!szStr)
            return;
        CStdString sX = szStr;

        int iB = 0,iE;
        if (!IsHasEndSep())
        {
            while ( (iB = sX.Find(m_cB)) != -1)
            {
                vstr_.push_back(sX.Left(iB));
                sX = sX.Mid(iB + 1);
            }
            if (!sX.IsEmpty())
            {
                vstr_.push_back(sX);
            }
        }
        else
        {
            while ( (iB = sX.Find(m_cB,iB)) != -1)
            {
                iE = sX.Find(m_cE,iB);
                if (iE != -1)
                {
                    vstr_.push_back(sX.Mid(iB+1,iE-iB));
                    iB = iE + 1;
                }
            }
        }
    }

private:
    wchar_t			m_cB;
    wchar_t           m_cE;
    std::vector<CStdString>		vstr_;
};

#endif // apphelper_h__
