#include "behavior_aux.h"
#include "time/QTime.h"

namespace htmlayout 
{

/*
BEHAVIOR: 
	xtimectrl
COMMENTS: 
<div type="timectrl">12:30
	<table class="xtimeitems">
	<tr><td>12:30</td></tr>
	<tr><td>12:40</td></tr>
	<tr><td>12:50</td></tr>
	<tr><td>12:30</td></tr>
	</table>
</div>
*/

#define ITEM_TIME_SPAN 10 

bool belongs_to( dom::element parent, dom::element child );
using namespace htmlayout::dom;

struct xtimepopup: public behavior
{
    // ctor
    xtimepopup(): behavior(HANDLE_MOUSE | HANDLE_BEHAVIOR_EVENT
		|HANDLE_KEY|HANDLE_FOCUS, "xtimepopup") {}

    virtual void attached  (HELEMENT he ) 
    { 
      dom::element el = he;

	  QTime qt;
	  qt.ParseDateTime(L"2013/5/5 0:0:0");
	  update_timeitems(el,qt);
    } 
   
    virtual void detached  (HELEMENT he ) 
    { 
    } 

	QTime get_item_time(dom::element & eItem)
	{
		QTime qt;
		if (!qt.ParseDateTime(aux::a2w(eItem.get_html(false).c_str())))
		{
			assert(false);
		}
		return qt;
	}

	void update_timeitems(dom::element &root,QTime t)
	{
		QTime tcur;
        const wchar_t *pszTime = root.get_attribute("time");
		tcur.ParseDateTime((NULL == pszTime)?L"0:0:0":pszTime, VAR_TIMEVALUEONLY);

		CStdString str, sall;// = L"<tr><td><widget type=\"time\" /></td></tr>";
		for (int i = 0; i < 10; i++)
		{
			str.Format(L"<tr><td %s %s>%s</td></tr>",
				(t.GetMinute())?L"":L".xclock", // 整点
				tcur.CompareTime(t)?L"":L"checked", // 选中
				t.Format(L"%H:%M:%S")); 
            sall += str;
			t += QTimeSpan(0,0,ITEM_TIME_SPAN,0);
		}
		utf8::ostream o;
		o<<sall;
		root.set_html(o.data(),o.length());
	}

    virtual BOOL on_mouse(HELEMENT he, HELEMENT target, UINT event_type, POINT pt, UINT mouseButtons, UINT keyboardStates )
    {
      switch( event_type )
      {
        case MOUSE_DOWN:
			{
				::HTMLayoutHidePopup(he);
				return true;
			}
		case MOUSE_WHEEL:
			{
				element etable = he,etd;
				if (1 == mouseButtons)	// 前滚
				{
					etd = element(etable.child(0)).child(0);
					update_timeitems(etable,(get_item_time(etd) - QTimeSpan(0,0,ITEM_TIME_SPAN,0)));
				}
				else	// 后滚
				{
					etd = element(etable.child(1)).child(0);
					update_timeitems(etable,get_item_time(etd));
				}
				return true;
			}
      }
      return false; 
    }

	virtual BOOL on_key(HELEMENT he, HELEMENT target, UINT event_type, UINT code, UINT keyboardStates )
	{
		if (event_type != KEY_DOWN)
			return false;
		switch (code)
		{
		case VK_UP:
			{
				element etable = he;
				element etd = element(etable.child(0)).child(0);
				update_timeitems(etable,(get_item_time(etd) - QTimeSpan(0,0,ITEM_TIME_SPAN,0)));
				return true;
			}
		case VK_DOWN:
			{
				element etable = he;
				element etd = element(etable.child(1)).child(0);
				update_timeitems(etable,get_item_time(etd));
				return true;
			}
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
         return true;
      }
      return false;
    }
  
    virtual BOOL on_focus  (HELEMENT he, HELEMENT target, UINT event_type ) 
    { 
      if( (event_type == FOCUS_LOST) || (event_type == (FOCUS_LOST | SINKING)) )
      {
        if( !belongs_to( he, target ) )
          ::HTMLayoutHidePopup(he);
        return TRUE;
      }
      return FALSE;
    }
};

struct xtimectrl: public behavior
{
    // ctor
    xtimectrl(): behavior(HANDLE_TIMER|HANDLE_MOUSE | HANDLE_FOCUS | HANDLE_BEHAVIOR_EVENT , "xtimectrl") {}

    virtual void attached  (HELEMENT he ) 
    { 
		element r = he;
		r.append(element::create("caption",L"00:00:00"));
		element etable = element::create("table");
		r.append(etable);
		etable.set_attribute("time",L"00:00:00");
	} 

	// hh:mm:ss
// 	void set_time(element&root,const json::string &s)
// 	{
// 		QTime qt;
// 		if (qt.ParseDateTime (s.c_str()))
// 		{
//             root.set_value(s);
//             root.set_text(s.c_str());
// 		}
// 	}
// 	void set_time(element&root,const QTime& t)
// 	{
//         root.set_text(t.Format(L"%H:%M:%S"));
//         root.set_value(json::string((LPCTSTR)(t.Format (L"%H:%M:%S"))));
// 	}

	virtual BOOL on_mouse(HELEMENT he, HELEMENT target, UINT event_type, 
		POINT pt, UINT mouseButtons, UINT keyboardStates )
	{
		switch(event_type)
		{
		case MOUSE_DOWN:
			{
				element er = he;
				HELEMENT hPopup = er.find_first("table");
				if( element(hPopup).is_valid() && er.is_valid() && !er.get_state(STATE_OWNS_POPUP) )
				{
					::HTMLayoutShowPopup(hPopup,er,2); // show it below
				}
				return true;
			}
		}
		return false;
	}

	virtual BOOL on_event (HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason ) 
	{ 
		switch(type)
		{
		case POPUP_DISMISSED:
			{
				dom::element eSel = element(target).find_first("td:checked");
				element ecaption = element(he).find_first("caption");
				if (eSel.is_valid() && ecaption.is_valid())
				{
					json::string s = aux::a2w(eSel.get_html(false).c_str());
					ecaption.set_value(json::value(s));
					element(he).send_event(SELECT_SELECTION_CHANGED);
					element(target).set_attribute("time", s.c_str());
				}
				return TRUE;
			}
		}
		return FALSE;
	}
	
};

// instantiating and attaching it to the global list
xtimectrl xtimectrl_instance;
xtimepopup xtimectrl_popup_instance;
}
