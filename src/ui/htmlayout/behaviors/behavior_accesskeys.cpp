#include "behavior_aux.h"

namespace htmlayout 
{
 
/*

BEHAVIOR: accesskeys
    goal: support of accesskey html attribute
COMMENTS: 

SAMPLE:
   See: html_samples/behaviors/accesskeys.htm

*/


struct accesskeys: public behavior
{
    // ctor
    accesskeys(): behavior(HANDLE_KEY, "accesskeys") {}
    
    virtual BOOL on_key(HELEMENT he, HELEMENT target, UINT event_type, UINT code, UINT keyboardStates ) 
    { 
        const char* keyname = 0;
        if( event_type == (KEY_DOWN | SINKING) && (keyboardStates & ALT_KEY_PRESSED) == 0  )
        {
          keyname = get_key_name( code, keyboardStates);
          if( !keyname )
            return FALSE;
        }
        else if( (event_type == (KEY_CHAR | SINKING)) && (keyboardStates == ALT_KEY_PRESSED) ) 
        {
          if( code != '\'' && code != '\"' )
          {
            static char name[2];
            name[0] = (char) code;
            name[1] = 0;
            keyname = name;
          }
          else
            return false;
        }
        else 
          return false;

        dom::element container = he;

        HWND thisHWnd = container.get_element_hwnd(false);
        
        // handling IFrame in focus situation
        if( ::GetFocus() == thisHWnd) 
        {
            // ！！此处有bug！！
            //  假如 A 仅是 B 的逻辑上的父亲，（即B并非ws_child，而是popup）。
            //     这时父亲 B 是不应该处理这个快捷键消息的
            //     例如：QDialog A通过DoModal调用生成了Dialog B。 B默认设置父亲是A
            //          A此时应该被disabled掉。B得到快捷键消息，A是不应该处理此消息的
            //          但假如B不判断自己是否是ws_child，A也会得到消息
            //  所以应该处理这个消息时，加上自己是否是child的判断
            if (::GetWindowLong(thisHWnd, GWL_STYLE) & WS_CHILD)
            {
                dom::element super_container = dom::element::root_element( ::GetParent(thisHWnd) );
                if( super_container.is_valid() ) // yes, we have outer frame
                {
                    if(process_key(super_container, keyname))
                        return TRUE;
                }
            }
        }

        // normal handling

        if(process_key(container, keyname))
            return TRUE;

        // child iframes handling (if any)
        struct CB:public htmlayout::dom::callback 
        {
          const char* keyname;
          bool done;
          virtual bool on_element(HELEMENT he) 
          {
            htmlayout::dom::element iframe = he; 
            if( iframe.enabled() && iframe.visible() ) // only if it is visible and enabled
            {
              HWND hwndIFrame = iframe.get_element_hwnd(false);
              htmlayout::dom::element iframeRoot = htmlayout::dom::element::root_element(hwndIFrame);
              if(accesskeys::process_key( iframeRoot, keyname ))
              {
                done = true;
                return true; // stop enumeration
              }
            }
            return false;
          }
        };
        CB cb;
        cb.done = false;        
        cb.keyname = keyname;
        container.find_all(&cb, "iframe");
        return cb.done;
       
    }

    static BOOL process_key( dom::element& container, const char* keyname )
    {
        // find all callback 
        struct:public htmlayout::dom::callback 
        {
          htmlayout::dom::element hot_key_element;
          virtual bool on_element(HELEMENT he) 
          {
            htmlayout::dom::element t = he;
            if( !t.enabled() )
              return false;
            if( t.test("menu>li") )
            {
              hot_key_element = t;
              return true; // found, stop;
            }
            // 2013/1/13 qiuchengw 
            // 修改为不可见的元素也可以响应快捷键
//            if(t.visible())
            {
              hot_key_element = t;
              return true; // found, stop;
            }
            return false;
          }
        } cb;
        
        //Original version was:
        //  container.find_all(&cb, "[accesskey=='%s']", keyname);
        
        //By request of Christopher Brown, the Great, from Symantec this became as: 
        container.find_all(&cb, "[accesskey=='%s'],[accesskey-alt=='%s']", keyname, keyname);

        if( cb.hot_key_element.is_valid())
        {
          METHOD_PARAMS prm; prm.methodID = DO_CLICK; 
          //cb.hot_key_element.set_state(STATE_FOCUS);
          if(cb.hot_key_element.call_behavior_method(&prm))
            return true;
          
          // accesskey is defined for the element but it does not
          // handle DO_CLICK. Ask parents to activate it.
          // See behavior_tabs.cpp...
          htmlayout::dom::element hot_element_parent = cb.hot_key_element.parent();
          
          return hot_element_parent.send_event(ACTIVATE_CHILD,0, cb.hot_key_element);
        }
        return false;
    }

    
    // get symbolic name of the key combination
    const char* get_key_name( UINT scanCode, UINT keyboardStates)
    {
      if( (keyboardStates & SHIFT_KEY_PRESSED) != 0)
        return 0;

      if( scanCode == VK_MENU )
        return 0;

      static char buffer[ 20 ]; buffer[0] = 0;
      if( (keyboardStates & CONTROL_KEY_PRESSED) != 0)
        strcat_s(buffer,20,"^"); // ctrl
      if( keyboardStates == 0)
        strcat_s(buffer,20,"!"); // key without ALT modificator (as is).

      if( scanCode >= 'A' && scanCode <= 'Z' )
      {
        char s[2]; s[0] = char(scanCode); s[1] = 0;
        strcat_s(buffer,20,s);
        return buffer;
      } 
      else if( scanCode >= '0' && scanCode <= '9' )
      {
        char s[2]; s[0] = char(scanCode); s[1] = 0;
        strcat_s(buffer,20,s);
        return buffer;
      } 
      else switch(scanCode)
      {
         case VK_BACK        : strcat_s(buffer,20,"BACK"); return buffer;           

         case VK_CLEAR       : strcat_s(buffer,20,"CLEAR"); return buffer;          
         case VK_RETURN      : strcat_s(buffer,20,"RETURN"); return buffer;         

         case VK_ESCAPE      : strcat_s(buffer,20,"ESCAPE"); return buffer;         

         case VK_SPACE       : strcat_s(buffer,20,"SPACE"); return buffer;          
         case VK_PRIOR       : strcat_s(buffer,20,"PRIOR"); return buffer;          
         case VK_NEXT        : strcat_s(buffer,20,"NEXT"); return buffer;           
         case VK_END         : strcat_s(buffer,20,"END"); return buffer;            
         case VK_HOME        : strcat_s(buffer,20,"HOME"); return buffer;           
         case VK_LEFT        : strcat_s(buffer,20,"LEFT"); return buffer;           
         case VK_UP          : strcat_s(buffer,20,"UP"); return buffer;             
         case VK_RIGHT       : strcat_s(buffer,20,"RIGHT"); return buffer;          
         case VK_DOWN        : strcat_s(buffer,20,"DOWN"); return buffer;           
         case VK_SELECT      : strcat_s(buffer,20,"SELECT"); return buffer;         
         case VK_PRINT       : strcat_s(buffer,20,"PRINT"); return buffer;          
         case VK_EXECUTE     : strcat_s(buffer,20,"EXECUTE"); return buffer;        
         case VK_SNAPSHOT    : strcat_s(buffer,20,"SNAPSHOT"); return buffer;       
         case VK_INSERT      : strcat_s(buffer,20,"INSERT"); return buffer;         
         case VK_DELETE      : strcat_s(buffer,20,"DELETE"); return buffer;         
         case VK_HELP: strcat_s(buffer, 20, "HELP"); return buffer;
         case VK_TAB:   strcat_s(buffer, 20, "TAB"); return buffer;
         
         case VK_LWIN        : strcat_s(buffer,20,"LWIN"); return buffer;           
         case VK_RWIN        : strcat_s(buffer,20,"RWIN"); return buffer;           
         case VK_APPS        : strcat_s(buffer,20,"APPS"); return buffer;           

         case VK_NUMPAD0     : strcat_s(buffer,20,"NUMPAD0"); return buffer;        
         case VK_NUMPAD1     : strcat_s(buffer,20,"NUMPAD1"); return buffer;        
         case VK_NUMPAD2     : strcat_s(buffer,20,"NUMPAD2"); return buffer;        
         case VK_NUMPAD3     : strcat_s(buffer,20,"NUMPAD3"); return buffer;        
         case VK_NUMPAD4     : strcat_s(buffer,20,"NUMPAD4"); return buffer;        
         case VK_NUMPAD5     : strcat_s(buffer,20,"NUMPAD5"); return buffer;        
         case VK_NUMPAD6     : strcat_s(buffer,20,"NUMPAD6"); return buffer;        
         case VK_NUMPAD7     : strcat_s(buffer,20,"NUMPAD7"); return buffer;        
         case VK_NUMPAD8     : strcat_s(buffer,20,"NUMPAD8"); return buffer;        
         case VK_NUMPAD9     : strcat_s(buffer,20,"NUMPAD9"); return buffer;        
         case VK_MULTIPLY    : strcat_s(buffer,20,"MULTIPLY"); return buffer;       
         case VK_ADD         : strcat_s(buffer,20,"ADD"); return buffer;            
         case VK_SEPARATOR   : strcat_s(buffer,20,"SEPARATOR"); return buffer;      
         case VK_SUBTRACT    : strcat_s(buffer,20,"SUBTRACT"); return buffer;       
         case VK_DECIMAL     : strcat_s(buffer,20,"DECIMAL"); return buffer;        
         case VK_DIVIDE      : strcat_s(buffer,20,"DIVIDE"); return buffer;         
         case VK_F1          : strcat_s(buffer,20,"F1"); return buffer;             
         case VK_F2          : strcat_s(buffer,20,"F2"); return buffer;             
         case VK_F3          : strcat_s(buffer,20,"F3"); return buffer;             
         case VK_F4          : strcat_s(buffer,20,"F4"); return buffer;             
         case VK_F5          : strcat_s(buffer,20,"F5"); return buffer;             
         case VK_F6          : strcat_s(buffer,20,"F6"); return buffer;             
         case VK_F7          : strcat_s(buffer,20,"F7"); return buffer;             
         case VK_F8          : strcat_s(buffer,20,"F8"); return buffer;             
         case VK_F9          : strcat_s(buffer,20,"F9"); return buffer;             
         case VK_F10         : strcat_s(buffer,20,"F10"); return buffer;            
         case VK_F11         : strcat_s(buffer,20,"F11"); return buffer;            
         case VK_F12         : strcat_s(buffer,20,"F12"); return buffer;            
         case VK_F13         : strcat_s(buffer,20,"F13"); return buffer;            
         case VK_F14         : strcat_s(buffer,20,"F14"); return buffer;            
         case VK_F15         : strcat_s(buffer,20,"F15"); return buffer;            
         case VK_F16         : strcat_s(buffer,20,"F16"); return buffer;            
         case VK_F17         : strcat_s(buffer,20,"F17"); return buffer;            
         case VK_F18         : strcat_s(buffer,20,"F18"); return buffer;            
         case VK_F19         : strcat_s(buffer,20,"F19"); return buffer;            
         case VK_F20         : strcat_s(buffer,20,"F20"); return buffer;            
         case VK_F21         : strcat_s(buffer,20,"F21"); return buffer;            
         case VK_F22         : strcat_s(buffer,20,"F22"); return buffer;            
         case VK_F23         : strcat_s(buffer,20,"F23"); return buffer;            
         case VK_F24         : strcat_s(buffer,20,"F24"); return buffer;            
      }
      return 0;
    }

};

// instantiating and attaching it to the global list
accesskeys accesskeys_instance;

}
