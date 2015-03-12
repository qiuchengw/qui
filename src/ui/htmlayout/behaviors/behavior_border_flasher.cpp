#include "behavior_aux.h"
#include "../../ECtrl.h"

/*
BEHAVIOR: border-flasher
    
  闪烁边框，可配合ECtrl：：FlashBorder使用

TYPICAL USE CASE:
   < div style="behavior:border-flasher " />
*/

namespace htmlayout 
{

  struct border_flahser: public behavior
  {
      // ctor
      border_flahser(): behavior(HANDLE_TIMER, "border-flasher") { }


      virtual BOOL on_timer  (HELEMENT he ,UINT_PTR extTimerId) 
      { 
		  if (ID_TIMER_ECTRL_FLASHBORDER != extTimerId)
			return FALSE;
		  dom::element el = he;
		  int nFlashTimes = el.get_attribute_int("flash-times");
		  if (nFlashTimes % 2)
		  {
			  el.set_style_attribute("border",
				  el.get_attribute("old-border-style"));
		  }
		  else
		  {
			  el.set_style_attribute("border",
				  el.get_attribute("flash-border-style"));
		  }

		  if (--nFlashTimes <= 0)
		  {
			  el.set_style_attribute("border",
				  el.get_attribute("old-border-style"));
			  el.remove_attribute("old-border-style");
			  el.remove_attribute("flash-border-style");
			  el.remove_attribute("flash-times");
			   // 不再闪烁
			  return 0;
		  }
		  else
		  {
			  el.set_attribute("flash-times",aux::itow(nFlashTimes));
		  }
         return true; // do not need timer anymore.
	  }
  
  };

  // instantiating and attaching it to the global list
  border_flahser border_flahser_instance;

} // htmlayout namespace



