#include "stdafx.h"
#include "QForm.h"
#include "ui/ECtrl.h"

using namespace htmlayout;

ECtrl QForm::GetRoot() const
{
    return m_heRoot;
}

ECtrl QForm::GetCtrl( const char* selector, ... ) const
{
    char buf[2049] = {0};
    va_list args; 
    va_start ( args, selector ); 
    vsnprintf(buf, 2048, selector, args ); 
    va_end ( args );

    dom::element eroot(m_heRoot);
    dom::element::find_first_callback ff;
    eroot.select_elements( &ff, buf); 

    return ff.hfound;
}

BOOL QForm::Attach( __in HELEMENT heRoot )
{
    if (NULL != m_heRoot)
    {
        ATLASSERT(FALSE);
        Detach();
    }
    if ( !dom::element(heRoot).is_valid() )
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
    // 先记录root才能init事件处理器啊。顺序不能颠倒
    m_heRoot = heRoot;
    __InitEventEntry();
    __InitReflectEventEntry();

    OnAttach();

    return TRUE;
}

QForm::QForm( HELEMENT heRoot /*= NULL*/ )
    :m_heRoot(heRoot)
{
    if (NULL != heRoot)
    {
        Attach(heRoot);
    }
}

