#include "behavior_aux.h"


#include "ui/ECtrl.h"

namespace htmlayout 
{

/*
BEHAVIOR: xcolorpicker
    goal: color selector
          
TYPICAL USE CASE:
	<widget type="colorpicker" />

SAMPLE:
*/

struct colorpicker: public behavior
{
    // ctor
    colorpicker(): behavior(HANDLE_BEHAVIOR_EVENT|HANDLE_MOUSE, "xcolorpicker") {}
	void init_colors(dom::element &ePopup)
	{
		LPCWSTR sTable = //L"<tr><td style=\"background-color:#F00\" /></tr>";
			L"<tr><td style=\"background-color:#CCFFCC\" /><td style=\"background-color:#CCFF99\" /><td style=\"background-color:#CCFF66\" /><td style=\"background-color:#CCFF33\" /><td style=\"background-color:#CCFF00\" /><td style=\"background-color:#66FF00\" />"
			L"<td style=\"background-color:#66FF33\" /><td style=\"background-color:#66FF66\" /><td style=\"background-color:#66FF99\" /><td style=\"background-color:#66FFCC\" /><td style=\"background-color:#66FFFF\" /><td style=\"background-color:#00FFFF\" /><td style=\"background-color:#00FFCC\" />"
			L"<td style=\"background-color:#00FF99\" /><td style=\"background-color:#00FF66\" />"
			L"<td style=\"background-color:#00FF33\" /><td style=\"background-color:#00FF00\" /></tr>"
			L"<tr><td style=\"background-color:#CCCCFF\" /><td style=\"background-color:#CCCCCC\" /><td style=\"background-color:#CCCC99\" /><td style=\"background-color:#CCCC66\" /><td style=\"background-color:#CCCC33\" /><td style=\"background-color:#CCCC00\" /><td style=\"background-color:#66CC00\" />"
			L"<td style=\"background-color:#66CC33\" /><td style=\"background-color:#66CC66\" /><td style=\"background-color:#66CC99\" /><td style=\"background-color:#66CCCC\" /><td style=\"background-color:#66CCFF\" /><td style=\"background-color:#00CCFF\" /><td style=\"background-color:#00CCCC\" />"
			L"<td style=\"background-color:#00CC99\" /><td style=\"background-color:#00CC66\" /><td style=\"background-color:#00CC33\" /></tr>"
			L"<tr><td style=\"background-color:#00CC00\" /><td style=\"background-color:#CC99FF\" /><td style=\"background-color:#CC99CC\" /><td style=\"background-color:#CC9999\" /><td style=\"background-color:#CC9966\" /><td style=\"background-color:#CC9933\" /><td style=\"background-color:#CC9900\" />"
			L"<td style=\"background-color:#669900\" /><td style=\"background-color:#669933\" /><td style=\"background-color:#669966\" /><td style=\"background-color:#669999\" /><td style=\"background-color:#6699CC\" /><td style=\"background-color:#6699FF\" /><td style=\"background-color:#0099FF\" />"
			L"<td style=\"background-color:#0099CC\" /><td style=\"background-color:#009999\" /><td style=\"background-color:#009966\" /></tr>"
			L"<tr><td style=\"background-color:#009933\" /><td style=\"background-color:#009900\" /><td style=\"background-color:#CC66FF\" /><td style=\"background-color:#CC66CC\" /><td style=\"background-color:#CC6699\" /><td style=\"background-color:#CC6666\" /><td style=\"background-color:#CC6633\" />"
			L"<td style=\"background-color:#CC6600\" /><td style=\"background-color:#666600\" /><td style=\"background-color:#666633\" /><td style=\"background-color:#666666\" /><td style=\"background-color:#666699\" /><td style=\"background-color:#6666CC\" /><td style=\"background-color:#6666FF\" />"
			L"<td style=\"background-color:#0066FF\" /><td style=\"background-color:#0066CC\" /><td style=\"background-color:#006699\" /></tr>"
			L"<tr><td style=\"background-color:#006666\" /><td style=\"background-color:#006633\" /><td style=\"background-color:#006600\" /><td style=\"background-color:#CC33FF\" /><td style=\"background-color:#CC33CC\" /><td style=\"background-color:#CC3399\" /><td style=\"background-color:#CC3366\" />"
			L"<td style=\"background-color:#CC3333\" /><td style=\"background-color:#CC3300\" /><td style=\"background-color:#663300\" /><td style=\"background-color:#663333\" /><td style=\"background-color:#663366\" /><td style=\"background-color:#663399\" /><td style=\"background-color:#6633CC\" />"
			L"<td style=\"background-color:#6633FF\" /><td style=\"background-color:#0033FF\" /><td style=\"background-color:#0033CC\" /></tr>"
			L"<tr><td style=\"background-color:#003399\" /><td style=\"background-color:#003366\" /><td style=\"background-color:#003333\" /><td style=\"background-color:#003300\" /><td style=\"background-color:#CC00FF\" /><td style=\"background-color:#CC00CC\" /><td style=\"background-color:#CC0099\" />"
			L"<td style=\"background-color:#CC0066\" /><td style=\"background-color:#CC0033\" /><td style=\"background-color:#CC0000\" /><td style=\"background-color:#660000\" /><td style=\"background-color:#660033\" /><td style=\"background-color:#660066\" /><td style=\"background-color:#660099\" />"
			L"<td style=\"background-color:#6600CC\" /><td style=\"background-color:#6600FF\" /><td style=\"background-color:#0000FF\" /></tr>"
			L"<tr><td style=\"background-color:#0000CC\" /><td style=\"background-color:#000099\" /><td style=\"background-color:#000066\" /><td style=\"background-color:#000033\" /><td style=\"background-color:#000000\" /><td style=\"background-color:#FF00FF\" /><td style=\"background-color:#FF00CC\" />"
			L"<td style=\"background-color:#FF0099\" /><td style=\"background-color:#FF0066\" /><td style=\"background-color:#FF0033\" /><td style=\"background-color:#FF0000\" /><td style=\"background-color:#990000\" /><td style=\"background-color:#990033\" /><td style=\"background-color:#990066\" />"
			L"<td style=\"background-color:#990099\" /><td style=\"background-color:#9900CC\" /><td style=\"background-color:#9900FF\" /></tr>"
			L"<tr><td style=\"background-color:#3300FF\" /><td style=\"background-color:#3300CC\" /><td style=\"background-color:#330099\" /><td style=\"background-color:#330066\" /><td style=\"background-color:#330033\" /><td style=\"background-color:#330000\" /><td style=\"background-color:#FF33FF\" />"
			L"<td style=\"background-color:#FF33CC\" /><td style=\"background-color:#FF3399\" /><td style=\"background-color:#FF3366\" /><td style=\"background-color:#FF3333\" /><td style=\"background-color:#FF3300\" /><td style=\"background-color:#993300\" /><td style=\"background-color:#993333\" />"
			L"<td style=\"background-color:#993366\" /><td style=\"background-color:#993399\" /><td style=\"background-color:#9933CC\" /></tr>"
			L"<tr><td style=\"background-color:#9933FF\" /><td style=\"background-color:#3333FF\" /><td style=\"background-color:#3333CC\" /><td style=\"background-color:#333399\" /><td style=\"background-color:#333366\" /><td style=\"background-color:#333333\" /><td style=\"background-color:#333300\" />"
			L"<td style=\"background-color:#FF66FF\" /><td style=\"background-color:#FF66CC\" /><td style=\"background-color:#FF6699\" /><td style=\"background-color:#FF6666\" /><td style=\"background-color:#FF6633\" /><td style=\"background-color:#FF6600\" /><td style=\"background-color:#996600\" />"
			L"<td style=\"background-color:#996633\" /><td style=\"background-color:#996666\" /><td style=\"background-color:#996699\" /></tr>"
			L"<tr><td style=\"background-color:#9966CC\" /><td style=\"background-color:#9966FF\" /><td style=\"background-color:#3366FF\" /><td style=\"background-color:#3366CC\" /><td style=\"background-color:#336699\" /><td style=\"background-color:#336666\" /><td style=\"background-color:#336633\" />"
			L"<td style=\"background-color:#336600\" /><td style=\"background-color:#FF99FF\" /><td style=\"background-color:#FF99CC\" /><td style=\"background-color:#FF9999\" /><td style=\"background-color:#FF9966\" /><td style=\"background-color:#FF9933\" /><td style=\"background-color:#FF9900\" />"
			L"<td style=\"background-color:#999900\" /><td style=\"background-color:#999933\" /><td style=\"background-color:#999966\" /></tr>"
			L"<tr><td style=\"background-color:#999999\" /><td style=\"background-color:#9999CC\" /><td style=\"background-color:#9999FF\" /><td style=\"background-color:#3399FF\" /><td style=\"background-color:#3399CC\" /><td style=\"background-color:#339999\" /><td style=\"background-color:#339966\" />"
			L"<td style=\"background-color:#339933\" /><td style=\"background-color:#339900\" /><td style=\"background-color:#FFCCFF\" /><td style=\"background-color:#FFCCCC\" /><td style=\"background-color:#FFCC99\" /><td style=\"background-color:#FFCC66\" /><td style=\"background-color:#FFCC33\" />"
			L"<td style=\"background-color:#FFCC00\" /><td style=\"background-color:#99CC00\" /><td style=\"background-color:#99CC33\" /></tr>"
			L"<tr><td style=\"background-color:#99CC66\" /><td style=\"background-color:#99CC99\" /><td style=\"background-color:#99CCCC\" /><td style=\"background-color:#99CCFF\" /><td style=\"background-color:#33CCFF\" /><td style=\"background-color:#33CCCC\" /><td style=\"background-color:#33CC99\" />"
			L"<td style=\"background-color:#33CC66\" /><td style=\"background-color:#33CC33\" /><td style=\"background-color:#33CC00\" /><td style=\"background-color:#FFFFFF\" />"
			L"<td style=\"background-color:#FFFFCC\" /><td style=\"background-color:#FFFF99\" /><td style=\"background-color:#FFFF66\" /><td style=\"background-color:#FFFF33\" /><td style=\"background-color:#FFFF00\" /><td style=\"background-color:#99FF00\" /></tr>"
			L"<tr><td style=\"background-color:#99FF33\" /><td style=\"background-color:#99FF66\" /><td style=\"background-color:#99FF99\" /><td style=\"background-color:#99FFCC\" /><td style=\"background-color:#99FFFF\" /><td style=\"background-color:#33FFFF\" /><td style=\"background-color:#33FFCC\" />"
			L"<td style=\"background-color:#33FF99\" /><td style=\"background-color:#33FF66\" /><td style=\"background-color:#33FF33\" /><td style=\"background-color:#33FF00\" /><td style=\"background-color:#000000\" /><td style=\"background-color:#333333\" /><td style=\"background-color:#666666\" />"
			L"<td style=\"background-color:#999999\" /><td style=\"background-color:#CCCCCC\" /><td style=\"background-color:#FFFFFF\" /></tr>";
		dom::element eTable = dom::element::create("table");
		ePopup.append(eTable);
		utf8::ostream o;
		o<<sTable;
		eTable.set_html(o.data(),o.length());
	}
	virtual void attached  (HELEMENT he ) 
	{ 
		dom::element el = he;
		dom::element ePopup = dom::element::create("popup");
		el.append(ePopup);
		init_colors(ePopup);
	}
	
	virtual BOOL on_mouse(HELEMENT he, HELEMENT target, UINT event_type, 
		POINT pt, UINT mouseButtons, UINT keyboardStates )
	{
		switch (event_type)
		{
		case MOUSE_DOWN:
			{
				dom::element er = he;
				HELEMENT hPopup = er.find_first("popup");
				if( dom::element(hPopup).is_valid() && er.is_valid() && !er.get_state(STATE_OWNS_POPUP) )
				{
					::HTMLayoutShowPopup(hPopup,er,2); // show it below
				}
				return true;
			}
		}
		return false;
	}
	virtual BOOL on_event(HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason )
	{
		if (type == POPUP_DISMISSED)
		{
			dom::element eTop = he;
			dom::element eCheck = eTop.find_first("popup>table td:checked");
			if (eCheck.is_valid())
			{
                DWORD dwRet = 0;
                LPCWSTR cr = eCheck.get_style_attribute("background-color");
                if (NULL != cr)
                {
                    CStdString sC;
                    int n = StrToInt(cr);
                    sC.Format(L"#%02X%02X%02X", GetRValue(n), GetGValue(n), GetBValue(n));
                    eTop.set_style_attribute("background-color",sC);
                    eTop.send_event(SELECT_SELECTION_CHANGED);
                }
			}
			return TRUE;
		}
		return FALSE;
	}
};

// instantiating and attaching it to the global list

colorpicker xcolorpicker_instance;
} // htmlayout namespace
