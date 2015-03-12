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

struct xprogress: public behavior
{
    // ctor
    xprogress(): behavior(HANDLE_BEHAVIOR_EVENT|HANDLE_MOUSE, "xprogress") {}
	
	virtual void attached  (HELEMENT he ) 
	{ 
		dom::element el = he;
		el.append(dom::element::create("div"));

	} 
	
	virtual BOOL on_mouse(HELEMENT he, HELEMENT target, UINT event_type, 
		POINT pt, UINT mouseButtons, UINT keyboardStates )
	{
		switch (event_type)
		{
		case MOUSE_DOWN:
			break;
		}
		return false;
	}
};


// instantiating and attaching it to the global list

xprogress xprogress_instance;
} // htmlayout namespace
