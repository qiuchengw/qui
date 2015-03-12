#include "behavior_aux.h"

#include <commctrl.h> // for tooltip support

namespace htmlayout 
{

/*
BEHAVIOR: popup
    goal: 
    // <button>test</button> 经解析后 变为 <button><text>test</text></button>
    // 按钮上的文本被包装了一层text标签。通常情况下，鼠标按下和弹起消息都会被定向到
    // 这层text元素里。要想捕获mouse_down/up消息，可使用behavior:mouse_updown_capture;
          
TYPICAL USE CASE:
    
SAMPLE:
    <div style="behavior:mouse_updown_capture button; width:50px;height:20px;" >MMMMMMMM</div>
*/

struct mouse_updown_capture: public behavior
{
    // ctor
    mouse_updown_capture(): behavior(HANDLE_MOUSE|HANDLE_INITIALIZATION, "mouse_updown_capture") {}
    
    virtual void attached  (HELEMENT he )
    {

    }

    virtual BOOL handle_mouse(HELEMENT he, MOUSE_PARAMS& prm )
    {
        switch (prm.cmd)
        {
//        case MOUSE_DOWN:
        case MOUSE_DOWN|HANDLED:
            {
                dom::element el(prm.target);
                if ( aux::streqi("text",el.get_element_type()) )
                {
                    prm.target = el.parent();
                    BOOL bProcessed = FALSE;
                    HTMLayoutTraverseUIEvent(HANDLE_MOUSE, &prm, &bProcessed);
                    el = prm.target;
                }
                el.set_capture();
                break;
            }
//        case MOUSE_UP:
        case MOUSE_UP|HANDLED:
            {
                dom::element el(prm.target);
                if ( aux::streqi("text",el.get_element_type()) )
                {
                    prm.target = el.parent();
                    BOOL bProcessed = FALSE;
                    HTMLayoutTraverseUIEvent(HANDLE_MOUSE, &prm, &bProcessed);
                    el = prm.target;
                }
                el.release_capture();
                break;
            }
        }
        return FALSE;
    }

};


// instantiating and attaching it to the global list

mouse_updown_capture button_with_capture_instance;
} // htmlayout namespace
