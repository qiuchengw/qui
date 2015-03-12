#include "behavior_aux.h"

namespace htmlayout 
{

/*
    BEHAVIOR: scroll-indicator.
    flow:hidden;
    goal: 悬浮在content上的滚动条
    TYPICAL USE CASE:
    <div .scroll-indicator-box vscroll=1>
        <div .content-box />
    </div>
*/
    void set_slider_dragging(dom::element& el, bool bdragging)
    {
        if (bdragging)
            el.set_attribute("is_dragging", L"1");
        else
            el.remove_attribute("is_dragging");
    }

    bool is_slider_dragging(dom::element& el)
    {
        return el.get_attribute("is_dragging") != NULL;
    }

    struct vscroll_indicator_slider: public behavior
    {
        vscroll_indicator_slider()
            : behavior(HANDLE_MOUSE|HANDLE_INITIALIZATION, "vscroll-indicator-slider") 
        {

        }

        void move_slider(dom::element eslider, POINT pt)
        {
            // pt 是在slider里面的位置
            dom::element ebar = eslider.parent();  // parent
            RECT rc_bar = ebar.get_location();
            RECT rc_slider = eslider.get_location();

            int nY = rc_slider.top - rc_bar.top + pt.y;
            //- rc_bar.top ;
            if ((nY >= 0) && (rc_slider.bottom <= rc_bar.bottom))
            {
                eslider.set_style_attribute("top",aux::itow(nY));
                // 此处滚动content
                dom::element econt = dom::element(ebar.parent()).find_first(".content-box");
                if (econt.is_valid())
                {
                    POINT pt;
                    pt.x = 0;
                    pt.y = nY;
                    econt.set_scroll_pos(pt);
                }
            }
        }

        virtual BOOL on_mouse(HELEMENT he, HELEMENT target, UINT event_type,
            POINT pt, UINT mouseButtons, UINT keyboardStates )
        {
            if( mouseButtons != MAIN_MOUSE_BUTTON )
                return false;

            dom::element el = he;
            switch (event_type)
            {
            case MOUSE_DOWN:
                el.set_capture();
                set_slider_dragging(el, true);
                return true;

            case MOUSE_UP:
                el.release_capture();
                set_slider_dragging(el, false);
                return true;

            case MOUSE_MOVE:
                move_slider(el, pt);
                return true;
            }
            return false;
        }
    }vscroll_indicator_slider_instance;

    struct vscroll_indicator_bar: public behavior
    {
        // ctor
        vscroll_indicator_bar() 
            : behavior(HANDLE_MOUSE | HANDLE_INITIALIZATION | HANDLE_METHOD_CALL, "vscroll-indicator-bar") {}

        virtual void attached(HELEMENT he)
        {
            dom::element ebar = he;
            dom::element slider = slider_el(ebar);
            if (!slider.is_valid())
            {
                slider = ebar.create("div");
                ebar.append(slider);
                slider.set_attribute("class", L"slider");
            }
        }

        dom::element slider_el(dom::element bar, bool attached_event = true)
        {
            dom::element slider = bar.find_first(".slider");
            if (attached_event && slider.is_valid())
            {
                slider.attach(&vscroll_indicator_slider_instance);
            }
            return slider;
        }

        void recalc_layout(dom::element ebar)
        {
            dom::element ebox = ebar.parent();
            dom::element econt = ebox.find_first(".content-box");
            if (!econt.is_valid())
                return;

            RECT rc_bar = ebar.get_location();
            RECT rc_box = ebox.get_location();
            int w = 0,nh_cont = 0;
            econt.get_intrinsic_height(w, nh_cont);
            // slider 应该多大？
            double nh_slider = double(rc_bar.bottom - rc_bar.top) / ((double) nh_cont / rc_box.bottom);
#pragma warning(disable:4244)
            slider_el(ebox).set_style_attribute("height",
                aux::itow((int)(nh_slider < 10) ? 10 : nh_slider));
#pragma warning(default:4244)
            ebar.update(false);
        }

        void scroll_down(dom::element& ebar)
        {

        }

        void scroll_up(dom::element& ebar)
        {

        }

        virtual BOOL on_script_call(HELEMENT he, LPCSTR name, UINT argc, 
            json::value* argv, json::value& /*retval*/)
        {
            dom::element ebar = he;
            if (aux::streq(name, "recalc"))
            {
                recalc_layout(ebar);
            }
            else if (aux::streq(name, "scrolldown"))
            {
                scroll_down(ebar);
            }
            else if (aux::streq(name, "scrollup"))
            {
                scroll_up(ebar);
            }
            return true;
        }

        void move_slider(dom::element eslider, POINT pt)
        {
            // pt 是在slider里面的位置
            dom::element ebar = eslider.parent();  // parent
            RECT rc_bar = ebar.get_location();
            RECT rc_slider = eslider.get_location();

            int nY = rc_slider.top + pt.y; //) + rc_slider.top;
            //- rc_bar.top ;
            if ((nY >= 0) && ((nY + rc_slider.bottom - rc_slider.top) <= rc_bar.bottom))
            {
                eslider.set_style_attribute("top",aux::itow(nY));
                // 此处滚动content
                dom::element econt = dom::element(ebar.parent()).find_first(".content-box");
                if (econt.is_valid())
                {
                    POINT pt;
                    pt.x = 0;
                    pt.y = nY;
                    econt.set_scroll_pos(pt);
                }
            }
        }

        virtual BOOL on_mouse(HELEMENT he, HELEMENT target, UINT event_type,
            POINT pt, UINT mouseButtons, UINT keyboardStates )
        {
            if( mouseButtons != MAIN_MOUSE_BUTTON )
                return false;

            switch (event_type)
            {
            case MOUSE_DOWN:
                {
                    break;
                }
            }
            return false; // it is ours - stop event bubbling
        }
    }vscroll_indicator_bar_instance;

    struct scroll_indicator_box: public behavior
    {
        scroll_indicator_box()
            : behavior(HANDLE_MOUSE|HANDLE_INITIALIZATION, "scroll-indicator-box") 
        {
            
        }

        virtual void attached(HELEMENT he)
        {
            dom::element el = he;
            if (el.get_attribute("vscroll") != NULL)
            {
                dom::element bar = el.create("div");
                el.append(bar);
                bar.set_attribute("class", L"vindicator-bar");
            }
        }

        dom::element vscroll_bar(dom::element box)
        {
            dom::element ebar = box.find_first(".vindicator-bar");
            if (ebar.is_valid())
            {
                ebar.attach(&vscroll_indicator_bar_instance);
            }
            return ebar;
        }

        virtual BOOL on_mouse(HELEMENT he, HELEMENT target, UINT event_type,
            POINT pt, UINT mouseButtons, UINT keyboardStates )
        {
            dom::element vbar = vscroll_bar(he);
            switch (event_type)
            {
            case MOUSE_ENTER:
                {
                    // 重新计算scrollbar
                    if (vbar.is_valid())
                    {
                        vbar.xcall("recalc");    // 重新计算
                    }
                    break;
                }
            case MOUSE_LEAVE:
                {
                    dom::element(he).update();
                    break;
                }
            case MOUSE_WHEEL:
                {

                    break;
                }
            }
            return false;
        }
    }scroll_indicator_box_instance;
}

