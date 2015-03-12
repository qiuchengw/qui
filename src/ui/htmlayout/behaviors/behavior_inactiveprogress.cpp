#include "behavior_aux.h"

#include <commctrl.h> // for tooltip support

namespace htmlayout 
{

	/*
	BEHAVIOR: inactive-progress
	goal: 鼠标可拉动操作的progressbar
		操作结果，将会产生BUTTON_STATE_CHANGED 事件

	TYPICAL USE CASE:

	SAMPLE:
		<widget type="inactive-progress" id="bar-progress" minvalue="0" maxvalue="100" value="30" />

	*/

	struct mouseprogress: public behavior
	{
		// ctor
		mouseprogress(): behavior(HANDLE_MOUSE, "inactive-progress") {}

		virtual void attached  (HELEMENT he ) 
		{ 
		} 

		double pos2value(dom::element&el,int x)
		{
			RECT rc = el.get_location();
			int w = rc.right - rc.left;
			int imin = aux::wtoi(el.get_attribute("minvalue")) ;
			int imax = aux::wtoi(el.get_attribute("maxvalue")) ;
			int v = imin + ((x * (imax - imin)) / ((w>0)?w:1));
			v = max(imin,v);
			v = min(imax,v);
			return v;
		}
		
		virtual BOOL on_mouse(HELEMENT he, HELEMENT target, UINT event_type, 
			POINT pt, UINT mouseButtons, UINT keyboardStates )
		{
			dom::element el(he);
			switch (event_type)
			{
			case MOUSE_CLICK:
				{
					el.set_value(pos2value(el,pt.x));
					el.post_event(BUTTON_STATE_CHANGED);
					break;
				}
			case MOUSE_DOWN:
				{
					m_btrack = true;
					el.set_capture();
					el.set_attribute("tracking",L"true");
					break;
				}
			case MOUSE_UP:
				{
					m_btrack = false;
					el.release_capture();
					el.remove_attribute("tracking");
					break;
				}
			case MOUSE_MOVE:
				{
					if (m_btrack)
					{
						el.set_value(pos2value(el,pt.x));
						el.post_event(BUTTON_STATE_CHANGED);
					}
					break;
				}
			}
			return false;
		}
		bool m_btrack;
	};


	// instantiating and attaching it to the global list

	mouseprogress mouseprogress_instance;
} // htmlayout namespace
