#include "behavior_aux.h"
#include "../htmlayout_canvas.hpp"

#include <math.h>
#include <time.h>


namespace htmlayout
{
    namespace dom
    {
        struct qcalendar : public behavior
        {
            struct _CalCell : public htmlayout::dom::ETable
            {
            public:
                _CalCell(HELEMENT h) : ETable(h)
                {

                }

                ECtrl _Day()
                {
                    return find_first(".day");
                }

                ECtrl _LunarDay()
                {
                    return find_first(".lunarday");
                }

                // 自定义的两个Td
                ECtrl _FirstC()
                {
                    return find_first(".first");
                }

                ECtrl _SecondC()
                {
                    return find_first(".second");
                }

                BOOL SetDateTime(__in QTime & dtTime, __in BOOL bCurMonth)
                {
                    CHolidayLookup *pHoliday = CHolidayLookup::GetInstance();
                    if (bCurMonth)
                    {
                        // 在本月内
                        RemoveClass(L"other-month");
                    }
                    else
                    { 
                        // 不在本月内
                        AddClass(L"other-month");
                    }

                    // 是否是星期天？
                    if ((dtTime.GetDayOfWeek() == 1) || (dtTime.GetDayOfWeek() == 7))
                    {
                        AddClass(L"off");
                    }
                    else
                    {
                        RemoveClass(L"off");
                    }

                    // 是否是今天？
                    if (dtTime.CompareDate(QTime::GetCurrentTime()) == 0)
                    {
                        AddClass(L"today");
                    }
                    else
                    {
                        RemoveClass(L"today");
                    }

                    QString sLunarDay = CLunarDateTime(dtTime).GetLunarDayName();
                    if(pHoliday->IsSolarHoliday(dtTime))
                    {
                        // 是节假日, 设置节假日背景图片
                        AddClass(L"holiday");

                        sLunarDay = pHoliday->GetSolarHoliday(dtTime);
                    }

                    // day
                    _Day().SetText(L"%d", dtTime.GetDay());
                    // lunarday
                    _LunarDay().SetText( sLunarDay );

                    return TRUE;
                }
            };

            static const int MIN_YEAR = 1901;
            static const int MAX_YEAR = 2050;

            qcalendar() 
                : behavior(HANDLE_BEHAVIOR_EVENT|HANDLE_METHOD_CALL, "qcalendar")
            {

            }

            // 如果这一行全是其它月的，是否隐藏掉这一行？
            bool is_hide_othermonthweek(ECtrl& el)
            {
                return el.get_attribute("hide_othermonthweek") != NULL;
            }

            virtual void attached  (HELEMENT he ) 
            {
                // 生成一个表 // 7 * 7
                // 第一行 表头
                //                 <widget type="qcalendar">
                //                     <caption></caption>
                //                     <table .cal>
                //                         <tr><th>....</th></tr>
                //                         <tr>...</tr>
                //                     </table>
                //                 </widget>

                ECtrl el(he);
                // 先生成caption
                ECtrl cap = el.create("caption");
                el.append(cap);
                QTime tm;
                init_caption(cap, tm);

                // 再生成表体 7 * 9
                // 最左列和最右列为自定义的列
                ETable cal = el.create("table");
                el.append(cal);
                cal.SetClass(L"body");
                init_body(cal, tm);
            } 

            // 初始化caption
            void init_caption(__inout ECtrl& cap, __out QTime& tm)
            {
                // 当前时间
                tm = QTime::GetCurrentTime();

                // caption 格式：
                // [今天] <   2010 - 09   >
                // 今天
                ECtrl bn_today = cap.create("div");
                cap.append(bn_today);
                bn_today.SetClass(L"qbtn to-today");

                // 前月
                ECtrl bn_l = cap.create("div");
                cap.append(bn_l);
                bn_l.SetClass(L"qbtn prev-date");

                // 年
                ECtrl cT = cap.create("widget");
                cT.set_attribute("type",L"select-dropdown");
                cap.append(cT);
                cT.SetClass(L"year");

                // 初始化从1901 ~ 2050
                ECombobox cmb = cT;
                QString s;
                for (int i = MIN_YEAR; i <= MAX_YEAR; i++) 
                {
                    s.Format(L"%d", i);
                    cmb.InsertItem(s);
                }
                s.Format(L"%d", tm.GetYear());
                cmb.SelectItem_Text(s);

                // 月
                cT = cap.create("select");
                cT.set_attribute("type",L"select-dropdown");
                cap.append(cT);
                cT.SetClass(L"month");

                // 初始化从1 - 12
                cmb = (HELEMENT)cT;
                for (int i = 1; i <= 12; i++) 
                {
                    s.Format(L"%02d", i);
                    cmb.InsertItem(s);
                }
                s.Format(L"%02d", tm.GetMonth());
                cmb.SelectItem_Text(s);

                // 下一个
                ECtrl bn_r = cap.create("div");
                cap.append(bn_r);
                bn_r.SetClass(L"qbtn next-date");

                // 显示到桌面
                ECtrl bn_d = cap.create("div", L"显示至桌面");
                cap.append(bn_d);
                bn_d.SetClass(L"qbtn to-desktop");
                bn_d.SetID(L"btn_to_desktop");
            }

            // 初始化日历体
            bool init_body(__inout ETable& cal, __in QTime& tm)
            {
                // 不能一个个的创建子元素，需要创建所有的html，一次性设置
                QString shtm = L"<tr>";
                // 第一行表头 为周
                //                 for (int i = 0; i < 9; ++i) 
                //                 {
                //                     shtm += L"<th .weekday />";
                //                 }
                shtm += L"<th .weekday />";
                shtm += L"<th .weekday>日</th>";
                shtm += L"<th .weekday>一</th>";
                shtm += L"<th .weekday>二</th>";
                shtm += L"<th .weekday>三</th>";
                shtm += L"<th .weekday>四</th>";
                shtm += L"<th .weekday>五</th>";
                shtm += L"<th .weekday>六</th>";
                shtm += L"<th .weekday />";
                shtm += L"</tr>";

                // 接下来6行为表体，除了第一列和最后一列，
                // 其余的每个<td>内都是一个<table>
                QString s;
                for (int i = 0; i < 6; ++i) 
                {
                    shtm += L"<tr>";
                    for (int j = 0; j < 9; ++j) 
                    {
                        if ((j == 0) || (j == 8))
                        {
                            shtm += L"<td .nweek/>";
                        }
                        else
                        {
                            shtm += L"<td>"
                                L"<table .cell>"
                                L"  <tr>"
                                L"      <td .first />"  // 自定义cell-1
                                L"      <td .day />"    // 公历day
                                L"      <td .second />" // 自定义cell-1
                                L"  </tr>"
                                L"  <tr><td .lunarday colspan=3 /></tr>" // 农历日期
                                L"</table>"
                                L"</td>";
                        }
                    }
                    shtm += L"</tr>";
                }

                // 生成之
                cal.SetHtml(shtm);

                // 改变时间
                return change_date(ECtrl(cal.parent()), tm.GetYear(), tm.GetMonth());
            }

            // helper
            inline ECombobox _CmbYear(ECtrl& el)
            {
                return el.find_first("caption>.year");
            }

            inline ECombobox _CmbMonth(ECtrl& el)
            {
                return el.find_first("caption>.month");
            }

            // 日期改变 el 为 widget wrapper
            void on_date_changed(ECtrl& el)
            {
                int y = StrToInt(_CmbYear(el).GetCurItemText());
                int m = StrToInt(_CmbMonth(el).GetCurItemText());

                change_date(el, y, m );
            }

            // 向前向后按钮改变月，日期改变 el 为 widget wrapper
            void change_month(__in ECtrl& el, __in BOOL bGotoPrev)
            {
                ECombobox cmb_y = _CmbYear(el);
                ECombobox cmb_m = _CmbMonth(el);

                int nYear = StrToInt(cmb_y.GetCurItemText());
                int nMonth = StrToInt(cmb_m.GetCurItemText());

                if (bGotoPrev) // 月向前
                {
                    nMonth -=1;
                    if(nMonth<1)
                    {
                        nMonth=12;
                        nYear -=1;
                    }
                }
                else // 月向后
                {
                    nMonth +=1;
                    if(nMonth>12)
                    {
                        nMonth=1;
                        nYear +=1;
                    }
                }

                // 更新界面
                if ( (nYear <= MAX_YEAR) && (nYear >= MIN_YEAR))
                {
                    QString s;
                    s.Format(L"%d", nYear);
                    cmb_y.SelectItem_Text(s);

                    s.Format(L"%02d", nMonth);
                    cmb_m.SelectItem_Text(s);

                    // 日期改变了
                    on_date_changed(el);
                }
            }

            // 改变日期到指定的年和月，nYear应该在 [MIN_YEAR, MAX_YEAR]范围内
            bool change_date(ECtrl&el , int nYear,int nMonth) 
            {
                if (   (nYear > MAX_YEAR) || (nYear < MIN_YEAR)
                    || (nMonth < 1) || (nMonth > 12))
                {
                    ASSERT(FALSE);
                    return false;
                }

                // 更新年、月下来列表显示
                QString s;
                s.Format(L"%d",nYear);
                _CmbYear(el).SelectItem_Text(s);
                s.Format(L"%02d", nMonth);
                _CmbMonth(el).SelectItem_Text(s);

                // 更新日历
                ETable body = el.find_first(L":root>table.body");

                int nApartDays;
                int nWeek = WeekDay(nYear,nMonth,1);
                int nMonthday = MonthDays(nYear,nMonth);
                int nFirstDay = (nWeek == 0) ? 7 : nWeek;

                ECtrl rowWeek, tdWeek;
                QTime dtFirstDay(nYear, nMonth, 1, 0 , 0 , 0);
                QTime dtDay;
                bool bHideOtherWeek = is_hide_othermonthweek(el);

                // 去掉那个当前的check
                ECtrl cCur = el.find_first("table:checked");
                if (cCur.is_valid())
                {
                    cCur.SetCheck(FALSE);
                }

                // 真正需要设置的day-cell只有 6 * 7 个
                // 6 - 共6行，一个月内最多可以跨越6个周
                // 7 - 一周7天
                for (int i = 1, day = 0; i <= 6; ++i)  // 共 6 行
                {   
                    // 这是行的DOM
                    rowWeek = body.child(i);
                    // 第一列显示 “xx 周”
                    tdWeek = rowWeek.child(0);

                    for (int j = 1; j <= 7; ++j)  // 一行一周，共 7 天
                    {
                        nApartDays = day++ - nFirstDay;
                        dtDay = dtFirstDay + QTimeSpan(nApartDays, 0, 0, 0);

                        // row + 1, col + 1, 因为第一行为周，第一列为自定义列
                        // _CalCell dayCell = _DayCell(body, j / 7 + 1, j % 7 + 1);
                        _CalCell dayCell = _DayCell(body, i, j);
                        // 是否在本月内
                        //BOOL bCurMonth = ((nApartDays >= 0) && (nApartDays < nMonthday));
                        BOOL bCurMonth = ((nApartDays >= 0) && (nApartDays < nMonthday));
                        if (1 == j)
                        {
                            // 显示周
                            tdWeek.SetHtml(dtDay.Format(L"%W") + L"<br/>周");
                            if ((6 == i) && bHideOtherWeek)
                            {
                                // 如果最后一行的第一天都在别的月份内。这一行可以不显示了
                                rowWeek.ShowCtrl(bCurMonth ? SHOW_MODE_SHOW : SHOW_MODE_COLLAPSE);
                            }
                        }
                        else if(bHideOtherWeek && (7 == j) && (1 == i))
                        {
                            // 如果第一行的最后一天都在别的月内，这一行可以不显示了
                            rowWeek.ShowCtrl(bCurMonth ? SHOW_MODE_SHOW : SHOW_MODE_COLLAPSE);
                        }

                        // 设置显示一下啦
                        dayCell.SetDateTime(dtDay, bCurMonth);
                    }
                }
                return true;
            }

            // 找到iRow， iCol对应的table>tr>td
            // iRow 应该在 [0, 6] 范围内
            // icol [0, 8]
            ECtrl _CalendarCell(__in ETable& body, __in int iRow, __in int iCol) 
            {
                // 第一行为周，索引：0
                if ((iRow < 0) || (iCol < 0) || (iCol > 8))
                {
                    ASSERT(FALSE);
                    return NULL;
                }

                ECtrl tr = body.find_first(":root>tr:nth-child(%d)", iRow + 1);
                if (tr.is_valid())
                {
                    return tr.child(iCol);
                }
                return NULL;
            }

            // 找到iRow， iCol对应的day-cell
            // table.body>tr>td>table.cell
            _CalCell _DayCell(__in ETable& body, __in int iRow, __in int iCol) 
            {
                return _CalendarCell(body, iRow, iCol).find_first(":root>table.cell");
            }

            virtual BOOL handle_event (HELEMENT he, BEHAVIOR_EVENT_PARAMS& params ) 
            {
#ifdef _DEBUG
                TraceBehaviorEventToDC(params);
#endif

                return behavior::handle_event(he, params);
            }

            void on_bnclick(ECtrl& el, ECtrl btn)
            {
                QString sCls = btn.GetClass();

                // 上个月
                if (sCls.Find(L"prev-date") != -1)
                    change_month(el, TRUE);
                // 下个月
                else if (sCls.Find(L"next-date") != -1)
                    change_month(el, FALSE);
                else if (sCls.Find(L"to-today") != -1)
                {
                    // 转到今天
                    QTime t = QTime::GetCurrentTime();
                    change_date(el, t.GetYear(), t.GetMonth());
                }
                else if (sCls.Find(L"to-desktop") != -1)
                {
                    // 钉到桌面
                    // 需要通知到上级
                }
            }

            virtual BOOL on_event(HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason )
            {
                ECtrl el(he);
                switch (type)
                {
                case BUTTON_CLICK:
                    {
                        on_bnclick(el, target);
                        break;
                    }

                case SELECT_SELECTION_CHANGED:
                    {
                        on_date_changed(el);

                        break;
                    }
                }
                return false;
            }
        };

        qcalendar qcalendar_instance;
    }
}
