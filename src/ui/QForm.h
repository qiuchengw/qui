#ifndef QForm_h__
#define QForm_h__

#pragma once

#include "QUIEvent.h"

/**
 *  QView的Client分割部分
**/
class QForm : public QUIEventTarget
{
public:
    QForm(HELEMENT heRoot = NULL);
    
    virtual ~QForm() throw()
    {
        Detach();
    }
    
    /** Attach到View的一个元素上
     *	return:
     *      TRUE        成功
     *	params:
     *		-[in]
     *          heRoot  DOM
    **/
    BOOL Attach(__in HELEMENT heRoot);

    virtual void OnAttach()
    {

    }
    
    void Detach()
    {
        OnDetach();

        __DeinitReflectEventEntry();
        __DeinitEventEntry();

        m_heRoot = NULL;
    }

    virtual void OnDetach()
    {

    }

    ECtrl GetRoot()const;

    HWND GetSafeHwnd()const
    {
        HWND hWnd = NULL;
        HTMLayoutGetElementHwnd(m_heRoot, &hWnd, TRUE);
        return hWnd;
    }

    virtual BOOL handle_event(HELEMENT,BEHAVIOR_EVENT_PARAMS&)
    {
        return FALSE;
    }

    virtual BOOL handle_mouse(HELEMENT,MOUSE_PARAMS&)
    {
        return FALSE;
    }

    virtual BOOL handle_key(HELEMENT,KEY_PARAMS&)
    {
        return FALSE;
    }

protected:
    // 在事件处理器中调用此函数可以将正在处理的事件反射到上层去
    inline void ReflectThisEvent()const
    {
        // 赋值为1024
        InterlockedExchange(/*(LONG*)*/&(quibase::__uibase_bEventReflect), 1024);
    }

	/** 选中第一个匹配selector的DOM元素
	 *	-return:		DOM		匹配的元素	
	 *	-params:	
	 *		-[in]	selector	标准的CSS语法
	 **/
	ECtrl GetCtrl(const char* selector, ...)const;

private:
    HELEMENT        m_heRoot;
};

#endif // QForm_h__
