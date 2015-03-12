#include "behavior_aux.h"
#include "time/QTime.h"

/*
BEHAVIOR: clock2

TYPICAL USE CASE:
	<div style="behavior:clock2;">
*/

namespace htmlayout 
{

  struct text_clock2: public behavior
  {
      // ctor
      text_clock2(): behavior(HANDLE_TIMER, "clock2") { }
    
	  virtual void attached  (HELEMENT he ) 
	  { 
		  dom::element el = he;
		  if(el.visible())
			  HTMLayoutSetTimer( he, 1000 ); // set one second timer      
	  } 

	  virtual void detached  (HELEMENT he ) 
	  { 
		  HTMLayoutSetTimer( he, 0 ); // remove timer
	  } 

	  virtual BOOL on_timer  (HELEMENT he ) 
	  { 
		  dom::element el = he;
		  
		  SYSTEMTIME st;
		  GetLocalTime(&st);

		  wchar_t buf[16];
		  swprintf_s(buf,16,L"%02d:%02d:%02d",st.wHour,st.wMinute,st.wSecond);
		  el.set_text(buf);

		  return TRUE; /*keep going*/ 
	  }
  };

  struct text_countdown: public behavior
  {
      // ctor
      text_countdown(): behavior(HANDLE_TIMER|HANDLE_METHOD_CALL, "clock-countdown") 
      {
          
      }

      int _current_left(dom::element& el)
      {
          return el.get_attribute_int("countdown");
      }

      virtual void attached  (HELEMENT he ) 
      { 
          dom::element el = he;

          settime(el, el.text().c_str());  

          // 假如不掉用这两个函数，他们就可能被编译器优化掉。
          increase(el,0);
          decrease(el,0);
      }

      void settime(dom::element el, const wchar_t* szTime )
      {
          QTime t1;
          if (t1.ParseDateTime(szTime, VAR_TIMEVALUEONLY))
          {
              QTime t2 = t1;
              t2.SetTime(0,0,0);
#pragma warning(disable:4244)
              int lSecs = (t1 - t2).GetTotalSeconds();
#pragma warning(default:4244)
              if (lSecs <= 0)
              {
                  lSecs = 0;
              }
              el.set_attribute("countdown", aux::itow(lSecs));

              start(el);
          }
      }

      bool setcountdown(dom::element el, int lSecs)
      {
          if (lSecs > 0)
          {
              int nH = lSecs / 3600;
              lSecs %= 3600;
              wchar_t buf[16];
              swprintf_s(buf,16,L"%02d:%02d:%02d", nH, lSecs / 60, lSecs % 60);
              el.set_text(buf);
          
              el.set_attribute("countdown", aux::itow(lSecs));

              return true;
          }
          else
          {
              // 停止计时
              el.set_attribute("countdown", L"0");
              el.set_text(L"00:00:00");
              stop(el);
              return false;
          }
      }

      virtual BOOL on_script_call(HELEMENT he, LPCSTR name, UINT argc, json::value* argv, json::value& /*retval*/)
      {
          if (aux::streq(name, "increase"))
          {
              increase(he, argv[0].get(0));
          }
          else if (aux::streq(name, "decrease"))
          {
              decrease(he, argv[0].get(0));
          }
          else if (aux::streq(name, "start"))
          {
              start(he);
          }
          else if (aux::streq(name, "stop"))
          {
              stop(he);
          }
          else if (aux::streq(name, "settime"))
          {
              settime(he, argv[0].to_string().c_str());
          }
          return true;
      }

      // behavior 调用
      void increase(dom::element el, int nSecs)
      {
          setcountdown(el, nSecs + _current_left(el));
          start(el);
      }

      // behavior 调用
      void decrease(dom::element el, int nSecs)
      {
          if ( setcountdown(el, _current_left(el) - nSecs) )
          {
              start(el);
          }
      }

      // behavior 调用
      void start(dom::element el)
      {
          stop(el);
          HTMLayoutSetTimer( el, 1000 ); // remove timer
      }

      // behavior 调用
      void stop(dom::element el)
      {
          HTMLayoutSetTimer( el, 0 ); // remove timer
      }

      virtual void detached  (HELEMENT he ) 
      { 
          stop(he);
      } 

      virtual BOOL on_timer  (HELEMENT he ) 
      { 
          dom::element el = he;

          int lSecs = el.get_attribute_int("countdown");
          if (lSecs > 0)
          {
              lSecs--;
          }
          setcountdown(el, lSecs);
          return TRUE; /*keep going*/ 
      }
  };
  // instantiating and attaching it to the global list
  text_clock2 text_clock2_instance;
  text_countdown text_countdown_instance;

} // htmlayout namespace

