#include "behavior_aux.h"

namespace htmlayout 
{

	/** behavior:scroller, 
	* another way of scrolling  
	**/

	struct scroller: public behavior
	{
		// ctor
		scroller(const char* name = "scroller"): behavior(HANDLE_MOUSE, name) {}

		virtual void attached  (HELEMENT he ) 
		{ 
		} 

		// 处理[show_checkeditem_onmouseleave]属性
		// 当mouse_leave时滚动到checked的item
		void handle_mouseleave(HELEMENT he)
		{
			dom::element ep = he;
			if (ep.get_attribute("show_checkeditem_onmouseleave") == nullptr)
				return;
			// 找到checked的元素
			dom::element ei = ep.find_first(":checked");
			if (!ei.is_valid())
				return;

			POINT scroll_pos;
			RECT  view_rect; 
			SIZE content_size;
			ep.get_scroll_info(scroll_pos, view_rect, content_size);

			int vh = view_rect.bottom - view_rect.top;
			if (content_size.cy <= vh)
				return;

			// 选中的item的top在content_view中的y坐标
			int py = ei.get_location(false).top	// 选中的item的top
				- dom::element(ep.child(0)).get_location(false).top; // 第一个item的top
			if (py + vh > content_size.cy)
			{
				py = content_size.cy - vh;
			}
			if (py != scroll_pos.y)
			{
				scroll_pos.y = py;
				// 显示之
				ep.set_scroll_pos(scroll_pos, true);
			}
		}

		virtual BOOL on_mouse(HELEMENT he, HELEMENT target, UINT event_type, POINT pt, UINT mouseButtons, UINT keyboardStates )
		{
			if (mouseButtons != 0)
				return false;
			if (MOUSE_LEAVE == event_type)
			{
				handle_mouseleave(he);
				return true;
			}
			if (event_type != (MOUSE_MOVE | SINKING))
				return false;

			POINT scroll_pos;
			RECT  view_rect; SIZE content_size;

			dom::element el = he;
			el.get_scroll_info(scroll_pos, view_rect, content_size);

			if( pt.x < view_rect.left || pt.x > view_rect.right || 
				pt.y < view_rect.top || pt.y > view_rect.bottom )
				return false;

			int vh = view_rect.bottom - view_rect.top;
			if( vh >= content_size.cy  )
			{
				if (scroll_pos.y > 0)
				{
					scroll_pos.y = 0;
					el.set_scroll_pos(scroll_pos,false);
				}
				return false;
			}

			int dh = content_size.cy - vh;
			int py = ( pt.y * dh) / vh;
			if( py != scroll_pos.y )
			{
				scroll_pos.y = py;
				el.set_scroll_pos(scroll_pos, true);
			}
			return false; 
		}


	};

	/** behavior:xscroller, 
	* another way of scrolling  
	**/

	struct xscroller: public behavior
	{
		// ctor
		xscroller(const char* name = "xscroller"): behavior(HANDLE_MOUSE, name) {}

		virtual void attached  (HELEMENT he ) 
		{ 
		} 

		// 处理[show_checkeditem_onmouseleave]属性
		// 当mouse_leave时滚动到checked的item
		void handle_mouseleave(HELEMENT he)
		{
			dom::element ep = he;
			if (!aux::wcseqi(ep.get_attribute("show_checkeditem_onmouseleave"),L"true"))
				return;
			// 找到checked的元素
			dom::element ei = ep.find_first(":checked");
			if (!ei.is_valid())
				return;

//            ei.scroll_to_view(true, true);

			POINT scroll_pos;
			RECT  view_rect; 
			SIZE content_size;
			ep.get_scroll_info(scroll_pos, view_rect, content_size);

			int px = ei.get_location(false).left	// 选中的item的top
				- dom::element(ep.child(0)).get_location(false).left; // 第一个item的top
			if (px != scroll_pos.x)
			{
				scroll_pos.x = px;
				// 显示之
				ep.set_scroll_pos(scroll_pos, true);
			}
		}

		virtual BOOL on_mouse(HELEMENT he, HELEMENT target, UINT event_type, 
            POINT pt, UINT mouseButtons, UINT keyboardStates )
		{
			if (mouseButtons != 0)
				return false;
			if (MOUSE_LEAVE == event_type)
			{
				handle_mouseleave(he);
				return false;
			}
			if (event_type != MOUSE_MOVE)
				return false;

			POINT scroll_pos;
			RECT  view_rect; SIZE content_size;

			dom::element el = he;
			el.get_scroll_info(scroll_pos, view_rect, content_size);

			if( pt.x < view_rect.left || pt.x > view_rect.right || 
				pt.y < view_rect.top || pt.y > view_rect.bottom )
				return false;

			int vw = view_rect.right - view_rect.left;
			if( vw >= content_size.cx  )
			{
				if (scroll_pos.x > 0)
				{
					scroll_pos.x = 0;
					el.set_scroll_pos(scroll_pos,true);
				}
				return false;
			}

			int dw = content_size.cx - vw;

			int px = ( pt.x * dw) / vw;
			if( px != scroll_pos.x )
			{
				scroll_pos.x = px;
				el.set_scroll_pos(scroll_pos, true);
			}
			return false; 
		}


	};

	xscroller          xscroller_instance;
	scroller          scroller_instance;


}