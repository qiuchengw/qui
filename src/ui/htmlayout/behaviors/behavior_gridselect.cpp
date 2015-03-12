#include "behavior_aux.h"
#include <string>

/*
BEHAVIOR: grid-select

提供规则的行列布局选择

TYPICAL USE CASE:
<table style="behavior:grid-select;" maxrow=4 maxcol=4 row=2 col=2 />

*/

bool IsInActiveWindow(HWND hwnd);

extern htmlayout::debug_output_console g_dbgcon;

namespace htmlayout 
{
	struct grid_select: public behavior
	{
		// ctor
		grid_select(): behavior(HANDLE_BEHAVIOR_EVENT|HANDLE_MOUSE, "grid-select") { }

		void attached(HELEMENT h)
		{
			dom::element el(h);
			int mr = el.get_attribute_int("maxrow",4);
			int mc = el.get_attribute_int("maxcol",4);
			int r = el.get_attribute_int("row",2);
			int c = el.get_attribute_int("col",2);
			r = (r > mr)?mr:r;
			c = (c > mc)?mc:c;

			const wchar_t* pname = el.get_attribute("tdname");

			el.clear();

			std::wstring s;
			for (int i = 0; i < mr; i++)
			{
				s += L"<tr>";
				for (int j = 0; j < mc; j++)
				{
					s += L"<td ";
					s += L"name=\"";
					s += pname;
					s += L"\"/>";
				}
				s += L"</tr>";
			}

			utf8::ostream o;
			o<<s.c_str();
			el.set_html(o.data(),o.length());

			select_grid(el,r,c);
		}

		void clear_select(dom::element &el)
		{
			for (unsigned int i = 0; i < el.children_count(); i++)
			{
				dom::element etr = el.child(i);
				for (unsigned int j = 0; j < etr.children_count(); j++)
				{
					dom::element(etr.child(j)).toggle_state(STATE_CHECKED,false);
				}
			}
		}

		void select_grid(dom::element& el,int row,int col,bool breal = false)
		{
			clear_select(el);

			dom::element etr,etd;
			for (int i = 0; i < row; i++)
			{
				etr = el.child(i);
				for (int j = 0; j < col; j++)
				{
					dom::element(etr.child(j)).toggle_state(STATE_CHECKED,true);
				}
			}
			if (breal)
			{
				el.set_attribute("row",aux::itow(row));
				el.set_attribute("col",aux::itow(col));
			}
		}

		void on_tdhover(dom::element &etd)
		{
			int c = etd.index() + 1;
			dom::element etr = etd.parent();
			int r = etr.index() + 1;

			select_grid(dom::element(etr.parent()),r,c,false);
		}

		void on_tdclick(dom::element &etd)
		{
			int c = etd.index() + 1;
			dom::element etr = etd.parent();
			int r = etr.index() + 1;

			select_grid(dom::element(etr.parent()),r,c,true);
		}

		virtual BOOL on_mouse(HELEMENT he, HELEMENT target, UINT event_type, 
			POINT pt, UINT mouseButtons, UINT keyboardStates )
		{
			switch( event_type )
			{
			case MOUSE_ENTER: 
				{
					dom::element el = he;

				}
				break;
			case MOUSE_LEAVE: 
				{
					dom::element el = he;
				
				}
				break;
			}
			return false;
		}

		virtual BOOL on_event (HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason ) 
		{ 
			if (BUTTON_CLICK == type)
			{
				dom::element etd = target;
				if (etd.is_valid())
				{
					on_tdclick(etd);
				}
			}
			return false;
		}  
	};

	// instantiating and attaching it to the global list
	grid_select grid_select_instance;

} // htmlayout namespace

