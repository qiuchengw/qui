#include "behavior_aux.h"
#include "ui/ECtrl.h"

namespace htmlayout 
{

    /** behavior:grid, browser of tabular data (records).
    *  Supports multiple selection mode. Use "multiple" attribute.
    *
    *   model is like this:
    *   <table fixedrows=1 ... style="behavior:grid"> 
    *     <tr>...
    *     <tr>...
    *   </table> 
    *
    *  LOGICAL EVENTS: 
    *    TABLE_HEADER_CLICK, click on some cell in table header, target is the cell
    *    TABLE_ROW_CLICK,    click on data row in the table, target is the row
    *
    *  See: html_samples/grid/scrollable-table.htm
    *  Authors: Andrew Fedoniouk, initial implementation.
    *           Andrey Kubyshev, multiselection mode.
    *
    **/

    struct grid: public behavior
    {
        // ctor
        grid(const char* name = "grid", int additional_flags = 0)
            :behavior(HANDLE_MOUSE | HANDLE_KEY | HANDLE_FOCUS |HANDLE_METHOD_CALL| additional_flags, name) {}

        virtual void attached  (HELEMENT he ) 
        { 
#ifdef _DEBUG
            dom::element table = he;
            assert( aux::streq(table.get_element_type(), "table") ); // must be table.
#endif
            //test: table.set_attribute( "width", L"32px");
        } 

        /** is it multiple selectable? **/
        bool is_multiple (const dom::element& table)
        {
            return table.get_attribute ("multiple") != 0;
        }

        /** Click on column header (fixed row).
        Overrided in sortable-grid **/
        virtual void on_column_click( dom::element& table, dom::element& header_cell )
        {
            table.post_event( TABLE_HEADER_CLICK, header_cell.index(), header_cell); 
        }

        /** returns current row (if any) **/
        dom::element get_current_row( dom::element& table )
        {
            for( int i = table.children_count() - 1; i >= 0 ; --i)
            {
                dom::element t = table.child((unsigned int)i);
                if( t.get_state(STATE_CURRENT))
                    return t;
            }
            return dom::element(); // empty
        }

        /** set current row **/
        void set_current_row( dom::element& table, dom::element& row, UINT keyboardStates, bool dblClick = false, bool smooth = false )
        {
            // get previously selected row:
            dom::element prev = get_current_row( table );

            uint new_row_checked = STATE_CHECKED;
            uint old_row_checked = STATE_CHECKED;

            if(is_multiple(table))
            {
                if (keyboardStates & SHIFT_KEY_PRESSED)
                {
                    checkall(table, false);
                    check_range(table,row.index(),TRUE); // from current to new
                } 
                else 
                {      
                    if (keyboardStates & CONTROL_KEY_PRESSED)
                    {
                        set_checked_row (table,row, true); // toggle
                        new_row_checked = 0;
                    }
                    else
                        checkall(table, false);
                    set_anchor(table,row.index ());
                }
                old_row_checked = 0;
            }

            if( prev.is_valid() )
            {
                if( prev != row ) 
                    prev.set_state(0,STATE_CURRENT | old_row_checked); // drop state flags
            }
            row.set_state(STATE_CURRENT | new_row_checked); // set state flags
            if (!row.visible())
            {
                row.scroll_to_view(false,smooth);
            }
            //::UpdateWindow(row.get_element_hwnd(false));
            table.post_event( dblClick? TABLE_ROW_DBL_CLICK:TABLE_ROW_CLICK, row.index(), row); 
        }

        dom::element target_row(const dom::element& table, const dom::element& target)
        {
            if( !target.is_valid() || target.parent() == table)
                return target;
            return target_row(table,target.parent());
        }

        dom::element target_header(const dom::element& header_row, const dom::element& target)
        {
            if( !target.is_valid() || target.parent() == header_row)
                return target;
            return target_header(header_row,target.parent());
        }

        int fixed_rows( const dom::element& table )
        {
            return table.get_attribute_int("fixedrows",0);
        }

        void set_checked_row( dom::element& table, dom::element& row, bool toggle = false )
        {
            if(toggle)
            {
                if( row.get_state( STATE_CHECKED) )
                    row.set_state( 0,STATE_CHECKED);
                else
                    row.set_state( STATE_CHECKED,0);
            }
            else
                row.set_state( STATE_CHECKED,0);
        }

        int get_anchor (const dom::element& table)
        {
            dom::element row = table.find_first("tr:anchor");
            if( row.is_valid() ) return (int)row.index();
            return 0;
        }

        void set_anchor (dom::element& table,const int idx)
        {
            dom::element row = table.find_first("tr:anchor");
            if( row.is_valid() ) row.set_state( 0,STATE_ANCHOR,false);
            row = table.child(idx);
            if( row.is_valid() )
                row.set_state( STATE_ANCHOR,0,false);
        }

        void check_range (const dom::element& table, int idx, bool check)
        {
            if (!is_multiple(table)) return;

            int   start_idx = get_anchor(table);
            int   start = min(start_idx,idx );
            int   end = max(start_idx,idx );

            int   f_rows  = fixed_rows(table);
            if(start < f_rows) start = f_rows;

            for( ;end >= start; --end ) 
            {
                dom::element row = table.child(end);
                if(!!row.visible())
                {
                    if (check) row.set_state(STATE_CHECKED,0);
                    else row.set_state(0,STATE_CHECKED);
                }
            }
        }
   
        virtual BOOL on_mouse(HELEMENT he, HELEMENT target, UINT event_type,
            POINT pt, UINT mouseButtons, UINT keyboardStates )
        {
            if( (event_type != (MOUSE_DOWN | SINKING)) && (event_type != (MOUSE_DCLICK|SINKING)) )
                return false;

            //if(mouseButtons != MAIN_MOUSE_BUTTON) 
            //  return false;

            // el must be table;
            dom::element table = he;
            dom::element row = target_row(table, target);

            if(row.is_valid()) // click on the row
            {
                if( (int)row.index() < (int)fixed_rows(table) )
                {
                    // click on the header cell
                    dom::element header_cell = target_header(row,target);
                    if( header_cell.is_valid() )  
                        on_column_click(table, header_cell);
                    return mouseButtons == MAIN_MOUSE_BUTTON;
                }

                if ( MAIN_MOUSE_BUTTON != mouseButtons )
                {
                    // 右键会自动选中/反选行，而不影响其它行
                    if ( PROP_MOUSE_BUTTON == mouseButtons)
                    {
                        if(is_multiple(table))
                        {
                            set_checked_row (table,row, false); // toggle
                        }
                        row.scroll_to_view(false,false);
                    }
                    return false;
                }
                else    // 左键按照资源管理器方式执行选中操作
                {
                    set_current_row(table, row, keyboardStates,  event_type == (MOUSE_DCLICK|SINKING));
                }
                HTMLayoutUpdateWindow(table.get_element_hwnd(false));
            }
            return FALSE;//mouseButtons == MAIN_MOUSE_BUTTON; // as it is always ours then stop event bubbling
        }

        //int firstVisibleRow( dom::element& table )

        virtual BOOL on_key(HELEMENT he, HELEMENT target, UINT event_type, UINT code, UINT keyboardStates ) 
        { 
            if( event_type == KEY_DOWN )
            {
                dom::element table = he;
                switch( code )
                {
                case VK_DOWN: 
                    {
                        dom::element c = get_current_row( table );
                        int idx = c.is_valid()? (c.index() + 1):fixed_rows(table);
                        while( idx < (int)table.children_count() )
                        {
                            dom::element row = table.child(idx);
                            if( !row.visible())
                            {
                                ++idx;
                                continue;
                            }
                            set_current_row(table, row, keyboardStates); 
                            break;
                        }
                    }
                    return TRUE;
                case VK_UP:             
                    {
                        dom::element c = get_current_row( table );
                        int idx = c.is_valid()? (c.index() - 1):(table.children_count() - 1);
                        while( idx >= fixed_rows(table) )
                        {
                            dom::element row = table.child(idx);
                            if( !row.visible())
                            {
                                --idx;
                                continue;
                            }
                            set_current_row(table, row, keyboardStates); 
                            break;
                        }
                    }
                    return TRUE;
                case VK_PRIOR:
                    {
                        RECT trc = table.get_location(ROOT_RELATIVE | SCROLLABLE_AREA);
                        int y = trc.top - (trc.bottom - trc.top);
                        int first = fixed_rows(table);
                        dom::element r;
                        for( int i = table.children_count() - 1; i >= first; --i )
                        {
                            dom::element nr = table.child(i);
                            if( aux::wcseq(nr.get_style_attribute("display"),L"none" ))
                                continue;
                            dom::element pr = r;
                            r = nr;
                            if( r.get_location(ROOT_RELATIVE | BORDER_BOX).top < y )
                            {
                                // row found
                                if(pr.is_valid()) r = pr; // to last fully visible
                                break;
                            }
                        }
                        set_current_row(table, r, keyboardStates, false, true); 
                    }
                    return TRUE;

                case VK_NEXT:
                    {
                        RECT trc = table.get_location(ROOT_RELATIVE | SCROLLABLE_AREA);
                        int y = trc.bottom + (trc.bottom - trc.top);
                        int last = table.children_count() - 1;
                        dom::element r; 
                        for( int i = fixed_rows(table); i <= last; ++i )
                        {
                            dom::element nr = table.child(i);
                            if( aux::wcseq(nr.get_style_attribute("display"),L"none" ))
                                continue;
                            dom::element pr = r;
                            r = nr;
                            if( r.get_location(ROOT_RELATIVE | BORDER_BOX).bottom > y )
                            {
                                // row found
                                if(pr.is_valid()) r = pr; // to last fully visible
                                break;
                            }
                        }
                        set_current_row(table, r, keyboardStates, false, true); 
                    }
                    return TRUE;

                case VK_HOME:
                    {
                        int idx = fixed_rows(table);
                        while( (int)idx < (int)table.children_count() )
                        {
                            dom::element row = table.child(idx);
                            if( !row.visible())
                            {
                                ++idx;
                                continue;
                            }
                            set_current_row(table, row, keyboardStates, false, true); 
                            break;
                        }
                    }
                    return TRUE;

                case VK_END:
                    {
                        int idx = table.children_count() - 1;
                        while( idx >= fixed_rows(table) )
                        {
                            dom::element row = table.child(idx);
                            if( !row.visible())
                            {
                                --idx;
                                continue;
                            }
                            set_current_row(table, row, keyboardStates, false, true); 
                            break;
                        }
                    }
                    return TRUE;
                case 'A':
                    if( is_multiple(table) && (keyboardStates & CONTROL_KEY_PRESSED) != 0 )
                    {
                        checkall(table, true);
                        return TRUE;
                    }
                    return FALSE;
                }
            }
            return FALSE; 
        }

        void checkall (dom::element& table, bool onOff )
        {
            if( !is_multiple(table) ) return;

            struct unchecker_cb: dom::callback 
            {
                bool on_element(HELEMENT he) 
                { 
                    htmlayout::dom::element el = he; if( el.get_state(STATE_CHECKED)) el.set_state(0,STATE_CHECKED ); return false; /*continue enumeration*/ 
                }
            };
            struct checker_cb: dom::callback 
            {
                bool on_element(HELEMENT he) 
                { 
                    htmlayout::dom::element el = he; if( !el.get_state(STATE_CHECKED)) el.set_state(STATE_CHECKED,0 ); return false; /*continue enumeration*/ 
                }
            };

            if(onOff) 
            {
                checker_cb checker;
                table.find_all(&checker,"tr");
            }
            else
            {
                unchecker_cb unchecker;
                table.find_all(&unchecker,"tr:checked");
            }
        }

    };

    struct treelist: public grid
    {
        void test()
        {
            OutputDebugString(L"-->hello world;\n");
        }

        treelist() : grid("treelist", HANDLE_METHOD_CALL|HANDLE_MOUSE|HANDLE_INITIALIZATION) {};

        virtual void attached(HELEMENT he)
        {
            dom::_qTLStructMan::GetInstance()->RegisterStruct(he);
        }

        virtual void detached(HELEMENT  he)
        {
            dom::_qTLStructMan::GetInstance()->UnregisterStruct(he);
        }

        virtual BOOL on_script_call(HELEMENT he, LPCSTR name, UINT argc, 
            json::value* argv, json::value& /*retval*/)
        {
            return true;
        }

        inline BOOL doubleclk_expand(dom::element &t)
        {
            return t.get_attribute("double_click_no_action") == NULL;
        }

        virtual BOOL handle_mouse  (HELEMENT he, MOUSE_PARAMS& params )
        {
            if(MAIN_MOUSE_BUTTON != params.button_state)
                return grid::handle_mouse(he, params);

            dom::element table = he;
            if (params.cmd == (MOUSE_DCLICK|SINKING))
            {   // 双击行
                // el must be table;
                if (!doubleclk_expand(table))
                    return grid::handle_mouse(he, params);

                dom::element row = target_row(table, params.target);
                if(row.is_valid() && ((int)row.index() >= (int)fixed_rows(table))) // click on the row
                {
                    // 双击行
                    ETreeList lst(table);
                    ETreeListItem r(row);
                    lst.IsItemExpand(r) ? lst.CollapseItem(r) : lst.ExpandItem(r);
                }
                return grid::handle_mouse(he, params);
            }

            if (params.cmd == (MOUSE_CLICK|SINKING))
            {
                if (params.is_on_icon)
                {
                    // 双击行
                    ETreeList lst(table);
                    ETreeListItem r(target_row(table, params.target));
                    lst.IsItemExpand(r) ? lst.CollapseItem(r) : lst.ExpandItem(r);
                    return TRUE;
                }
            }
            return grid::handle_mouse(he, params);
        }
    };

    struct sortable_grid: public grid
    {
        typedef grid super;

        // ctor
        sortable_grid(const char* name="sortable-grid",int additional_flags = 0)
            : super(name, additional_flags) {}

        virtual void on_column_click( dom::element& table, dom::element& header_cell )
        {
            super::on_column_click( table, header_cell );

            dom::element current = table.find_first("th:checked");
            if( current == header_cell )
                return; // already here, nothing to do.

            if( current.is_valid() )
                current.set_state(0, STATE_CHECKED);
            header_cell.set_state(STATE_CHECKED);

            dom::element ctr = get_current_row( table );
            sort_rows( table, header_cell.index() );
            if( ctr.is_valid() )
                ctr.scroll_to_view();

        }

        struct row_sorter: public dom::element::comparator
        {
            int column_no;

            row_sorter( int col_no ): column_no(col_no) {}

            virtual int compare(const htmlayout::dom::element& r1, const htmlayout::dom::element& r2)
            {
                if( !r1.is_valid() || !r2.is_valid() )
                    return 0;

                htmlayout::dom::element c1 = r1.child(column_no);
                htmlayout::dom::element c2 = r2.child(column_no);

                json::string t1 = c1.text();
                json::string t2 = c2.text();
                return wcscmp(t1,t2);
            }
        };

        void sort_rows( dom::element& table, int column_no )
        {
            row_sorter rs( column_no );

            int fr = fixed_rows( table );
            table.sort(rs,fr);
        }
    };

    struct sheet: public sortable_grid
    {
        typedef sortable_grid _base;

        sheet() : sortable_grid("sheet", HANDLE_METHOD_CALL|HANDLE_MOUSE|HANDLE_KEY) {};

        dom::element cell_widget(const dom::element& table, const dom::element& cell)
        {
            if (cell.get_attribute("no-cellwidget"))
                return NULL;

            if (fixed_rows(table) <= 0)
                return NULL;

            dom::element trHeader = table.child(0);
            if (!trHeader.is_valid())
                return NULL;

            dom::element th = trHeader.child(cell.index());
            if (!th.is_valid())
                return NULL;

            return th.find_first("[cell-widget]");
        }

        // bUpdate 是否更新cell-widget的值到cell上
        void close_cell_widget(dom::element& table, bool bUpdate=true)
        {
            using namespace htmlayout::dom;

            // 当前选中的td，cell
            // 不使用传过来的参数，我们自己找到当前选中的
            dom::element cell = (HELEMENT)table.get_attribute_int("sheet_cur_check_cell", 0);
            table.remove_attribute("sheet_cur_check_cell");
            if (!cell.is_alive())
                return;

            element wdt = cell.find_first("[cell-widget]");
            if (!wdt.is_valid())
                return ;

            // 更新到cell上
            if (bUpdate)
            {
                switch (wdt.get_ctl_type())
                {
                case CTL_EDIT:
                case CTL_TEXTAREA:
                case CTL_NUMERIC:
                    {
                        cell.set_text(EEdit(wdt).GetText());
                        break;
                    }
                case CTL_DATE:
                    {
                        SYSTEMTIME st;
                        if (FileTimeToSystemTime(&(wdt.get_value().get_date()), &st))
                        {
                            wchar_t szb[64] = {0};
                            swprintf_s(szb,64,L"%d/%d/%d",st.wYear,st.wMonth,st.wDay);
                            cell.set_text(szb);
                        }
                        break;
                    }
                case CTL_TIME:
                    {
                        SYSTEMTIME st;
                        if (FileTimeToSystemTime(&(wdt.get_value().get_date()), &st))
                        {
                            wchar_t szb[64] = {0};
                            swprintf_s(szb,64,L"%d:%d:%d",st.wHour,st.wMinute,st.wSecond);
                            cell.set_text(szb);
                        }
                        break;
                    }
                case CTL_DD_SELECT:
                    {
                        cell.set_text(wdt.get_value().to_string().c_str());
                        break;
                    }
                default:
                    {
                        cell.set_text(wdt.text().c_str());
                        break;
                    }
                }
            }
            else
            {
                const wchar_t* psz = cell.get_attribute("old_text");
                if (nullptr != psz)
                {
                    cell.set_text(psz);
                }
            }
            // wdt销毁
            wdt.destroy();

            // 发一个cell-widget关闭消息，应用程序可以响应此消息执行相应操作
            dom::element tbl = cell.select_parent(L"table",4);
            tbl.post_event(SHEET_CELLWIDGET_CLOSED, 0, cell);
        }

        void show_cell_widget(dom::element& table, const dom::element& cell)
        {
            using namespace htmlayout::dom;
            
            element wdt = cell.find_first("[cell-widget]");
            if (!wdt.is_valid())
                return ;

            table.set_attribute("sheet_cur_check_cell", aux::itow((int)(HELEMENT)cell));

            wdt.set_style_attribute("visibility",L"visible");
            wdt.toggle_state(STATE_FOCUS, true);

            json::string sv = cell.get_attribute("old_text");
            switch (wdt.get_ctl_type())
            {
            case CTL_EDIT:
            case CTL_TEXTAREA:
            case CTL_NUMERIC:
                {
                    EEdit eb = wdt;
                    eb.SetText(sv.c_str());
                    eb.SelectText();
                    break;
                }
            case CTL_DATE:
            case CTL_TIME:
                {
                    wdt.set_value(json::value(sv));
                    break;
                }
            case CTL_DD_SELECT:
                {
                    dropdown_selectitem_by_itemtext(wdt,sv);
                    break;
                }
            }
            wdt.toggle_state(STATE_FOCUS, true);
        }

        virtual BOOL on_script_call(HELEMENT he, LPCSTR name, UINT argc,
            json::value* argv, json::value& /*retval*/)
        {
            if (aux::streq(name, "wfoff") && (nullptr != argv))
            { // widget focus off
                dom::element table = he; //cell = (HELEMENT)argv[0].get_object_data();
                close_cell_widget(table, true);
            }
//             else if (aux::streqi(name, "showiget") && (nullptr != argv))
//             {
//                 dom::element cell = (HELEMENT)argv[0].get_object_data();
//                 json::string sv = cell.text();
//                 cell.set_text();
//                 on_cell_widget_show(cell);
//             }
            return true;
        }

        virtual BOOL on_method_call(HELEMENT he, UINT methodID, METHOD_PARAMS* params )
        {
            switch (methodID)
            {
            case QUI_CUSTOM_METHOD_SHEET_SHOWCELLWIDGET:
                {
                    _METHOD_PARAM_WITHDOM *p = reinterpret_cast<_METHOD_PARAM_WITHDOM*>(params);
                    ASSERT(p != nullptr);
                    if (nullptr != p)
                    {
                        dom::element etable = p->h1;
                        dom::element ecell = p->h2;

                        set_current_row(etable, dom::element(ecell.parent()), 0, false, false);
                        on_cell_click(etable, ecell);
                    }
                    return TRUE;
                }
            case QUI_CUSTOM_METHOD_SHEET_CLOSECELLWIDGET:
                {
                    _METHOD_PARAM_WITHDOM *p = reinterpret_cast<_METHOD_PARAM_WITHDOM*>(params);
                    ASSERT(p != nullptr);
                    if (nullptr != p)
                    {
                        dom::element etable = p->h1;
                        close_cell_widget(etable, p->dw);
                    }
                    return TRUE;
                }
            case QUI_CUSTOM_METHOD_SHEET_GETCUREDITCELL:
                {
                    _METHOD_PARAM_WITHDOM *p = reinterpret_cast<_METHOD_PARAM_WITHDOM*>(params);
                    ASSERT(p != nullptr);
                    if (nullptr != p)
                    {
                        dom::element etable = p->h1;
                        // 当前选中的td，cell
                        // 不使用传过来的参数，我们自己找到当前选中的
                        p->h2 = (HELEMENT)etable.get_attribute_int("sheet_cur_check_cell", 0);
                    }
                    return TRUE;
                }
            }
            return FALSE;
        }

        inline BOOL is_has_header(const dom::element& table)
        {
            return fixed_rows(table) > 0;
        }

        inline BOOL is_cell_has_widget(const dom::element &cell)
        {
            return cell.find_first("[cell-widget]") != NULL;
        }

        inline BOOL is_cellwidget(const dom::element& el)
        {
            return el.is_valid() && (el.get_attribute("cell-widget") != nullptr);
        }

        void on_cell_click(dom::element &table, dom::element& cell)
        {
            using namespace dom;

            if (!is_has_header(table))
                return;

            // 关闭当前显示的
            close_cell_widget(table, true);
            //close_current_cell_widget(table);
            
            // 即将显示的
            dom::element wdt = cell_widget(table, cell);
            if (wdt.is_valid())
            {
                switch (wdt.get_ctl_type())
                {
                case CTL_DATE:
                    {
                        wdt = dom::element::create("widget");
                        wdt.set_attribute("type",L"date");
                        break;
                    }
                case CTL_TIME:  // 特殊处理，因为clone出来的控件不对
                    {
                        wdt = dom::element::create("widget");
                        wdt.set_attribute("type",L"time");
                        break;
                    }
                default:
                    {
                        wdt = wdt.clone();
                        break;
                    }
                }
                cell.set_attribute("old_text", cell.text());
                cell.set_text(L"");
                cell.append(wdt);
                wdt.set_attribute("cell-widget",L"1");
                // cell.set_attribute("sheet_cur_check_cell",L"1");
                show_cell_widget(table, cell);
            }
            else
            {
                // cell.remove_attribute("sheet_cur_check_cell");
                // table.remove_attribute("sheet_cur_check_cell");
            }
        }

        dom::element target_cell(const dom::element& row, const dom::element& target)
        {
            if( !target.is_valid() || target.parent() == row)
                return target;
            return target_cell(row, target.parent());
        }

        virtual BOOL on_mouse(HELEMENT he, HELEMENT target, UINT event_type, 
            POINT pt, UINT mouseButtons, UINT keyboardStates )
        {
            if( (mouseButtons != MAIN_MOUSE_BUTTON) // 只有鼠标左键可激活
                || (event_type != (MOUSE_DCLICK | SINKING)) ) // 而且非双击不处理
            {
                return _base::on_mouse(he, target, event_type, pt, mouseButtons, keyboardStates);
            }

            // el must be table;
            dom::element table = he;
            dom::element cell = target;
            dom::element row = target_row(table, cell );
            
            if(row.is_valid() && ((int)row.index() >= (int)fixed_rows(table))) 
            {
                //if (get_current_row(table) == row )
                {   
                    // 只有在row已经选中的情况下才能显示cell-widget
                    // 这个target是否是cell (tr>td)?
                    cell = target_cell(row, cell);
                    // 这个cell是否已经有cell-widget了？
                    // 有widget的话，消息应该是发给widget的，这儿不应该处理
                    if (is_cell_has_widget(cell))
                        return FALSE;

                    // 必须点击到td上
                    on_cell_click(table, cell);
                }
            }

            return _base::on_mouse(he, target, event_type, pt, mouseButtons, keyboardStates);
        }

        virtual BOOL on_key(HELEMENT he, HELEMENT target, UINT event_type, 
            UINT code, UINT keyboardStates )
        {
            if (((KEY_DOWN|SINKING) == event_type)
                && ((VK_RETURN == code) || (VK_ESCAPE == code)))
            {
                dom::element wdt(target);
                if (!is_cellwidget(wdt))
                    wdt = wdt.parent();
                if (is_cellwidget(wdt))
                {
                    if ( VK_RETURN == code )    // 更新之
                        close_cell_widget(dom::element(he), true);
                        // close_cell_widget(dom::element(wdt.parent()), true);
                    else if (VK_ESCAPE == code) // 不更新
                        close_cell_widget(dom::element(he), false);
                }
            }
            return _base::on_key(he, target, event_type, code, keyboardStates);
        }

    };

    // instantiating and attaching it to the global list
    treelist        treelist_instance;
    sheet          sheet_instance;
    grid          grid_instance;
    sortable_grid sortable_grid_instance;

}
