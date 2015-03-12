#include "behavior_aux.h"

#include <commctrl.h> // for tooltip support

namespace htmlayout 
{

/*
BEHAVIOR: popup
    goal: 
          
          
TYPICAL USE CASE:
    
SAMPLE:
	<ul type="starbox" stars="5" index="3" />
*/

struct starbox: public behavior
{
    // ctor
    starbox(): behavior(HANDLE_BEHAVIOR_EVENT|HANDLE_MOUSE, "starbox") {}

	virtual void attached  (HELEMENT he ) 
	{ 
		dom::element eUl(he);
        eUl.clear();
		int nStarts = aux::wtoi(eUl.get_attribute("stars"));
		if ((nStarts <= 0) || (nStarts > 10))
		{
			assert(false);
			return;
		}
		for (int i = 0; i < nStarts; i++)
		{
			eUl.append(dom::element::create("li"));
		}
		fresh_starbox(eUl);
	} 

	void on_click(dom::element eul,int idx)
	{
		int c = eul.children_count();
		idx = (idx > c)?c:idx;
		eul.set_attribute("index",aux::itow(idx));
	}
	void fresh_starbox(dom::element eul)
	{
		int idx = aux::wtoi(eul.get_attribute("index"));
		int c = eul.children_count();
		idx = (idx > c)?c:idx;
		for (int i = 0; i < idx; ++i)
		{
			dom::element(eul.child(i)).toggle_state(STATE_CHECKED,true);
		}
		for (int i = idx; i < c; ++i)
		{
			dom::element(eul.child(i)).toggle_state(STATE_CHECKED,false);
		}
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
		  {
			  dom::element eli = target;
			  if (eli.is_valid())
			  {
				  on_click(he,eli.index()+1);
				  dom::element(he).send_event(BUTTON_STATE_CHANGED);
				  fresh_starbox(he);
			  }
			return true;
		  }
      }
      return false;
    }
};


// instantiating and attaching it to the global list

starbox starbox_instance;
} // htmlayout namespace
