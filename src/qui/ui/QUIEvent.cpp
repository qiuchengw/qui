#include "stdafx.h"
#include "QUIEvent.h"
#include "ECtrl.h"
#include "QForm.h"

using namespace htmlayout;

namespace quibase
{
    // 用于记录事件是否被反射
    // 此界面只支持单线程模式
    // 使用TLS（线程本地存储）可以扩展为多线程模式，但是不想搞那么麻烦 -_-!!!
    // 仅能在事件回调函数中使用此数据，请不要在其他地方使用
    long volatile __uibase_bEventReflect = 0;

    // 用于 drag & drop 
    // 当前正在拖动的原始DOM值（鼠标拖动的其实是这个DOM的clone）
    HELEMENT volatile __uibase_heDragOrigin = NULL;
    // 拖动进入元素DOM值，当drop的时候，会将此值传给drop响应函数
    HELEMENT volatile __uibase_heDragEnter = NULL;
};

//////////////////////////////////////////////////////////////////////////

// 调试
#ifdef _DEBUG
htmlayout::debug_output_console g_dbgcon;

inline bool set_string( std::string& stupid_std_string, const wchar_t* str )
{
    if(!str) return false;
    stupid_std_string = aux::w2a(str);
    return true;
}

inline std::string id_or_name_or_text( HELEMENT he )
{
    dom::element el = he;

    std::string name;
    if(set_string( name, el.get_attribute("id")) ) return name;
    if(set_string( name, el.get_attribute("name")) ) return name;

    name = "{";
    std::string str = aux::w2a(el.text().c_str());
    if( str.length() )
        name += str;
    name += "}";
    return name.c_str();
}

inline std::string id_or_name_or_tag( HELEMENT he )
{
    if(!he)
        return "(no element)";
    dom::element el = he;
    std::string name;
    if(set_string( name, el.get_attribute("id")) ) 
        return name;
    if(set_string( name, el.get_attribute("name")) ) 
        return name;
    name = el.get_element_type();
    return name.c_str();
}

void TraceBehaviorEventToDC( BEHAVIOR_EVENT_PARAMS &p )
{
    g_dbgcon.printf("CmdCode:[0x%08X] " ,p.cmd);
    switch( p.cmd )
    {
    case BUTTON_CLICK:              // click on button
        g_dbgcon.printf("BUTTON_CLICK : %s",id_or_name_or_tag(p.heTarget).c_str());
        break;
    case BUTTON_PRESS:             
        g_dbgcon.printf("BUTTON_PRESS : %s",id_or_name_or_tag(p.heTarget).c_str());
        break;
    case BUTTON_STATE_CHANGED:      
        g_dbgcon.printf("BUTTON_STATE_CHANGED : %s",id_or_name_or_tag(p.heTarget).c_str());
        break;
    case EDIT_VALUE_CHANGING:      
        g_dbgcon.printf("EDIT_VALUE_CHANGING : %s",id_or_name_or_tag(p.heTarget).c_str());
        break;
    case EDIT_VALUE_CHANGED:        //after text change
        g_dbgcon.printf("EDIT_VALUE_CHANGED : %s",id_or_name_or_tag(p.heTarget).c_str());
        break;
    case SELECT_SELECTION_CHANGED:  
        g_dbgcon.printf("SELECT_SELECTION_CHANGED : %s",id_or_name_or_tag(p.heTarget).c_str());
        break;
    case SELECT_STATE_CHANGED:      
        g_dbgcon.printf("SELECT_STATE_CHANGED : %s",id_or_name_or_tag(p.heTarget).c_str());
        break; 
    case POPUP_REQUEST: 
        g_dbgcon.printf("POPUP_REQUEST : %s",id_or_name_or_tag(p.heTarget).c_str());
        break; 
    case POPUP_READY:               
        g_dbgcon.printf("POPUP_READY : %s",id_or_name_or_tag(p.heTarget).c_str());
        break; 
    case POPUP_DISMISSED:           break;// popup element is closed,
        g_dbgcon.printf("POPUP_DISMISSED : %s",id_or_name_or_tag(p.heTarget).c_str());
        break; 
    case MENU_ITEM_CLICK:                 // menu item click 
        g_dbgcon.printf("MENU_ITEM_CLICK : %s",id_or_name_or_tag(p.heTarget).c_str());
        break; 
    case ELEMENT_COLLAPSED:        
        g_dbgcon.printf("ELEMENT_COLLAPSED : %s",id_or_name_or_tag(p.heTarget).c_str());
        break; 
    case ELEMENT_EXPANDED:          
        g_dbgcon.printf("ELEMENT_EXPANDED : %s",id_or_name_or_tag(p.heTarget).c_str());
        break;
    case TABLE_ROW_CLICK:
        g_dbgcon.printf("TABLE_ROW_CLICK : %s",id_or_name_or_tag(p.heTarget).c_str());
        break;
    }
    g_dbgcon.printf("\n");
}

void TraceMouseEventToDC(MOUSE_PARAMS& p)
{
    switch (p.cmd)
    {
    case MOUSE_ENTER:// = 0,
        {
            g_dbgcon.printf("MOUSE_ENTER:[%s] \n" ,id_or_name_or_tag(p.target).c_str());
            break;
        }
    case MOUSE_LEAVE:// = 1,
        {
            g_dbgcon.printf("MOUSE_LEAVE:[%s] \n" ,id_or_name_or_tag(p.target).c_str());
            break;
        }
    case MOUSE_IDLE://  = 8, // mouse stay idle for some time
        {
            g_dbgcon.printf("MOUSE_IDLE:[%s] \n" ,id_or_name_or_tag(p.target).c_str());
            break;
        }
        //         case MOUSE_MOVE://  = 2,
        //             {
        //                 g_dbgcon.printf("MOUSE_MOVE:[%s][%d,%d] \n" ,
        //                     id_or_name_or_tag(p.target).c_str(),p.pos.x,p.pos.y);
        //                 break;
        //             }
    case MOUSE_UP | HANDLED://    = 3,
    case MOUSE_UP://    = 3,
        {
            g_dbgcon.printf("MOUSE_UP:[%s] - is_on_icon[%s] \n" ,
                id_or_name_or_tag(p.target).c_str(),p.is_on_icon ? "yes" : "no");
            break;
        }
    case MOUSE_DOWN | HANDLED://  = 4,
    case MOUSE_DOWN://  = 4,
        {
            g_dbgcon.printf("MOUSE_DOWN:[%s] - is_on_icon[%s] \n" ,
                id_or_name_or_tag(p.target).c_str(),p.is_on_icon ? "yes" : "no");
            break;
        }
    case MOUSE_DCLICK:// = 5,
        {
            g_dbgcon.printf("MOUSE_DCLICK:[%s] - is_on_icon[%s] \n" ,
                id_or_name_or_tag(p.target).c_str(),p.is_on_icon ? "yes" : "no");
            break;
        }
    case MOUSE_WHEEL:// = 6,        button_state 的值为滚轮移动值
        {
            g_dbgcon.printf("MOUSE_WHEEL:[%s] - delta[%d]\n" ,
                id_or_name_or_tag(p.target).c_str(),p.button_state);
            break;
        }
    case MOUSE_TICK://  = 7, // mouse pressed ticks
        {
            g_dbgcon.printf("MOUSE_TICK:[%s]\n" , id_or_name_or_tag(p.target).c_str());
            break;
        }
    case DROP://        = 9,   // item dropped, target is that dropped item 
        {
            g_dbgcon.printf("mouse DROP: drop[%s] to [%s]\n" , 
                id_or_name_or_tag(p.dragging).c_str(), id_or_name_or_tag(
                htmlayout::dom::element(p.target).parent()).c_str());

            break;
        }
    case DRAG_ENTER://  = 0xA, // drag arrived to the target element that is one of current drop targets.  
        {
            g_dbgcon.printf("DRAG_ENTER: drag[%s] enter [%s]上\n" , 
                id_or_name_or_tag(p.dragging).c_str(),id_or_name_or_tag(p.target).c_str());
            break;
        }
    case DRAG_LEAVE://  = 0xB, // drag left one of current drop targets. target is the drop target element.  
        {
            g_dbgcon.printf("DRAG_LEAVE: drag[%s] leave [%s]\n" , 
                id_or_name_or_tag(p.dragging).c_str(),id_or_name_or_tag(p.target).c_str());
            break;
        }
    case DRAG_REQUEST:// = 0xC,  // drag src notification before drag start. To cancel - return true from handler.
        {
            g_dbgcon.printf("DRAG_REQUEST: request drag[%s] cancel-return TRUE\n" , 
                id_or_name_or_tag(p.target).c_str());
            break;
        }
    case MOUSE_CLICK:// = 0xFF, // mouse click event
        {
            g_dbgcon.printf("MOUSE_CLICK: [%s]\n" , 
                id_or_name_or_tag(p.target).c_str());
            break;
        }
    case DRAGGING:// = 0x100,
        {
            g_dbgcon.printf("MOUSE_DRAGGING: dragging[%s]\n" , id_or_name_or_tag(p.dragging).c_str());
            break;
        }
    }
}

#endif

//////////////////////////////////////////////////////////////////////////

BOOL QUIEventEntry::ProcessBehaviorEvent( __in QUIEventTarget *pTarget, 
    __in BEHAVIOR_EVENT_PARAMS& params,
    __out LRESULT *pResult /*= NULL*/ )
{
    ATLASSERT( NULL != pTarget );
    //		ASSERT( NULL != params.he );

    ATLASSERT( pResult != NULL);
    *pResult = 0;

    // 调试显示消息到控制台窗口
#ifdef _DEBUG
    TraceBehaviorEventToDC(params);
#endif

    CTL_TYPE ctlType;

    union _BehaviorEventHandler _eh;

    switch (params.cmd)
    {
        // click on button
    case BUTTON_CLICK:
        // mouse down or key down in button
    case BUTTON_PRESS:// = 1,             
        // checkbox/radio/slider changed its state/value 
    case BUTTON_STATE_CHANGED:// = 2,      
        // before text change
    case EDIT_VALUE_CHANGING:// = 3,       
        // after text change
    case EDIT_VALUE_CHANGED:// = 4,        
        // broadcast notification, sent to all elements of some container being shown or hidden   
    case VISIUAL_STATUS_CHANGED:// = 0x11, 
        // broadcast notification, 
        // sent to all elements of some container that got new value of :disabled state
    case DISABLED_STATUS_CHANGED:// = 0x12,
        // element was collapsed, so far only behavior:tabs is sending these two to the panels
    case ELEMENT_COLLAPSED:// = 0x90,     
        // element was expanded,
    case ELEMENT_EXPANDED://,             
        // activate (select) child, 
        // command to switch tab programmatically, handled by behavior:tabs 
    case ACTIVATE_CHILD:// or DO_SWITCH_TAB,
        // hyperlink click
    case HYPERLINK_CLICK:// = 0x80,        
        // ui state changed, observers shall update their visual states. 
    case UI_STATE_CHANGED:
        {
            _eh.pfn = event_handler_.GetHandlerOfElment(params.heTarget,params.cmd);
            if (_eh.pfn != NULL)
            {
                (pTarget->*_eh.pfn_v_h)(params.heTarget);
                return TRUE;
            }
            break;
        }
        // selection in <select> changed
    case SELECT_SELECTION_CHANGED:// = 5,  
        // node in select expanded/collapsed, heTarget is the node
    case SELECT_STATE_CHANGED:// = 6, 
        {
            _eh.pfn = event_handler_.GetHandlerOfElment(params.heTarget,params.cmd);
            if (_eh.pfn != NULL)
            {
                ECtrl ctl(params.heTarget);
                ctlType = ctl.get_ctl_type();
                if ((CTL_SELECT_SINGLE == ctlType) || (CTL_SELECT_MULTIPLE == ctlType))
                {
                    if (aux::wcseqi(ctl.get_attribute("type"),L"tree"))// tree
                        (pTarget->*_eh.pfn_v_t_ti)(params.heTarget,params.he);
                    else    // list
                        (pTarget->*_eh.pfn_v_l_o)(params.heTarget,params.he);
                }
                else if (CTL_DD_SELECT == ctlType)  // combobox
                {
                    (pTarget->*_eh.pfn_v_x_o)(params.heTarget,params.he);
                }
                else    // other select
                {
                    (pTarget->*_eh.pfn_v_h_h)(params.heTarget,params.he);
                }
                return TRUE;
            }
            break;
        }

        // menu item activated by mouse hover or by keyboard,
    case MENU_ITEM_ACTIVE:// = 0xA,        
        // menu item click, 
        // he 是要求弹出menu的元素
    case MENU_ITEM_CLICK:// = 0xB,
        {
            _eh.pfn = event_handler_.GetHandlerOfElment(params.heTarget,params.cmd);
            if (_eh.pfn != NULL)
            {
                (pTarget->*_eh.pfn_v_h_mi)(params.he,params.heTarget);
                return TRUE;
            }
            break;
        }
        // request to show popup just received, 
        // here DOM of popup element can be modifed.
    case POPUP_REQUEST://   = 7,           
        // popup element has been measured and ready to be shown on screen,
        // here you can use functions like ScrollToView.
    case POPUP_READY://     = 8,          
        // popup element is closed,
        // here DOM of popup element can be modifed again - e.g. some items can be removed to free memory.
    case POPUP_DISMISSED:// = 9,          
        // popup is about to be closed
    case POPUP_DISMISSING:// = 0x13, 
        {
            _eh.pfn = event_handler_.GetHandlerOfElment(params.heTarget,params.cmd);
            if (_eh.pfn != NULL)
            {
                (pTarget->*_eh.pfn_v_h_p)(params.he,params.heTarget);
                return TRUE;
            }
            break;
        }
        // evt.he is a menu dom element that is about to be shown.
        // You can disable/enable items in it.      
    case CONTEXT_MENU_SETUP://   = 0xF,    
        // "right-click", BEHAVIOR_EVENT_PARAMS::he is current popup menu HELEMENT being processed or NULL.
        //	application can provide its own HELEMENT here (if it is NULL) or modify current menu element.
    case CONTEXT_MENU_REQUEST:// = 0x10,   
        {
            _eh.pfn = event_handler_.GetHandlerOfElment(params.he,params.cmd);
            if (_eh.pfn != NULL)
            {
                (pTarget->*_eh.pfn_v_h_m)(params.he,params.heTarget);
                return TRUE;
            }
            break;
        }
        // click on cell in table header,  
        //		target = the cell, reason = index of the cell (column number, 0..n)
    case TABLE_HEADER_CLICK: 
        // click on data row in the table, 
        //		target = the row,reason = index of the row (fixed_rows..n)
    case TABLE_ROW_CLICK:           
        // mouse dbl click on data row in the table, target is the row
        //     target = the row, 
        //     reason = index of the row (fixed_rows..n)
    case TABLE_ROW_DBL_CLICK:       
        //      target = the cell
    case SHEET_CELLWIDGET_CLOSED:
        {
            _eh.pfn = event_handler_.GetHandlerOfElment(params.he,params.cmd);
            if (_eh.pfn != NULL)
            {
                (pTarget->*_eh.pfn_v_t_c)(params.he,params.heTarget);
                return TRUE;
            }
            break;
        }
        //	ROWS_DATA_REQUEST, 
        //		request from virtual grid to data source behavior to fill data in the table
        //		parameters passed throug DATA_ROWS_PARAMS structure.
    case ROWS_DATA_REQUEST:
        {
            _eh.pfn = event_handler_.GetHandlerOfElment(params.heTarget,params.cmd);
            if (_eh.pfn != NULL)
            {
                (pTarget->*_eh.pfn_v_t_drp)(params.heTarget,*((DATA_ROWS_PARAMS*)params.reason));
                return TRUE;
            }
            break;
        }
        // behavior:form detected submission event. 
        // BEHAVIOR_EVENT_PARAMS::data field contains data to be posted.
        // BEHAVIOR_EVENT_PARAMS::data is of type T_MAP in this case key/value pairs of data that is about 
        // to be submitted. You can modify the data or discard submission by returning TRUE from the handler.
    case FORM_SUBMIT | SINKING:
        // behavior:form detected reset event (from button type=reset). 
        // BEHAVIOR_EVENT_PARAMS::data field contains data to be reset.
        // BEHAVIOR_EVENT_PARAMS::data is of type T_MAP in this case key/value pairs of data that is about 
        // to be rest. You can modify the data or discard reset by returning TRUE from the handler.
    case FORM_RESET | SINKING://,                    
        {
            _eh.pfn = event_handler_.GetHandlerOfElment(params.heTarget,params.cmd);
            if (_eh.pfn != NULL)
            {
                *pResult = (pTarget->*_eh.pfn_b_c_d)(params.heTarget,params.data);
                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

BOOL QUIEventEntry::ProcessMouseEvent( __in QUIEventTarget *pTarget, 
    __in MOUSE_PARAMS &prm, __out BOOL &bReturn )
{
    using namespace htmlayout::dom;

#ifdef _DEBUG
    TraceMouseEventToDC(prm);
#endif

    _MouseEventHandler _meh;
    switch (prm.cmd)
    {
    case MOUSE_ENTER:// = 0,
    case MOUSE_LEAVE:// = 1,
    case MOUSE_TICK://  = 7, // mouse pressed ticks
    case MOUSE_IDLE://  = 8, // mouse stay idle for some time
    case DRAGGING:// = 0x100,
        {
            if (NULL != prm.target)
            {
                _meh.pfn = mouse_handler_.GetHandlerOfElment(prm.target,prm.cmd);
                if (NULL != _meh.pfn)
                {
                    (pTarget->*_meh.pfn_v_h)(prm.target); 
                    return TRUE;
                }
            }
            break;
        }
    case MOUSE_WHEEL:// = 6,        button_state 的值为滚轮移动值
        {
            _meh.pfn = mouse_handler_.GetHandlerOfElment(prm.target,MOUSE_WHEEL);
            if (NULL != _meh.pfn)
            {
                (pTarget->*_meh.pfn_v_h_u_i)(prm.target, prm.alt_state, (int)prm.button_state ); 
                return TRUE;
            }
            break;
        }
    case MOUSE_UP | HANDLED://    = 3,
    case MOUSE_UP://    = 3,
    case MOUSE_DOWN | HANDLED://  = 4,
    case MOUSE_DOWN://  = 4,
    case MOUSE_MOVE://  = 2,
    case MOUSE_DCLICK:// = 5,
    case MOUSE_CLICK:// = 0xFF, // mouse click event
        {
            UINT nCmd = prm.cmd;
            if ( (MOUSE_UP | HANDLED) == nCmd)  // 通常捕获到的都是已经handled的
                nCmd = MOUSE_UP;
            else if ((MOUSE_DOWN | HANDLED) == nCmd)
                nCmd = MOUSE_DOWN;
            _meh.pfn = mouse_handler_.GetHandlerOfElment(prm.target, nCmd);
            if (NULL != _meh.pfn)
            {
                (pTarget->*_meh.pfn_v_mp)(prm); 
                return TRUE;
            }
            break;
        }

    case DROP://        = 9,   // item dropped, target is that dropped item 
        {
            _meh.pfn = mouse_handler_.GetHandlerOfElment(prm.target,prm.cmd);
            if (NULL != _meh.pfn)
            {
                (pTarget->*_meh.pfn_v_h_h_h)(quibase::__uibase_heDragEnter,  // 容器
                    quibase::__uibase_heDragOrigin,   // src
                    prm.dragging);    // dest，这个draggging是target的clone，或者就是target 
                return TRUE;
            } 
            break;
        }
    case DRAG_ENTER://  = 0xA, // drag arrived to the target element that is one of current drop targets.  
    case DRAG_LEAVE://  = 0xB, // drag left one of current drop targets. target is the drop target element.  
        {
            _meh.pfn = mouse_handler_.GetHandlerOfElment(prm.target,prm.cmd);
            quibase::__uibase_heDragEnter = prm.target;
            if (NULL != _meh.pfn)
            {
                (pTarget->*_meh.pfn_v_h_h)(prm.target, prm.dragging); 
                return TRUE;
            }
            break;
        }
    case DRAG_REQUEST:// = 0xC,  // drag src notification before drag start. To cancel - return true from handler.
        {
            _meh.pfn = mouse_handler_.GetHandlerOfElment(prm.target,DRAG_REQUEST);
            // 记录下当前拖动到这个DOM元素的原始值
            // 否则在drag_enter /over/drop的时候是获取不到这个值的 
            quibase::__uibase_heDragOrigin = prm.target;
            // 然后再处理这个消息
            if (NULL != _meh.pfn)
            {
                bReturn = (pTarget->*_meh.pfn_b_h)(prm.target); 
                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////
QUIEventEntry::QUI_ITEM::QUI_ITEM( BOOL bUseID,LPCWSTR v,QUI_PEVENT p )
    :bID(bUseID),pfn(p),bEnable(TRUE)
{
    ATLASSERT(NULL != v);
    WTL::SecureHelper::strcpyW_x(sVal, _MAX_ID_LEN_, v);
    ATLASSERT(pfn != NULL);
}

QUI_PEVENT QUIEventEntry::MapCodeHanlder::GetHandlerOfElment( __in HELEMENT he, __in int eCode )
{
    // 先检查是否注册了这个事件处理器
    CodeHandler *pCH = GetCodeHandler(eCode,FALSE);
    if (NULL == pCH)
    {
        return NULL;
    }

    // 检查这个DOM对象id对应的是否有此代码处理器
    QUI_PEVENT pHandler = NULL;
    dom::element el(he);
#ifdef _DEBUG
    // 只是为了方便观察元素到底是谁
    json::astring shtml = el.get_html(true);
#endif
    LPCWSTR pszAtti = el.get_attribute("id");
    if ( NULL != pszAtti )
    {
        pHandler = pCH->GetHandler(pszAtti,TRUE);
        if ( NULL != pHandler )
        {
            return pHandler;
        }
    }
    // 没有找到属性id对应的事件处理器，试一下name对应的处理器
    pszAtti = el.get_attribute("name");
    if ( NULL != pszAtti )
    {
        pHandler = pCH->GetHandler(pszAtti, FALSE);
    }
    return pHandler;
}

void QUIEventEntry::MapCodeHanlder::Clear()
{
    _MapItr itrEnd = _End();
    for (_MapItr i = m_map.begin(); i != itrEnd; ++i)
    {
        i->second->ClearAll();
        delete i->second;
    }
    m_map.clear();
}

QUIEventEntry::CodeHandler* QUIEventEntry::MapCodeHanlder::GetCodeHandler( 
    __in int eCode, __in BOOL bNewIfNotExist/*=FALSE*/ )
{
    _MapItr iRet = FindTList(eCode);
    if (_End() != iRet) 
    {
        return iRet->second;
    }

    if ( bNewIfNotExist )
    {
        CodeHandler *pCH = new CodeHandler(eCode);
        m_map[eCode] = pCH;
        return pCH;
    }
    return NULL;
}

QUIEventEntry::CodeHandler::_ListItr QUIEventEntry::CodeHandler::Find( 
    LPCWSTR szVal,BOOL bID )
{
    _ListItr itrEnd = _End();
    for (_ListItr i = m_list.begin(); i != itrEnd; ++i)
    {
        if (((*i)->bID == bID) && (wcsicmp((*i)->sVal , szVal) == 0))
        {
            return i;
        }
    }
    return itrEnd;
}

void QUIEventTarget::__AddFormEntry( QForm* pForm )
{
    if (   IsFormInChain(pForm) // 只能注册一次
        || ((QForm*)this == pForm))  // 不能是自己
    {
        ATLASSERT(FALSE);
        return ;
    }
    forms_.push_back(pForm);
}

BOOL QUIEventTarget::handle_event_body(HELEMENT he,BEHAVIOR_EVENT_PARAMS& bep, LRESULT *pResult)
{
    if ( !events_entry_.ProcessBehaviorEvent(this, bep, pResult) )
    {
        LstFormItr iEnd = forms_.end();
        for (LstFormItr i = forms_.begin(); i != iEnd; ++i)
        {
            if ( (*i)->handle_event_body(he, bep, pResult) )
            {
                // 如果子form又把事件反射回来的话，那么本地还需要处理这个反射事件
                if (IsFormEventReflected())
                {
                    QUIEventTarget::handle_reflect_event(he, bep, pResult);
                }
                return TRUE;
            }
        }
        return FALSE;
    }
    return TRUE;
}

BOOL QUIEventTarget::handle_mouse_body(__in HELEMENT he, __in MOUSE_PARAMS& mp, __out BOOL &bReturn)
{
    if ( !events_entry_.ProcessMouseEvent(this, mp, bReturn) )
    {
        LstFormItr iEnd = forms_.end();
        for (LstFormItr i = forms_.begin(); i != iEnd; ++i)
        {
            if ( (*i)->handle_mouse_body(he, mp, bReturn) )
            {
                if (IsFormEventReflected())
                {
                    QUIEventTarget::handle_reflect_mouse(he, mp, bReturn);
                }
                return TRUE;
            }
        }
        return FALSE;
    }
    return TRUE;
}

BOOL QUIEventTarget::handle_key_body(HELEMENT he, KEY_PARAMS& kp)
{
    if ( !events_entry_.ProcessKeyEvent(this, kp) )
    {
        LstFormItr iEnd = forms_.end();
        for (LstFormItr i = forms_.begin(); i != iEnd; ++i)
        {
            if ( (*i)->handle_key_body(he, kp) )
            {
                if (IsFormEventReflected())
                {
                    QUIEventTarget::handle_reflect_key(he, kp);
                }
                return TRUE;
            }
        }
        return FALSE;
    }
    return TRUE;
}
