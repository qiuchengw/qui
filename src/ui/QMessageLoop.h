#pragma once

#include <atlapp.h>

// 适用于Htmlayout的消息循环
// 使用此消息循环可以保证htmlayout::queue ,gui_task 机制正常运作
//    添加了手动退出消息循环的代码
class QMessageLoop : public CMessageLoop
{
protected:
    BOOL m_bContinueRun;

public:
    QMessageLoop() : m_bContinueRun(TRUE) { }

    // message loop
    int Run()
    {
        BOOL bDoIdle = TRUE;
        int nIdleCount = 0;
        BOOL bRet;

        while (m_bContinueRun)
        {
            while(m_bContinueRun && bDoIdle 
                && !::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE))
            {
                if(!OnIdle(nIdleCount++))
                    bDoIdle = FALSE;
            }

            bRet = ::GetMessage(&m_msg, NULL, 0, 0);

            if(bRet == -1)
                continue;   // error, don't process
            else if(!bRet || !m_bContinueRun)
                break;   // WM_QUIT, exit message loop
            
            // 执行gui_task
            htmlayout::queue_hl::execute();

            if(!PreTranslateMessage(&m_msg))
            {
                ::TranslateMessage(&m_msg);
                ::DispatchMessage(&m_msg);
            }

            if(IsIdleMessage(&m_msg))
            {
                bDoIdle = TRUE;
                nIdleCount = 0;
            }
        }
        // 以待下次正常使用
        m_bContinueRun = TRUE;
        // 返回值
        return (int)m_msg.wParam;
    }

    BOOL IsRunning()
    {
        return m_bContinueRun;
    }

    void EndRun()
    {
        m_bContinueRun = FALSE;
    }

    // Overrideables
    // Override to change message filtering
    virtual BOOL PreTranslateMessage(MSG* pMsg)
    {
        return CMessageLoop::PreTranslateMessage(pMsg);
    }

    // override to change idle processing
    virtual BOOL OnIdle(int nIdleCount)
    {
        return CMessageLoop::OnIdle(nIdleCount);
    }
};
