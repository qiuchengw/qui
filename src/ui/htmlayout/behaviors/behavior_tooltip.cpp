#include "behavior_aux.h"

#include <commctrl.h> // for tooltip support

namespace htmlayout 
{

/*
BEHAVIOR: popup
    goal: 
          
          
TYPICAL USE CASE:
    
SAMPLE:
*/

struct qtooltip: public behavior
{
    // ctor
    qtooltip(): behavior(HANDLE_MOUSE | HANDLE_BEHAVIOR_EVENT | HANDLE_FOCUS, "qtooltip") {}

	virtual BOOL on_focus(HELEMENT he, HELEMENT target, UINT event_type )
	{
		if (FOCUS_LOST == event_type)
		{
			HTMLayoutHidePopup(he);
			dom::element(he).set_style_attribute("visibility",L"collapse");
		}
		return TRUE;
	}
    virtual BOOL on_mouse(HELEMENT he, HELEMENT target, UINT event_type, POINT pt, UINT mouseButtons, UINT keyboardStates )
    {
      switch( event_type )
      {
        case MOUSE_DOWN:
        case MOUSE_UP | HANDLED:
        case MOUSE_UP:
          ::HTMLayoutHidePopup(he);
		  dom::element(he).set_style_attribute("visibility",L"collapse");
          return true;
      }
      return false; 
    }
 
    virtual BOOL on_event (HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason ) 
    { 
      switch( type )
      {
      case BUTTON_CLICK: 
#pragma warning( suppress:4063 ) // case 'XXX' is not a valid value for switch of enum 'BEHAVIOR_EVENTS'
      case BUTTON_CLICK | HANDLED:
      case HYPERLINK_CLICK: 
#pragma warning( suppress:4063 ) // case 'XXX' is not a valid value for switch of enum 'BEHAVIOR_EVENTS'
      case HYPERLINK_CLICK | HANDLED:
         ::HTMLayoutHidePopup(he);
		 dom::element(he).set_style_attribute("visibility",L"collapse");
         break;
      }
      return false;
    }
};


// instantiating and attaching it to the global list

qtooltip qtooltip_instance;


} // htmlayout namespace
