#ifndef exarray_h__
#define exarray_h__

#pragma once

#include <vector>

template<typename T>
class ExArray : public std::vector<T>
{
public:
    inline bool contain(const T&t ) const
    { 
        return std::find(begin(), end(), t) != end();
    }

    /*
     * 在[ib,ie]索引范围内查找t
     * ie=-1的时候，范围为 [ib,size);
     *
     */
    bool contain_within_index(const T&t ,int ib, int ie=-1)const
    {
        if (ib < 0) 
            ib = 0;
        
        size_t c = size();
        if (ib >= c) 
            return false;
        
        if (-1 == ie ) 
            ie = c - 1;

        for (; ib <= ie; ib++)
        {
            if (at(ib) == t)
            {
                return true;
            }
        }
        return false;
    }

    /*
     * 在索引范围[ib,ie]内查找第一个大于等于t的数值
     *
     * 返回其所在索引，-1 为未找到
     *
     */
    int find_first_lgoreq(const T &t , int ib=0,int ie=-1) const
    {
        if (ib < 0) 
            ib = 0;
        
        int c = size();
        if (ib >= c) 
            return -1;
        
        if (-1 == ie ) 
            ie = c - 1;
        
        for (; ib <= ie; ib++)
        {
            if (at(ib) >= t)
            {
                return ib;
            }
        }
        return -1;
    }
};

#endif // apphelper_h__
