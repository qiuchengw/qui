#include "behavior_aux.h"

namespace htmlayout 
{

/*
BEHAVIOR: 
    goal: 

TYPICAL USE CASE:
    
SAMPLE:
*/

struct pagenav: public behavior
{
    // ctor
    pagenav(): behavior(HANDLE_BEHAVIOR_EVENT|HANDLE_MOUSE|HANDLE_KEY, "pagenav") {}

	virtual BOOL on_key(HELEMENT he, HELEMENT target, UINT event_type, UINT code, UINT keyboardStates )
	{
		dom::element e(he);
		if (!e.get_state(STATE_CHECKED) &&(event_type != (KEY_UP | SINKING)))
			return TRUE;
		bool bsend = false;
		switch (code)
		{
		case VK_LEFT:
			{
				dom::element epv = e.prev_sibling();
				if (epv.is_valid())
				{
					e.toggle_state(STATE_CHECKED,false);
					e.toggle_state(STATE_FOCUS,false);
					epv.toggle_state(STATE_CHECKED,true);
					epv.toggle_state(STATE_FOCUS,true);
					bsend = true;
				}
				break;
			}
		case VK_RIGHT:
			{
				dom::element epn = e.next_sibling();
				if (epn.is_valid())
				{
					e.toggle_state(STATE_CHECKED,false);
					e.toggle_state(STATE_FOCUS,false);
					epn.toggle_state(STATE_CHECKED,true);
					epn.toggle_state(STATE_FOCUS,true);
					bsend = true;
				}
				break;
			}
		}
		if (bsend)
		{
			dom::element ep = e.parent();
			ep.send_event(SELECT_SELECTION_CHANGED,0,ep);
			return TRUE;
		}
		return TRUE;
	}
    virtual BOOL on_event (HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason ) 
    { 
      switch( type )
      {
      case BUTTON_STATE_CHANGED: 
		  {
			  dom::element eCtrl = dom::element(he).select_parent(L"widget[type=\"pagenav\"]",4);
			  if (eCtrl.is_valid())
			  {
				  eCtrl.send_event(BUTTON_STATE_CHANGED,0,eCtrl);
			  }
			    return true;
		  }
      }
      return false;
    }
};


// instantiating and attaching it to the global list

pagenav pagenav_instance;
} // htmlayout namespace
