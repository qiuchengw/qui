#include "behavior_aux.h"
#include <string>
#include <vector>
#include <algorithm>

namespace htmlayout 
{
 
/** behavior:
style="behavior:dropdown-select sysfontfamily;" Ë³Ðò²»ÄÜ±ä
 * another way of scrolling  
 **/


using namespace dom;
static std::vector<std::wstring>  sg_vFontNames;

int CALLBACK xEnumFontsProc(LPLOGFONT lplf,LPTEXTMETRIC lptm,
	DWORD  dwStyle, LONG lParam) 
{ 
	if ( (L'@' != *(lplf->lfFaceName)) 
        &&  (std::wstring(lplf->lfFaceName).length() <= 8)
		&&  (std::find(sg_vFontNames.begin(),sg_vFontNames.end(),lplf->lfFaceName) == sg_vFontNames.end())
		)
	{
		sg_vFontNames.push_back(lplf->lfFaceName);
	}
	return  1; 
} 

struct xfontfamily: public behavior
{
    // ctor
    xfontfamily(): behavior(HANDLE_BEHAVIOR_EVENT, "sysfontfamily") {}
    
	virtual BOOL on_event (HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason ) 
	{ 
		switch( type )
		{
		case POPUP_READY: 
			{
				element ePopup(target);
				if (!ePopup.is_valid() || ePopup.children_count())
					return true;
				if (sg_vFontNames.empty())
				{
					LOGFONT lf;
					lf.lfFaceName[0] = L'\0';
					lf.lfCharSet = DEFAULT_CHARSET/*ANSI_CHARSET*/;
                    
                    HDC hDC = ::GetDC(NULL);
                    EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)xEnumFontsProc, NULL, 0);
					::ReleaseDC(NULL,hDC);
					sg_vFontNames.shrink_to_fit();
				}
				ePopup.set_style_attribute("max-height",L"200");
				auto itrEnd = sg_vFontNames.end();
				element eItem;
				for (auto itr = sg_vFontNames.begin(); itr != itrEnd; ++itr)
				{
					eItem = element::create("option");
					ePopup.append(eItem);
					eItem.set_text((*itr).c_str());
					eItem.set_style_attribute("font-family",(*itr).c_str());
				}
				return true;
			}
		}
		return false;
	}
};

xfontfamily          xfontfamily_instance;


}