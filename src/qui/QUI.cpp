// // // // #include "QUI.h"
#include "stdafx.h"
#include "ui/QApp.h"

#if 0
#include "AppHelper.h"

#ifdef _DEBUG
LONG WINAPI Q_UnhandledException(__in  struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	QString sExcp = L"未处理异常";

	::MessageBox(NULL,sExcp,L"未处理异常",MB_OK);
	
	ExitProcess(-1);

	return 0;
}

#else   // release 发布版

#include "breakpad/breakpad_inc.h"

/**
 *  使用google_break来dump异常的数据，以供调试
**/

class __QExceptionHandler__
{
    ~__QExceptionHandler__()
    {
        if (NULL != m_pEH)
        {
            // 运行到这儿的话，程序应该是没啥异常啦
            delete m_pEH;       
        }
    }

public:
    __QExceptionHandler__()
    {
        // 在本程序的运行目录下生成"__crash_dumps"文件夹，
        // 运行异常dump出的文件将会写入到此文件夹下面
        QString sDumpDir = quibase::GetModulePath() + L"__crash_dumps\\";
        sDumpDir.Replace(L'/',L'\\');
        quibase::MakeSureDirExist(sDumpDir);
        _CrtSetReportMode(_CRT_ASSERT, 0);
        m_pEH = new google_breakpad::ExceptionHandler(sDumpDir,NULL,
                    &__QExceptionHandler__::AfterFileDumped, this,       // 文件dump后的调用
                    google_breakpad::ExceptionHandler::HANDLER_ALL);    // 处理所有异常
    }

    static BOOL PreotectProcess()
    {
        static __QExceptionHandler__ _inst;

        return nullptr != _inst.m_pEH;
    }

protected:
    static bool AfterFileDumped(const wchar_t* dump_path, const wchar_t* minidump_id,
        void* context, EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion,
        bool succeeded) 
    {
        if ( succeeded )
        {   // dump 完成 
            wchar_t szDmpFile[512] = {0};   // dump 文件的全路径
            swprintf_s(szDmpFile, 512, L"%s%s.dmp", dump_path, minidump_id);
            // 可以启动新进程处理该文件（如发送到服务器）
        }

        return succeeded;
    }

private:
    google_breakpad::ExceptionHandler*  m_pEH;
};

#endif

#endif

//#define _TEST_QUIBASE_WITHOURT_MAINPROC
// 定义_TEST_QUIBASE_WITHOURT_MAINPROC可以自己生成入口点函数
#ifndef _TEST_QUIBASE_WITHOURT_MAINPROC

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
#ifdef _DEBUG
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );	// 警告信息输出到调试器的输出窗口
    //	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );	// 输出到控制台
    _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_WNDW );	// 不可恢复的错误信息，显示中断窗口
    //	_CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_WNDW );	// 断言为false，显示中断窗口
    //	_CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

//     _CrtSetBreakAlloc(9338);
//     _CrtSetBreakAlloc(1500);
//    _CrtSetBreakAlloc(15561);

    //SetUnhandledExceptionFilter(Q_UnhandledException);
#else   // release版本
    // 监控程序的所有异常
#if 0
    if (!__QExceptionHandler__::PreotectProcess())
    {
        // shit!
        // but....life is still go on!
    }
#endif
#endif

    // 初始化COM，如果是非ActiveX宿主，可以注释掉
    // If you are running on NT 4.0 or higher you can use the following call instead to 
    // make the EXE free threaded. This means that calls come in on a random RPC thread.
    //  HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLVERIFY(SUCCEEDED(::OleInitialize(NULL)));

    int nRet = QUIRun(hInstance);

    // 如果是非ActiveX宿主，可以注释掉
	::CoUninitialize();

	return nRet;
}

#endif
