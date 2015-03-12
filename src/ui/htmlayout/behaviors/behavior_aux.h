#ifndef __behavior_aux__
#define __behavior_aux__

#pragma warning( push )
#pragma warning(disable:4786) //identifier was truncated...

#include "../htmlayout.h"
#include "../htmlayout_dom.h"
#include "../htmlayout_behavior.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "notifications.h"

inline int width( const RECT& rc ) { return rc.right - rc.left; }
inline int height( const RECT& rc ) { return rc.bottom - rc.top; }

namespace htmlayout
{

/** 根据itemtex选中组合框的项
 *	return:
 *      true    找到此item并且选中
 *	params:
 *		-[in]
 *          edropdown      dropdown ctrl
 *          stext          item text      
**/
inline bool dropdown_selectitem_by_itemtext(dom::element& edropdown, json::string& stext)
{
    using namespace htmlayout::dom;

    element epopup = edropdown.find_first("popup");
    if (!epopup.is_valid() || !wcslen(stext.c_str()))
        return false;

    // find item
    int item_count = epopup.children_count();
    for (int i = 0; i < item_count; i++)
    {
        element eopt = epopup.child(i);
        if (eopt.text() != stext)
            continue;

        // current item
        element ecur_opt = epopup.find_first("option:checked");
        if (ecur_opt.is_valid())
        {
            ecur_opt.toggle_state(STATE_CHECKED,false);
            ecur_opt.toggle_state(STATE_CURRENT,true);
            ecur_opt.remove_attribute("selected");
        }
        // set current
        eopt.toggle_state(STATE_CHECKED,true);
        eopt.toggle_state(STATE_CURRENT,true);
        eopt.set_attribute("selected",L"1");
        return true;
    }
    return false;
}

};


#pragma warning( pop )

#endif


