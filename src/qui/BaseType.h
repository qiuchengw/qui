#ifndef _QUI_BASETYPE_
#define _QUI_BASETYPE_

#pragma once

//////////////////////////////////////////////////////////////////////////
// 过期函数
#pragma warning(disable:4995)
// 不安全函数
#pragma warning(disable:4996)

#include <Windows.h>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

#ifdef _WIN32_WINNT
#	undef _WIN32_WINNT
#	define _WIN32_WINNT 0x0500
#endif

#ifndef _WTL_NO_CSTRING
#   define _WTL_NO_CSTRING
#endif

#ifdef _WTL_USE_CSTRING
#   undef _WTL_USE_CSTRING
#endif

//////////////////////////////////////////////////////////////////////////
// 转换相对路径到绝对路径
//      转换路径基于使用此宏的文件
// eg:
//  E:\dir\a.h 中使用此宏
//      _CONVERT_2_ABSPATH(lib/test.lib)
//  宏展开后为：
//      E:\dir\a.h/../lib/test.lib  == E:/dir/lib/test.lib
#ifndef _CONVERT_2_ABSPATH
    #define _CONVERT_2_ABSPATH(f) __FILE__##"/../"#f
#endif

// 链接到lib文件（lib文件路径为使用此宏的文件的相对路径）
//  eg:
//      #pragma _LINK_LIBFILE(lib/test.lib)
#ifndef _LINK_LIBFILE
    #define _LINK_LIBFILE(f) comment(lib, _CONVERT_2_ABSPATH(f))
#endif 

//////////////////////////////////////////////////////////////////////////

// #include <Windows.h>
// #include <WTypes.h>

// 启用HTMLayout支持
// #include "./ui/htmlayout/htmlayout.h"
// using namespace htmlayout;
// using namespace htmlayout::dom;

// 字符串支持
// #include <shlwapi.h>
// #pragma comment(lib,"shlwapi.lib")

// 不使用WTL中的字符串类，此字符串类是独立的
// 继承自std::stringt,可以和std的字符串无缝转换
#ifndef _ATL_TMP_NO_CSTRING
#   define _ATL_TMP_NO_CSTRING
#endif
#include "stdstring.h"

typedef std::vector<CStdString> StringArray;

// 只是用高级字符串转换类，如ATL::CA2WEX<123>(...)
#ifndef _ATL_EX_CONVERSION_MACROS_ONLY
#   define _ATL_EX_CONVERSION_MACROS_ONLY
#endif

// WTL 支持
// #include <atlbase.h>
// #include <atlapp.h>
// #include <atlcrack.h>
// #include <atlmisc.h>
// #include <atldef.h>
// 
// using namespace WTL;

// 启用下面的宏定义以支持flash
// #define UIBASE_SUPPORT_FLASH    1

// 启用下面的宏定义以支持web浏览器
// #define UIBASE_SUPPORT_WEBBROWSER   1

//////////////////////////////////////////////////////////////////////////
// 单实例模式宏
#define SINGLETON_ON_DESTRUCTOR(clsname) \
    public: static clsname* GetInstance(){ static clsname _instance;return &_instance; }\
    private:~clsname()

//////////////////////////////////////////////////////////////////////////
#ifndef POINT_FROM_LPARAM
#	define POINT_FROM_LPARAM(p,l) do{p.x=GET_X_LPARAM(l),p.y=GET_Y_LPARAM(l);} while(0)
#endif

#ifndef _HasFlag
#	define _HasFlag(l,f) ((l) & (f))
#endif

#ifndef _AddFlag
#	define _AddFlag(l,f) ((l) |= (f))
#endif

#ifndef _RemoveFlag
#	define _RemoveFlag(l,f) ((l) &= ~(f))
#endif

//////////////////////////////////////////////////////////////////////////
// message relations
// 用户自定义消息
#ifndef DECLARE_USER_MESSAGE
#	define DECLARE_USER_MESSAGE(name) \
    const UINT name=::RegisterWindowMessage(L#name);
#endif

//////////////////////////////////////////////////////////////////////////
// 内存泄漏检测

#ifdef _DEBUG
    #define _CRTDBG_MAP_ALLOC 
    #define _CRTDBG_MAP_ALLOC_NEW
    #include<stdlib.h> 
    #include<crtdbg.h>

    #define _QUIMemCheckPoint(s) \
        _CrtMemState __ui_##s; \
        _CrtMemCheckpoint(&(__ui_##s));

    #define _QUIIsMemLeak(s1,s2) \
         { \
             _CrtMemState __ui_s3;\
            if ( _CrtMemDifference(&__ui_s3, &(__ui_##s1),&(__ui_##s2)) )\
            {\
                TRACE(L"------------内存泄漏---------------\n");\
                _CrtMemDumpStatistics(&__ui_s3);\
                TRACE(L"-----------------------------------\n");\
                _ASSERT_EXPR(FALSE,L"发生内存泄漏！请检查!");\
             }\
         }
#else
    #define _QUIMemCheckPoint(s) 
    #define _QUIIsMemLeak(s1,s2)
#endif

#endif
