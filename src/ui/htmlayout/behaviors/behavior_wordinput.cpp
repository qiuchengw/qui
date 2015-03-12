#include "behavior_aux.h"
#include "ui/ECtrl.h"

/*
 *  <widget type="keyword-input" 
 *	        word-maxlen = 5     // input 关键字最长几个字符
 *          size=10             // select最多显示多少item
 *  />
 */
namespace htmlayout
{
    namespace dom
    {
        struct keyword_input : public behavior
        {
            keyword_input() 
                : behavior(HANDLE_BEHAVIOR_EVENT|HANDLE_MOUSE|HANDLE_KEY, "keyword-input")
            {

            }

            virtual void attached  (HELEMENT he ) 
            {
                ECtrl el(he);

                ECtrl cap = el.create("caption");
                el.append(cap);

                EPopup pp = el.create("popup");
                el.append(pp);

                ECtrl inpt = pp.create("input");
                pp.append(inpt);
                inpt.set_attribute("type",L"text");
                inpt.set_attribute("maxlength", aux::itow(el.get_attribute_int("word-maxlen", 5)));
                inpt.SetName(L"input_new_word");
                inpt.set_attribute("novalue",L"创建……");

//                 ETagBox box = pp.create("widget");
//                 pp.append(box);
//                 pp.set_attribute("type",L"tagbox");

                ECtrl sel = pp.create("select");
                pp.append(sel);
                // 默认显示多少个？
                sel.set_attribute("size", aux::itow(el.get_attribute_int("size", 5)));
                sel.set_attribute("type", L"select");
            } 

#ifdef _DEBUG
            void test_input(HELEMENT he)
            {
                ECtrl el = he;
                EWordInput wi = he;
                wi.AddWordItem(L"hello");
                wi.AddWordItem(L"world");
                wi.AddWordItem(L"test");
                wi.AddWordItem(L"this");
                wi.AddWordItem(L"is");

                json::astring ss = el.get_html(1);
            }
#endif

            void on_selchanged(EWordInput wi, EOptionsBox box)
            {
                EOption op = box.GetCurSelItem();
                wi.AddCaptionItem(op.GetText());
                op.AddClass(L"check");
            }

            EPopup _popup(ECtrl el)
            {
                return el.find_first("popup");
            }

            void filter_select(EWordInput wi)
            {

            }

            virtual BOOL on_event(HELEMENT he, HELEMENT target,
                BEHAVIOR_EVENTS type, UINT_PTR reason )
            {
                switch (type)
                {
                case SELECT_SELECTION_CHANGED:
                    {
                        on_selchanged(he, target);

                        return TRUE;
                    }

                case BUTTON_CLICK:
                    {
                        EPopup pp = _popup(he);
                        if (!pp.IsHasAttribute("showing"))
                        {
                            HTMLayoutShowPopup(pp, he, 2);
                            pp.set_attribute("showing", L"1");
                        }
                        else
                        {
                            pp.hide_popup();
                            pp.remove_attribute("showing");
                        }
                        return TRUE;
                    }
                case EDIT_VALUE_CHANGED:
                    {

                        return TRUE;
                    }
                case POPUP_DISMISSED:
                    {
                        EPopup pp = _popup(he);
                        if (target == pp)
                        {
                            pp.remove_attribute("showing");
                        }
                        break;
                    }
                  }
                return FALSE;
            }

            virtual BOOL on_mouse(HELEMENT he, HELEMENT target, UINT event_type, 
                POINT pt, UINT mouseButtons, UINT keyboardStates )
            {
                if (MOUSE_UP != event_type)
                    return FALSE;

                ECtrl c = target;
                if (c.IsHasClass(L"word"))
                {
                    c.destroy();
                }
                return FALSE;
            }

            virtual BOOL on_key(HELEMENT he, HELEMENT target, 
                UINT event_type, UINT code, UINT keyboardStates )
            {
                if (KEY_DOWN != event_type)
                    return FALSE;

                if (VK_RETURN == code)
                {
                    ECtrl el = target;
                    if (el.Name().CompareNoCase(L"input_new_word") == 0)
                    {
                        EWordInput wi = he;
                        EEdit inpt = target;
                        CStdString wd = inpt.GetText();
                        if (!wd.IsEmpty())
                        {
                            wi.AddWordItem(wd);
                            wi.AddCaptionItem(wd);
                            inpt.SetText(L"");
                        }
                    }
                    return TRUE;
                }
                else if (VK_ESCAPE == code)
                {
                    ECtrl pp = ECtrl(he).find_first("popup");
                    pp.hide_popup();
                    return TRUE;
                }
                else if (VK_SPACE == code)
                {
                    // 不准输入空格
                    return TRUE;
                }
                else if (VK_DELETE == code)
                {
                    EWordInput(he).DeleteCheckedCaptionItem();
                    return TRUE;
                }
                return FALSE;
            }
        };

        keyword_input keyword_input_instance;
    }
}
