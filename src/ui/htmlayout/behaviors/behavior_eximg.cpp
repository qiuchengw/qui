#include "behavior_aux.h"
#include "ui/ECtrl.h"
#include <atlmisc.h>


/*
BEHAVIOR: eximg

TYPICAL USE CASE:
<widget type="text-image" src="d:\test.png">学习吉他</widget>
*/

namespace htmlayout 
{
    using namespace dom;

	struct eximage: public behavior
	{
		// ctor
		eximage(): behavior(0, "text-image") { }

		virtual void attached  (HELEMENT he ) 
		{
			ECtrl el(he);
			const wchar_t* src = el.get_attribute("src");
			if (NULL != src)
			{
                el.SetFrgndImage(src);
			}
			dom::element ec = dom::element::create("div",el.text());
			el.clear();
			el.append(ec);
		}

	};

	// instantiating and attaching it to the global list
	eximage eximg_instance;

    struct hover_div: public behavior
    {
        // ctor
        hover_div(): behavior(HANDLE_METHOD_CALL, "hover-div") { }

        void adjust_size(ECtrl&el, ECtrl& ediv, int& w, int& h)
        {
            RECT rc_el = el.get_location();
            if (ediv.IsHasAttribute("adjust-width"))
            {
                w = rc_el.right - rc_el.left;
            }
            else
            {
                int wmin = 0;
                ediv.get_intrinsic_widths(wmin, w);
            }

            if (ediv.IsHasAttribute("ajust-height"))
            {
                h = rc_el.bottom - rc_el.top;
            }
            else
            {
                ediv.get_intrinsic_height(w, h);
            }

            ediv.SetSize(w, h);
        }

        void adjust_pos(ECtrl&el, ECtrl& ediv, int w, int h)
        {
//             wchar_t szB[20] = {0};
//             RECT rc = el.get_location();
//             int w1 = rc.right - rc.left;
//             int h1 = rc.bottom - rc.top;

            switch(el.get_attribute_int("div-pos"))
            {
            case 0:case 3:  // 默认放在右下角
                //left:0;right:0;bottom:0px;
                ediv.set_style_attribute("right",L"0");
                ediv.set_style_attribute("bottom",L"0");
                break;

            case 7: // 左上
                ediv.set_style_attribute("left",L"0");
                ediv.set_style_attribute("top",L"0");
                break;

            case 9: // 右上
                ediv.set_style_attribute("right",L"0");
                ediv.set_style_attribute("top",L"0");
                break;

            case 1: // 左下
                ediv.set_style_attribute("left",L"0");
                ediv.set_style_attribute("bottom",L"0");
                break;

            case 5: // 正中
            default:
//                 p.x = (w1 - w2) / 2 + rc_el.left;
//                 p.y = (h1 - h2) / 2 + rc_el.top;
                ASSERT(FALSE); // 其他位置暂时没实现，用到再实现
                break;
            }
        }

        virtual void attached  (HELEMENT he ) 
        {
            ECtrl el(he);
            ECtrl ed = el.find_first("div[div-hover]");
            ASSERT(ed.is_valid());
            if (ed.is_valid())
            {
                int w, h;
                adjust_size(el, ed ,w ,h);

                adjust_pos(el, ed , w, h);
            }
        }

        virtual BOOL on_script_call(HELEMENT he, LPCSTR name, UINT argc, 
            json::value* argv, json::value& retval) 
        {
            if (aux::streqi("mouse-enter", name))
            {
                ECtrl el(he);
                json::astring sss = el.get_html(1);
                return TRUE;
            }
            return FALSE;
        }
    };

    // instantiating and attaching it to the global list
    hover_div hover_div_instance;

    struct hover_widget: public behavior
    {
        // ctor
        hover_widget(): behavior(HANDLE_METHOD_CALL|HANDLE_INITIALIZATION, "hover-widget") { }

        virtual void attached  (HELEMENT he ) 
        {
            dom::element el(he);
            el.set_attribute("attach_behavior", L"hover-widget");
        }

        bool get_attached_widget(__in ECtrl& el,
            __out ECtrl& wdt, __out POINT& p)
        {
            ECtrl r = el.root();
            const wchar_t *sel = el.get_attribute("widget-selector");
            if (nullptr == sel)
            {
                sel = el.attribute("-widget-selector",nullptr);
            }
            if (nullptr == sel)
                return false;

            wdt = r.find_first((const char*)aux::w2a(sel));
            if (!wdt.is_valid())
                return false;

            // 元素滚动到全部可见位置
            el.scroll_to_view();

            // 调整popup大小
            RECT rc_el = el.get_location();
            if (wdt.IsHasAttribute("adjust-width"))
            {
                wdt.SetWidth(rc_el.right - rc_el.left);
            }
            if (wdt.IsHasAttribute("ajust-height"))
            {
                wdt.SetHeight(rc_el.bottom - rc_el.top);
            }

            int w1 = rc_el.right - rc_el.left;
            int h1 = rc_el.bottom - rc_el.top;

            // popup应该显示到的位置
            RECT rc_wdt = wdt.get_location(ROOT_RELATIVE|BORDER_BOX);
            int w2 = rc_wdt.right - rc_wdt.left;
            int h2 = rc_wdt.bottom - rc_wdt.top;
            switch(el.get_attribute_int("popup-pos"))
            {
            case 0:case 3:  // 默认放在右下角
                p.x = rc_el.right - w2;
                p.y = rc_el.bottom - h2;
                break;

            case 7: // 左上
                p.x = rc_el.left;
                p.y = rc_el.top;
                break;

            case 9: // 右上
                p.y = rc_el.top;
                p.x = rc_el.right - w2;
                break;

            case 1: // 左下
                p.x = rc_el.left;
                p.y = rc_el.bottom - h2;
                break;

            case 5: // 正中
                p.x = (w1 - w2) / 2 + rc_el.left;
                p.y = (h1 - h2) / 2 + rc_el.top;
                break;
            }
            return true;
        }

        inline bool is_cursor_in_el(ECtrl &el)
        {
            CRect rc = el.get_location();
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(el.get_element_hwnd(true), &pt);
            return rc.PtInRect(pt);
        }

        inline void show_popup_widget(ECtrl& wdt, ECtrl &el, POINT &p, bool bShow)
        {
            HELEMENT he = (HELEMENT)el;
            if (bShow)
            {
                if (((HELEMENT)wdt.GetData() != he) || !wdt.IsHasAttribute("showing"))
                {
                    wdt.show_popup(p, true);
                    wdt.SetData(he);
                    wdt.set_attribute("showing",L"1");
                }
            }
            else
            {
                wdt.hide_popup();
                wdt.remove_attribute("showing");
                wdt.RemoveData();
            }
        }

        virtual BOOL on_script_call(HELEMENT he, LPCSTR name, UINT argc, 
            json::value* argv, json::value& retval) 
        { 
            using namespace htmlayout::dom;
            if (aux::streqi("mouse-enter", name))
            {
                POINT p;
                ECtrl wdt;
                ECtrl el(he);
                if (el.IsHasAttribute("popup-not-ready"))
                    return TRUE; // 不要显示
                if (get_attached_widget(el, wdt, p))
                {
                    show_popup_widget(wdt, el, p, true);
                }
                return TRUE;
            }
            else if (aux::streqi("mouse-leave", name))
            {
                ECtrl wdt;
                ECtrl el(he);
                POINT p;
                if (get_attached_widget(el, wdt, p))
                {
                    // 鼠标移动到popup上也会产生这个消息
                    // 这种情况下并不想隐藏弹出窗口
                    show_popup_widget(wdt, el, p, is_cursor_in_el(el));
                }
                return TRUE;
            }
            return FALSE; 
        }
    };

    // instantiating and attaching it to the global list
    hover_widget hover_widget_instance;

} // htmlayout namespace


