#include "stdafx.h"
#include "ECtrl.h"
#include "BaseType.h"
#include "QUIGlobal.h"

#pragma warning(disable:4018)

#ifdef UIBASE_SUPPORT_FLASH
    #include "FlashWnd.h"
#endif

#ifdef UIBASE_SUPPORT_WEBBROWSER
    #include "browser/WebBrowser.h"
#endif

namespace htmlayout
{
    extern treelist        treelist_instance;

	namespace dom
	{
		ECtrl& ECtrl::operator=( HELEMENT h )
		{
			(NULL != he)?element::set(h):element::use(h);
			return *this;
		}

		ECtrl& ECtrl::operator=(const ECtrl& e)
		{
			this->operator=((HELEMENT)e);
			return *this;
		}

		void ECtrl::EnableCtrl( BOOL bEnable /*= TRUE*/ )
		{
			toggle_state(STATE_DISABLED,!bEnable);
		}

		void ECtrl::DeleteAllChild()
		{
			DeleteChild(0);

			// 为了正确清除元素所占用的资源，不要使用ECtrl::clear()
			// 因为它不会销毁子元素，只是重新设置了元素的内部html而已
			// 如果子元素是实体窗口（hwnd）的话，这个窗口并不会被销毁！
		}

		void ECtrl::DeleteChild( int ibegin,int iend/*=-1*/ )
		{
			int c = children_count();
			if ((-1 == iend) || iend >= c)
				iend = c - 1;
			else if(iend < ibegin)
			{
				return;
			}
			iend -= ibegin;
			while ((children_count() >= ibegin) && (iend >= 0))
			{
				ECtrl(child(ibegin)).destroy();
				iend--;
			}
			refresh();
		}

		void ECtrl::ShowTooltip( LPCWSTR sTip,int nDir/*=0*/ )
		{
			ECtrl r = root();
			ECtrl e = r.find_first(".qtooltip");
			if (!e.is_valid())
			{
				e = ECtrl::create("popup");
				r.append(e);
				e.set_attribute("class",L"qtooltip");
			}
			if (e.is_valid())
			{
                e.SetHtml(sTip);
				HTMLayoutShowPopup(e,*this,nDir);
			}
            SetFocus();
		}

		void ECtrl::SetTooltipText( LPCWSTR sTipText )
		{
			if (NULL == sTipText)
				remove_attribute("title");
			else
				set_attribute("title",sTipText);
		}

		void ECtrl::SetBkgndImage( LPCWSTR szImg )
		{
			if ((NULL == szImg) || !wcslen(szImg))
			{
				set_style_attribute("background-image",L"none");
			}
			else
			{
				WCHAR szBuf[512] = {0}; 
				swprintf_s(szBuf,512,L"url(%s)",szImg);
				set_style_attribute("background-image",szBuf);
			}
		}

		void ECtrl::SetTransparent( int nAlpha /*= 0*/ )
		{
			nAlpha = max(0,nAlpha);
			nAlpha %= 101;
			WCHAR szBuf[64] = {0};
			swprintf_s(szBuf,64,L"%.2f",(float)(100 - nAlpha) / (100.0f));
			set_style_attribute("opacity",szBuf);
		}

		void ECtrl::SetBkgndPosition( LPCWSTR sPt )
		{
			set_style_attribute("background-position",sPt); 
		}

		void ECtrl::SetBkgndPosition( int t,int r,int b,int l )
		{
			WCHAR szBuf[128] = {0};
			swprintf_s(szBuf,128,L"%d %d %d %dpx",t,r,b,l);
			SetBkgndPosition(szBuf);
		}

		void ECtrl::SetBkgndPosition( int x,int y )
		{
			WCHAR szBuf[64] = {0}; 
			swprintf_s(szBuf,64,L"%d %dpx",x,y);
			SetBkgndPosition(szBuf);
		}

		void ECtrl::SetFrgndImage( LPCWSTR szImg )
		{
			if ((NULL == szImg) || !wcslen(szImg))
			{
				set_style_attribute("foreground-image",L"none");
			}
			else
			{
				WCHAR szBuf[512] = {0}; 
				swprintf_s(szBuf,512,L"url(%s)",szImg);
				set_style_attribute("foreground-image",szBuf);
			}
		}

		void ECtrl::SetFrgndRepeat( LPCWSTR sRepeat )
		{
			set_style_attribute("foreground-repeat",sRepeat); 
		}

		void ECtrl::SetFrgndPosition( int x,int y )
		{
			WCHAR szBuf[64] = {0}; 
			swprintf_s(szBuf,64,L"%d %dpx",x,y);
			SetFrgndPosition(szBuf);
		}

		void ECtrl::SetFrgndPosition( int t,int r,int b,int l )
		{
			WCHAR szBuf[128] = {0};
			swprintf_s(szBuf,128,L"%d %d %d %dpx",t,r,b,l);
			SetFrgndPosition(szBuf);
		}

		void ECtrl::SetFrgndPosition( LPCWSTR sPt )
		{
			set_style_attribute("foreground-position",sPt); 
		}

		void ECtrl::SetBkgndRepeat( LPCWSTR sRepeat )
		{
			set_style_attribute("background-repeat",sRepeat); 
		}

		void ECtrl::SetBkgndColor( LPCWSTR sTop,LPCWSTR sRight, LPCWSTR sBottom,LPCWSTR sLeft )
		{
			WCHAR szBkgnd[100] = {0};
			swprintf_s(szBkgnd,100,L"%s %s %s %s",sTop,sRight,sBottom,sLeft);
			set_style_attribute("background-color",szBkgnd);
		}

		void ECtrl::SetBkgndColor( LPCWSTR sColor )
		{ 
			set_style_attribute("background-color",sColor); 
		}

		void ECtrl::SetCOLORREF( LPCWSTR sColor )
		{ 
			set_style_attribute("color",sColor); 
		}

		void ECtrl::SetFrgndColor( LPCWSTR sColor )
		{
			set_style_attribute("foreground-color",sColor); 
		}

		void ECtrl::SetFrgndColor( LPCWSTR sTop,LPCWSTR sRight, LPCWSTR sBottom,LPCWSTR sLeft )
		{
			WCHAR szBkgnd[100] = {0};
			swprintf_s(szBkgnd,100,L"%s %s %s %s",sTop,sRight,sBottom,sLeft);
			set_style_attribute("foreground-color",szBkgnd);
		}

		void ECtrl::SetWidth( int nx,LPCWSTR unit/*=L"px"*/ )
		{ 
			wchar_t sz[255]; 
			swprintf_s(sz,255,L"%d%s",nx,unit); 
			set_style_attribute("width",sz); 
		}

		void ECtrl::SetHeight( int ny,LPCWSTR unit/*=L"px"*/ )
		{
			wchar_t sz[255]; 
			swprintf_s(sz,255,L"%d%s",ny,unit); 
			set_style_attribute("height",sz);
		}

		void ECtrl::SetSize( int nx,int ny,LPCWSTR unit/*=L"px"*/ )
		{
			SetWidth(nx,unit);
			SetHeight(ny,unit);
		}

		void ECtrl::ShowCtrl( _CTRL_SHOW_MODE nMode )
		{
			if (SHOW_MODE_SHOW == nMode)
				set_style_attribute("visibility",L"visible"); 
			else if (SHOW_MODE_HIDE == nMode)
				set_style_attribute("visibility",L"hidden"); 
			else if (SHOW_MODE_COLLAPSE == nMode)
				set_style_attribute("visibility",L"collapse"); 
		}

		void ECtrl::ClearBkgndStyle()
		{
			clear_style_attribute("background");
		}

		void ECtrl::ClearFrgndStyle()
		{
			clear_style_attribute("background");
		}

		void ECtrl::SetData( LPVOID pData )
		{
			_Edata *p = reinterpret_cast<_Edata*>(get_expando());
			if (NULL == p)
			{
				p = new _Edata(pData);
				set_expando(p);
			}
			else
			{
				p->_data = pData;
			}
		}

		void ECtrl::RemoveData()
		{
			_Edata *p = reinterpret_cast<_Edata*>(get_expando());
			if (NULL != p)
			{
				set_expando(NULL);
				delete p;
			}
		}

		LPVOID ECtrl::GetData()const
		{
			_Edata *p = reinterpret_cast<_Edata*>(const_cast<ECtrl*>(this)->get_expando());
			return (NULL != p) ? (p->_data) : NULL;
		}

		void ECtrl::SetCheck( BOOL bCheck/*=TRUE*/,BOOL bUncheckSibling/*=FALSE*/ )
		{
			if (bCheck && bUncheckSibling)
			{
				ECtrl ep = parent();
				if (ep.is_valid())
				{
					ECtrl es = ep.find_first(":checked");
					if (es.is_valid() && ((HELEMENT)es != he))
					{
						es.toggle_state(STATE_CHECKED|STATE_FOCUS,false);
                        CTL_TYPE ct = es.get_ctl_type();
                        if ( (CTL_CHECKBOX == ct) || (CTL_RADIO == ct) )
                        {
                            es.set_value(json::value(false));
                        }
					}
				}
			}
			toggle_state(STATE_CHECKED|STATE_FOCUS,bCheck);
            CTL_TYPE ct = get_ctl_type();
            if ( (CTL_CHECKBOX == ct) || (CTL_RADIO == ct) )
            {
                set_value(json::value(bCheck));
            }
		}

		ECtrl ECtrl::FindFirstWithData( LPVOID pData )
		{
			int nChild = children_count();
			for ( int i = 0; i < nChild; i++)
			{
				if ((DWORD)ECtrl(child(i)).GetData() == (DWORD)pData)
				{
					return child(i);
				}
			}
			return NULL;
		}

		void ECtrl::FlashBorder( int nFlashTime /*= 3*/,
			UINT nMillsecods /*= 1000*/,
			LPCWSTR sBorderStyle/*=L"1px"*/)
		{
			nMillsecods = max(nMillsecods,300);
			nMillsecods = min(nMillsecods,2000);
			
			nFlashTime *= 2;
			nFlashTime = max(2,nFlashTime);

			const wchar_t* pOldStyle = get_style_attribute("border");
			set_attribute("old-border-style",pOldStyle);
			set_attribute("flash-border-style",sBorderStyle);
			set_attribute("flash-times",aux::itow(nFlashTime));

			start_timer(nMillsecods,ID_TIMER_ECTRL_FLASHBORDER);
		}

		ECtrl ECtrl::CreateWindowedWidget()
		{
			ECtrl ctlWindowed = dom::element::create("widget");
			ctlWindowed.set_attribute("type",L"windowed");
			return ctlWindowed;
		}

        BOOL ECtrl::SetFocus()
        {
            HWND hWnd = get_element_hwnd(true);
            if (hWnd != ::GetFocus())
            {
                if ( NULL == ::SetFocus(hWnd) )
                {
                    return FALSE;
                }
            }
            HELEMENT hCF = focus_element(hWnd);
            if (hCF != *this)
            {
                if (NULL != hCF)
                {
                    ECtrl(hCF).toggle_state(STATE_FOCUS, false);
                }
                toggle_state(STATE_FOCUS, true);
            }
            return TRUE;
        }

        ECtrl ECtrl::LastChild()
        {
            int nc = children_count();
            if ( nc > 0 )
            {
                return child(nc-1);
            }
            return NULL;
        }

        void ECtrl::SetHtml( LPCWSTR pszHtml ,int nWay)
        {
            utf8::ostream o;
            o<<pszHtml;
            set_html(o.data(),o.length(),nWay);
        }

        void ECtrl::SetText( LPCWSTR lpszFormat, ... )
        {
            const int __MAX_BUFFER_SIZE = 4096;
            TCHAR buf1[__MAX_BUFFER_SIZE] = {0};

            va_list va;
            va_start(va, lpszFormat);
            _vsntprintf_s(buf1, __MAX_BUFFER_SIZE - 1, lpszFormat, va);
            va_end(va);

            set_text(buf1);
        }

        BOOL ECtrl::SimulateMouseEvent( int cmd,UINT uMouseBtn /*= MAIN_MOUSE_BUTTON*/,
            POINT ptMouse /*= POINT(0,0)*/, UINT uKeybord /*= 0*/ )
        {
            MOUSE_PARAMS mp;
            mp.cmd = cmd;
            mp.target = he;
            mp.pos = ptMouse;
            mp.button_state = uMouseBtn;
            BOOL bProcessed = FALSE;
            HTMLayoutTraverseUIEvent(HANDLE_MOUSE, &mp, &bProcessed);
            return bProcessed;
        }

        BOOL ECtrl::SimulateKeybordEvent( UINT uKeycode /*= 0*/,
            UINT cmd /*= KEY_DOWN*/, UINT uAltState /*= 0*/ )
        {
            KEY_PARAMS mp;
            mp.cmd = cmd;
            mp.target = he;
            mp.key_code = uKeycode;
            mp.alt_state = uAltState;
            BOOL bProcessed = FALSE;
            HTMLayoutTraverseUIEvent(HANDLE_KEY, &mp, &bProcessed);
            return bProcessed;
        }

        void ECtrl::SetClass( LPCWSTR szClsName /*= NULL*/ )
        {
            if (NULL == szClsName)
            {
                remove_attribute("class");
            }
            else
            {
                set_attribute("class", szClsName);
            }
        }

        void ECtrl::AddClass( LPCWSTR szClsName )
        {
            if (!IsHasClass(szClsName))
            {
                SetClass(GetClass() + L" " + szClsName);
            }
        }

        void ECtrl::RemoveClass( LPCWSTR szClsName )
        {
            if (IsHasClass(szClsName))
            {
                CStdString sCls = GetClass();
                sCls.Replace(szClsName, L"");
                SetClass(sCls.Trim());
            }
        }

        BOOL ECtrl::IsHasClass( LPCWSTR szClsName )
        {
            if (NULL == szClsName)
            {
                ATLASSERT(FALSE);
                return FALSE;
            }

            CStdString sCls;
            sCls.Format(L" %s ", GetClass());
            CStdString sNew;
            sNew.Format(L" %s ", szClsName);

            return (-1 != sCls.Find(sNew));
        }

        //////////////////////////////////////////////////////////////////////////
        EOption EOption::InsertItem( LPCWSTR szItem, int idx /*= -1*/ )
        {
            EOption eopt = ECtrl::create("option");
            int c = children_count();
            if ((idx >= c) || (idx < 0))
                append(eopt);
            else
                insert(eopt,idx);
            eopt.set_value(json::value(szItem));
            return eopt;
        }

        EOption EOption::InsertItem_WithImageIdx( 
            LPCWSTR szItem,int iImageIdx, int idx /*= -1*/ )
        {
            EOption eopt = InsertItem(szItem,idx);
            if (eopt.is_valid())
            {
                ECtrl img = ECtrl::create("img");
                eopt.insert(img,0);
                img.set_attribute("img_idx",aux::itow(iImageIdx));
            }
            return eopt;
        }

        int EOption::GetItemImageIndex()
        {
            ECtrl img = find_first("img[img_idx]");
            if (img.is_valid())
            {
                return img.get_attribute_int("img_idx");
            }
            return -1;
        }

        EOption::EOption( HELEMENT h/*=NULL*/ )
            : ECtrl(h)
        {

        }

		//////////////////////////////////////////////////////////////////////////
        int EOptionsBox::SetCurSel( EOption& op )
        {
            if (!op.is_valid())
            {
                ATLASSERT(FALSE);
                return -1;
            }

            ECtrl cur_itm = GetCurSelItem();
            if (cur_itm == op) 
                return op.index();
            
            if (cur_itm.is_valid())
            {
                cur_itm.toggle_state(STATE_CHECKED,false);
                cur_itm.toggle_state(STATE_CURRENT,false);
                cur_itm.remove_attribute("selected");
            }

            op.toggle_state(STATE_CHECKED,true);
            op.toggle_state(STATE_CURRENT,true);
            op.set_attribute("selected",L"true");

            op.post_event(SELECT_SELECTION_CHANGED,1,he);

            return op.index();
        }

		EOption EOptionsBox::SelectItem_IDorName(LPCWSTR id_or_name )
		{
			if (id_or_name == nullptr)
            {
                ATLASSERT(FALSE);
                return NULL;
            }

			EOption item = find_first("option[id='%S'],option[name='%S']",
                id_or_name,id_or_name);
			if (item.is_valid())
			{
				SetCurSel(item);
			}
			return item;
		}

		EOption EOptionsBox::SelectItem_Text( LPCWSTR sValue )
		{
			if (nullptr == sValue)
            {
                ATLASSERT(FALSE);
                return NULL;
            }

            EOption op;
			int c = GetItemCount();
			for (int i = 0; i < c; i++)
			{
                op = child(i);
                if (op.GetText() == sValue)
				{
					SetCurSel(op);
                    return op;
				}
			}
			return NULL;
		}

		EOption EOptionsBox::SelectItem_Attribute( char* atti,LPCWSTR sValue )
		{
            if ( nullptr == atti )
            {
                ATLASSERT(FALSE);
                return NULL;
            }

            if (NULL == sValue)
            {
                return find_first("[%s]", atti);
            }

            return find_first("[%s='%S']", atti, sValue);
		}

		CStdString EOptionsBox::GetCurSelItemAttribute( const char* atti )
		{
			return GetItemAttribute(GetCurSelIndex(), atti);
		}

		void EOptionsBox::SetItemText( int idx,LPCWSTR sText )
		{
			ECtrl item = GetItem(idx);
			if (item.is_valid())
			{
				item.set_value(json::value(sText));
			}
		}

		void EOptionsBox::SetItemAttribute( int idx,LPCSTR satti,LPCWSTR sValue )
		{
			ECtrl item = GetItem(idx);
			if (item.is_valid())
			{
				item.set_attribute(satti,sValue);
			}
		}

		void EOptionsBox::SetItemStyleAttribute( int idx,LPCSTR satti,LPCWSTR sValue )
		{
			ECtrl item = GetItem(idx);
			if (item.is_valid())
			{
				item.set_style_attribute(satti,sValue);
			}
		}

		CStdString EOptionsBox::GetItemStyleAttribute( int idx,LPCSTR satti )
		{
			ECtrl item = GetItem(idx);
			if (item.is_valid())
			{
				return item.get_style_attribute(satti);
			}
			return L"";
		}

		int EOptionsBox::GetCurSelIndex()
		{
            EOption op = GetCurSelItem();
            if (op.is_valid())
            {
                return op.index();
            }
            return -1;
		}

		//////////////////////////////////////////////////////////////////////////
		BOOL ECombobox::SetCurSel( int idx /*= -1*/ )
		{
			idx = op_box_.SetCurSel(idx);
			if (idx >= 0)
			{
				caption_.set_value(ECtrl(op_box_.child(idx)).get_value());
				return TRUE;
			}
			return FALSE;
		}

		CStdString ECombobox::GetCurItemText()
		{
			int idx = GetCurSelIndex();
			if (-1 != idx)
			{
				return GetItemText(idx);
			}
			return L"";
		}

        ECombobox::ECombobox( HELEMENT h/*=NULL*/ )
            :ECtrl(h)
        {
            if (NULL != h)
            {
                ATLASSERT(get_ctl_type() == CTL_DD_SELECT);
                op_box_ = find_first("popup");
                ATLASSERT(op_box_.is_valid());
                caption_ = find_first("caption");
                ATLASSERT(caption_.is_valid());
            }
        }

        EOption EOptionsBox::GetItem( int idx )
        {
            if ( (idx >= GetItemCount()) || (idx < 0))
                return NULL;
            return child(idx);
        }

		CStdString EOptionsBox::GetItemText( int idx )
		{
			ECtrl item = GetItem(idx);
			if (item.is_valid())
			{
				return item.text().c_str();
			}
			return L"";
		}

		CStdString EOptionsBox::GetItemAttribute( int idx, const char* atti )
		{
			ECtrl item = GetItem(idx);
			if (item.is_valid())
			{
				return item.get_attribute(atti);
			}
			return L"";
		}

        EOption EOptionsBox::InsertItem( LPCWSTR sItem, int idx /*= INT_MAX */ )
        {
            ECtrl op = ECtrl::create("option");
            insert(op,idx);
            op.SetText(sItem);
            return op;
        }

		void EOptionsBox::DeleteAllItem()
		{
			ECtrl::DeleteAllChild();
		}

		void EOptionsBox::DeleteItem( int iItem )
		{
			ECtrl::DeleteChild(iItem,iItem);
		}

		BOOL EOptionsBox::SetItemData( int idx,LPVOID dwData )
		{
			int c = GetItemCount();
			if ((idx < 0) || (idx >= c))
				return FALSE;
			ECtrl item = GetItem(idx);
			item.SetData((LPVOID)dwData);
			return TRUE;
		}

		LPVOID EOptionsBox::GetItemData( int idx )
		{
            EOption op = GetItem(idx);
            if (op.is_valid())
            {
    			return op.GetData();
            }
            return NULL;
		}

		BOOL EOptionsBox::IsItemGroup( int idx )
		{
			ECtrl eItem = GetItem(idx);
			if (eItem.is_valid())
			{
				return aux::streqi(eItem.get_element_type(),"options");
			}
			return FALSE;
		}

        EOptionsBox::EOptionsBox( HELEMENT h/*=NULL*/ )
            :ECtrl(h)
        {

        }

        EOption EOptionsBox::SelectItem_ItemData( LPVOID dwData )
        {
            EOption op = FindFirstWithData(dwData);
            if (op.is_valid())
            {
                SetCurSel(op);
            }
            return op;
        }

        //////////////////////////////////////////////////////////////////////////
        EOption ETree::SetCurSel( EOption ti )
        {
            EOption eCurItem = find_first("option:checked");
            if ( ti.is_valid() )
            {
                if (eCurItem.is_valid() && (eCurItem != ti))
                {
                    eCurItem.toggle_state(STATE_CHECKED,false);
                    eCurItem.toggle_state(STATE_CURRENT,false);
                    eCurItem.remove_attribute("selected");
                }
                // 选中新的
                ti.toggle_state(STATE_CHECKED,true);
                ti.toggle_state(STATE_CURRENT,true);
                ti.set_attribute("selected",L"");

                ti.post_event(SELECT_SELECTION_CHANGED,1,he);
            }
            return eCurItem;
        }

        EOption ETree::GetCurSelIndex()
        {
            return find_first("option:checked");
        }

        EOption ETree::InsertItem( LPCWSTR sItem ,int idx /*= -1 */ )
        {
            return EOptionsBox::InsertItem(sItem, idx);
        }

        EOption ETree::InsertItem( EOption tiParent, LPCWSTR sItem,int idx /*= -1*/ )
        {
            ATLASSERT(tiParent.is_valid());
            if (tiParent.is_valid())
            {
                return tiParent.InsertItem(sItem,idx);
            }
            return NULL;
        }

        EOption ETree::GetChildItem( EOption tiParent )
        {
            if (tiParent.is_valid())
            {
                return tiParent.find_first("option");
            }
            return NULL;
        }

        EOption ETree::GetRootItem()
        {
            if (GetItemCount())
            {
                return child(0);
            }
            return NULL;
        }

        void ETree::Expand( EOption tItem )
        {
            if (!tItem.is_valid())
                return;
            SetCurSel(tItem);
            tItem.toggle_state(STATE_EXPANDED,true);
        }

        ETree::ETree( HELEMENT h/*=NULL*/ )
            :EOptionsBox(h)
        {
#ifdef _DEBUG
            if (NULL != h)
            {

            }
#endif
        }
        //////////////////////////////////////////////////////////////////////////
		void EEditCmb::SetValue( LPCWSTR sValue )
		{

		}

        EEditCmb::EEditCmb( HELEMENT h/*=NULL*/ )
            :ECombobox(h)
        {
#ifdef _DEBUG
            if (NULL != h)
            {
                ATLASSERT(nullptr != get_attribute("editable"));
            }
#endif
        }


		//////////////////////////////////////////////////////////////////////////
		EPageCtrl::EPageCtrl( HELEMENT h ) :ECtrl(h)
		{
			ATLASSERT(aux::wcseqi(L"pagenav",get_attribute("type")));
			m_root = ECtrl(h);
		}
		
		void EPageCtrl::SetPageNum( int nPage )
		{
			int pageNum = GetPageNum();
			if (nPage > pageNum)
			{
				for (int i = pageNum + 1; i <= nPage; i++)
				{
					ECtrl div = ECtrl::create("div",aux::itow(i));
					m_root.append(div);
					div.set_attribute("name",m_root.get_attribute("name"));
				}
			}
			else if (nPage < pageNum)
			{
				ECtrl((HELEMENT)m_root).DeleteChild(nPage);
				POINT pt = {0,0};
				m_root.set_scroll_pos(pt);
			}
			if (!GetCurPage() && nPage)
			{ // 如果没有选中项，那么设置第一个为选中项
				ECtrl(m_root.child(0)).SetCheck(TRUE);
			}
		}
		// page
		int EPageCtrl::GetCurPage()
		{
			ECtrl div = m_root.find_first("div:checked");
			return (div.is_valid()) ? (div.index() + 1) : 0;
		}
		// iPage - index [1,n]
		void EPageCtrl::SetCurPage(int ipage)
		{
			if ((ipage <= 0) || (ipage > GetPageNum()) )
			{
				return;
			}
			if (GetCurPage() != ipage)
			{
				ECtrl el = m_root.child(ipage-1);
				el.SetCheck(TRUE,TRUE);
				el.scroll_to_view(false);
			}
		}

		int EPageCtrl::GetPageNum()
		{
			return m_root.children_count();
		}

		//////////////////////////////////////////////////////////////////////////
		void EColorPicker::SetCOLORREF( DWORD dwColor )
		{
			ECtrl eTD = popup_.find_first("td:checked");
			if (eTD.is_valid())
			{
				eTD.toggle_state(STATE_CHECKED,false);
			}
			root_.set_style_attribute("background-color",COLORREF2HtmlColor(dwColor));
		}

		void EColorPicker::SetColorARGB( DWORD dwColor )
		{
			ECtrl eTD = popup_.find_first("td:checked");
			if (eTD.is_valid())
			{
				eTD.toggle_state(STATE_CHECKED,false);
			}
			root_.set_style_attribute("background-color",ColorARGB2HtmlColor(dwColor));
		}

		void EColorPicker::SetCOLORREF( LPCWSTR sColor )
		{
			root_.set_style_attribute("background-color",sColor);
		}

		DWORD EColorPicker::GetColorARGB()
		{
            ATLASSERT(FALSE);
            return 0;
		}
		
		DWORD EColorPicker::GetCOLORREF()
		{
            DWORD dwRet = 0;
            LPCWSTR cr = root_.get_style_attribute("background-color");
            if (NULL != cr)
            {
                dwRet = StrToInt(cr);
            }

            return dwRet;
		}

		CStdString EColorPicker::GetColor()
		{
            DWORD n = GetCOLORREF();
            CStdString sC;
            sC.Format(L"#%02X%02X%02X", GetRValue(n), GetGValue(n), GetBValue(n));
            return sC;
		}

        EColorPicker::EColorPicker( HELEMENT h/*=NULL*/ )
            :ECtrl(h)
        {
            if (NULL != h)
            {
                root_ = h;
                ATLASSERT(root_.is_valid());
                ATLASSERT(aux::wcseqi(root_.get_attribute("type"),L"colorpicker"));
                popup_ = root_.find_first("popup");
                ATLASSERT(root_.is_valid());
            }
        }

		//////////////////////////////////////////////////////////////////////////
		// FilePath
		CStdString EFilePath::GetFileName()
		{
			return ECtrl(find_first("caption")).text().c_str();
		}

		CStdString EFilePath::GetFilePath()
		{
            ECtrl cap = find_first("caption");
            return CStdString(cap.get_attribute("filename")).Trim();
		}

		void EFilePath::SetFilePath(LPCWSTR pszPath)
		{
			ECtrl eCap = ECtrl(find_first("caption"));
			eCap.set_attribute("filename",pszPath);
			eCap.set_attribute("title",pszPath);
			eCap.set_text(pszPath);
		}
		//////////////////////////////////////////////////////////////////////////
		// FilePath
		CStdString EFolderPath::GetFolderName()
		{
			return ECtrl(find_first("caption")).text().c_str();
		}

		CStdString EFolderPath::GetFolderPath()
		{
            ECtrl cap = find_first("caption");
            return CStdString(cap.get_attribute("filename")).Trim();
		}

		void EFolderPath::SetFolderPath( LPCWSTR pszPath )
		{
			ECtrl eCap = ECtrl(find_first("caption"));
			eCap.set_attribute("filename",pszPath);
			eCap.set_attribute("title",pszPath);
			eCap.set_text(pszPath);
		}

		//-----------------------------------
		// ETime
		QTime ETime::GetTime()
		{
            FILETIME ft = get_value().get_date();
            SYSTEMTIME st;
            FileTimeToSystemTime(&ft, &st);
			return st;
		}

        void ETime::SetTime( const QTime& t )
        {
            SetValue(json::value(t.Format(L"%H:%M:%S")));
        }

        ETime::ETime( HELEMENT h/*=NULL*/ )
            :ECtrl(h)
        {
#ifdef _DEBUG
            if (NULL != h)
            {
                ATLASSERT(get_ctl_type() == CTL_TIME);
            }
#endif
        }

		//////////////////////////////////////////////////////////////////////////
		void ENumber::SetLimit( int nMin,int nMax )
		{
			if (nMax < nMin)
			{
				ATLASSERT(FALSE);
				return;
			}
			set_attribute("minvalue",aux::itow(nMin));
			set_attribute("maxvalue",aux::itow(nMax));
		}

		void ENumber::SetStep( int nStep )
		{
			set_attribute("step",aux::itow(nStep));
		}

		int ENumber::GetNum()
		{
			return GetInt();
		}

        ENumber::ENumber( HELEMENT h/*=NULL*/ )
            :EEdit(h)
        {
#ifdef _DEBUG
            if (NULL != h)
            {
                auto t = get_ctl_type();
                ATLASSERT( (t == CTL_NUMERIC)
                    || ( CTL_DECIMAL == t));
            }
#endif
        }


		//////////////////////////////////////////////////////////////////////////
		// Progress
		int EProgress::GetPos()
		{
			return (int)get_value().get(0.0f);
		}

		void EProgress::SetPos( int nPos )
		{
			set_value(json::value((double)nPos));
		}

		void EProgress::SetRange( int nLow,int nHigh )
		{
			ATLASSERT(FALSE);
			// min=0 max=100
		}

		void EProgress::GetRange( __out int &nLow,__out int &nHigh )
		{
			ATLASSERT(FALSE);
		}

        EProgress::EProgress( HELEMENT h/*=NULL*/ )
            :ECtrl(h) 
        {

        }

        //////////////////////////////////////////////////////////////////////////
        void ETableRow::SetCellText( __in int iCol, LPCWSTR szText )
        {
            ETableCell cell = GetCell(iCol);
            if (cell.is_valid())
            {
                cell.SetText(szText);
            }
        }

        void ETableRow::SetCellHtml( __in int iCol, LPCWSTR szHtml )
        {
            ETableCell cell = GetCell(iCol);
            if (cell.is_valid())
            {
                cell.SetHtml(szHtml);
            }
        }

        ETableCell ETableRow::GetCell( __in int iCol )
        {
            if (iCol < GetColCount())
            {
                return child(iCol);
            }
            return NULL;
        }

        CStdString ETableRow::GetCellText( __in int iCol )
        {
            ETableCell cell = GetCell(iCol);
            if (cell.is_valid())
            {
                return cell.GetText();
            }
            return L"";
        }

		//////////////////////////////////////////////////////////////////////////
		int ETable::GetSelectedItemCount()
		{
			int nCount = 0;
			int nTotalRow = children_count();
			for ( int iRow = GetFixedRowCount(); iRow < nTotalRow; iRow++)
			{
				if (ECtrl(child(iRow)).IsChecked())
				{
					nCount++;
				}
			}
			return nCount;
		}

		ETableRow ETable::GetRow( int iRow )
		{
			if ((iRow < 0) || (iRow > GetRowCount()))
				return NULL;
			return child(_RealRowIndex(iRow));
		}

		ETableCell ETable::GetCell( int iRow,int iCol )
		{
			ETableRow cRow = GetRow(iRow);
			if (!cRow.is_valid())
				return NULL;
            return cRow.GetCell(iCol);
		}

		ETableRow ETable::GetSelected( int iBeginRow )
		{
			iBeginRow = max(0,iBeginRow);
			int nTotalRow = GetRowCount();
			for ( ; iBeginRow < nTotalRow; iBeginRow++)
			{
				ETableRow cRow = GetRow(iBeginRow);
				if (cRow.IsChecked())
				{
					return cRow;
				}
			}
			return NULL;
		}

        int ETable::GetAllSelected(__out std::vector<HELEMENT> & vItems)
        {
            struct _SelctedCB : public htmlayout::dom::callback
            {
            public:
                // 选择index 为 [row_after, ---) 后的
                _SelctedCB(std::vector<HELEMENT> *p,int row_after)
                    :_v(p),_row_after(row_after) {}
                std::vector<HELEMENT>* _v;
                int _row_after;
                virtual bool on_element(HELEMENT he) /* return false to continue enumeration*/
                {
                    // 过滤掉header
                    if (element(he).index() >= _row_after)
                    {
                        _v->push_back(he);
                    }
                    return false;
                }
            };
            select_elements(&_SelctedCB(&vItems,GetFixedRowCount()), L"tr:checked");
            return vItems.size();
        }

		LPVOID ETable::GetRowData(int iRow)
		{
			ETableRow cRow = GetRow(iRow);
			return cRow.is_valid() ? cRow.GetData() : NULL;
		}

		void ETable::SetRowData( int iRow,LPVOID p )
		{
			ETableRow cRow = GetRow(iRow);
			if (cRow.is_valid())
			{
				cRow.SetData(p);
			}
		}

		LPVOID ETable::GetCellData( int iRow,int iCol )
		{
			ETableCell cCell = GetCell(iRow,iCol);
			return cCell.is_valid() ? cCell.GetData() : NULL;
		}

		void ETable::SetCellData( int iRow,int iCol,LPVOID p )
		{
			ETableCell cCell = GetCell(iRow,iCol);
			if (cCell.is_valid())
			{
				cCell.SetData(p);
			}
		}

		ETableRow ETable::InsertRow( LPCWSTR szCells,int iRow )
		{
			ETableRow cRow = ECtrl::create("tr");
			if ((0 > iRow) || (iRow >= GetRowCount()))
				append(cRow);
			else
				insert(cRow,_RealRowIndex(iRow));
			
            cRow.SetHtml(szCells);

            return cRow;
		}

		void ETable::DeleteRow( int iRow )
		{
			ECtrl cRow = GetRow(iRow);
			if (cRow.is_valid())
			{
				cRow.destroy();
			}
		}

        void ETable::DeleteAllSelectedRow()
        {
            std::vector<HELEMENT> vSels;
            if (GetAllSelected(vSels) > 0)
            {
                for (int i = 0; i < vSels.size(); i++)
                {
                    ETableRow(vSels[i]).destroy();
                }
            }
        }

        ETableRow ETable::SelectRow( int idx,BOOL bSelect /*= TRUE*/ )
        {
            return SelectRow(GetRow(idx), bSelect);
        }

        ETableRow ETable::SelectRow( ETableRow tRow,BOOL bSelect /*= TRUE */ )
        {
            ETableRow tCur = GetSelected();
            if (!IsMultiSelect())
            {   // 非多选
                if (tCur.is_valid())
                { 
                    if (bSelect && (tCur == tRow))
                    { 
                        return tCur;
                    }
                    // 取消当前被选中项目
                    tCur.SetCheck(FALSE, FALSE);
                    tCur.set_state(0, STATE_CURRENT);
                }
            }

            if (tRow.is_valid())
            {
                tRow.SetCheck(bSelect, FALSE);
                tRow.toggle_state(STATE_CURRENT, bSelect);
            }
            return tCur;
        }

        void ETable::SetCellText( int iRow,int iCol,LPCWSTR pszText )
        {
            ETableCell cCell = GetCell(iRow,iCol);
            if (cCell.is_valid())
            {
                cCell.SetText(pszText);
            }
        }

        void ETable::SetCellText( ETableRow &cRow, int iCol,LPCWSTR pszText )
        {
            if (cRow.is_valid())
            {
                cRow.SetCellText(iCol, pszText);
            }
        }

        CStdString ETable::GetCellText( int iRow,int iCol )
        {
            ECtrl cCell = GetCell(iRow,iCol);
            if (cCell.is_valid())
            {
                return cCell.GetText();
            }
            return L"";
        }

        CStdString ETable::GetCellText( ETableRow &cRow ,int iCol )
        {
            if (cRow.is_valid())
            {
                ECtrl cell = cRow.child(iCol);
                if (cell.is_valid())
                {
                    return cell.GetText();
                }
            }
            return L"";
        }

        ETable::ETable( HELEMENT h/*=NULL*/ )
            :ECtrl(h)
        {
       
        }

		//////////////////////////////////////////////////////////////////////////
		ECtrl ETabCtrl::InsertTab( LPCWSTR szPanel,LPCWSTR szTab,int idx /*= -1*/ )
		{
            if (GetTab(szPanel).is_valid())
            {
                ATLASSERT(FALSE);
                return NULL;
            }

			ECtrl eTabPage = ECtrl::create("div");
			append(eTabPage);
			eTabPage.set_attribute("name",szPanel);

            return CreateTabOfPage(eTabPage, szTab, idx);
		}

        ECtrl ETabCtrl::CreateTabOfPage(__in ECtrl ePage, __in LPCWSTR szTabCaption, __in int idx )
        {
            // page 必须已经存在了
            if (!ePage.is_valid() || (ePage.parent() != *this))
            {
                ATLASSERT(FALSE);
                return NULL;
            }

            // 对应的tab应该不存在
            ECtrl eTab = GetTabPage(ePage);
            if (eTab.is_valid())
            {
                ATLASSERT(FALSE);
                return eTab;
            }

            // page ： name="xx"
            // tab ： panel="xx"
            LPCWSTR szTabName = ePage.get_attribute("name");
            if ((NULL == szTabName) || !wcslen(szTabName) )
            {
                ATLASSERT(FALSE);
                return NULL;
            }

            eTab = ECtrl::create("div", szTabCaption); 
            if ((idx < 0) || (idx >= GetTabCount()))
                strip_.append(eTab);
            else
                strip_.insert(eTab, idx);
            eTab.set_attribute("panel", szTabName);

            if ( IsShowCloseButton() )
            {
                ECtrl icon = ECtrl::create("div",L"r");
                eTab.append(icon);
                icon.set_attribute("class",L"icon");
                icon.set_attribute("name", get_attribute("show_close_button"));
            }

            return eTab;
        }

		int ETabCtrl::GetTabCount()
		{
			return strip_.children_count();
		}

		ECtrl ETabCtrl::GetTab( int idx )
		{
			int nTab = GetTabCount();
			if (nTab <= 0)
				return NULL;
			idx = max(0,idx);
			idx = min(idx,nTab);

			return strip_.child(idx);
		}

		ECtrl ETabCtrl::GetTab( LPCWSTR szPanel )
		{
			return strip_.find_first("[panel='%S']",szPanel);
		}

        ECtrl ETabCtrl::GetTabWithSelector( const char* szSelector )
        {
            return strip_.find_first(szSelector);
        }

		ECtrl ETabCtrl::GetTabPage( int idx )
		{
			return GetTabPage(GetTab(idx));
		}

		ECtrl ETabCtrl::GetTabPage( LPCWSTR szPanel )
		{
			return GetTabPage(GetTab(szPanel));
		}

		ECtrl ETabCtrl::GetTabPage( ECtrl& eTabItem )
		{
			if (!eTabItem.is_valid())
				return NULL;
			return find_first("[name='%S']",eTabItem.get_attribute("panel"));
		}

        ECtrl ETabCtrl::GetPageTab( __in ECtrl &ePage )
        {
            if (!ePage.is_valid())
                return NULL;
            return find_first("[panel='%S']", ePage.get_attribute("name"));
        }

		void ETabCtrl::SelectTab(int idx)
		{
			SelectTab(GetTab(idx));
		}

		void ETabCtrl::SelectTab( LPCWSTR szPanel )
		{
			SelectTab(GetTab(szPanel));
		}

		void ETabCtrl::SelectTab( ECtrl ctlTabItem )
		{
			if (ctlTabItem.is_valid())
			{
				HTMLayoutPostEvent(he, DO_SWITCH_TAB,ctlTabItem, 0);
			}
		}

		void ETabCtrl::RemoveTab( LPCWSTR szPanel )
		{
			RemoveTab(GetTab(szPanel));
		}

		void ETabCtrl::RemoveTab( int idx )
		{
			RemoveTab(GetTab(idx));
		}

		void ETabCtrl::RemoveTab( ECtrl tab )
		{
			if (tab.is_valid())
			{
				BOOL bIsCurrentTab = (tab == GetCurrentTab());
				GetTabPage(tab).destroy();
				tab.destroy();

				if (bIsCurrentTab && GetTabCount())
				{
					SelectTab(1);
				}
			}
		}

		void ETabCtrl::RemoveAllTabs()
		{
			// 清除tab
			strip_.DeleteAllChild();
			// 清除page
			DeleteChild(1,-1);
		}

		ECtrl ETabCtrl::GetCurrentTab()
		{
			return strip_.find_first("[selected]");
		}

		void ETabCtrl::ShowCloseButton( BOOL bShow/*=TRUE*/,LPCWSTR szButtonName/*=NULL*/ )
		{
			if (bShow)
				set_attribute("show_close_button",szButtonName);
			else
				remove_attribute("show_close_button");

			for (int i = 0, nTab = GetTabCount(); i < nTab; i++)
			{
				ShowItemCloseButton(GetTab(i),bShow,szButtonName);
			}
		}

		BOOL ETabCtrl::IsShowCloseButton()
		{
			return NULL != get_attribute("show_close_button");
		}

		ECtrl ETabCtrl::GetTabWithData( LPVOID pData )
		{
			return strip_.FindFirstWithData(pData);
		}

		void ETabCtrl::ShowItemCloseButton( int iItem,BOOL bShow/*=TRUE*/,LPCWSTR szButtonName/*=NULL*/ )
		{
			ShowItemCloseButton(GetTab(iItem),bShow,szButtonName);
		}

		void ETabCtrl::ShowItemCloseButton( ECtrl&tabItem,BOOL bShow/*=TRUE*/,LPCWSTR szButtonName/*=NULL*/ )
		{
			if (tabItem.is_valid())
			{
				ECtrl icon = tabItem.find_first(".icon");
				if (bShow)
				{
					if (!icon.is_valid())
					{ // add icon
						// 使用字符‘r’，会显示为‘X’，秘密在于css中使用的字体为“marlett”
						icon = ECtrl::create("div",L"r");	
						tabItem.append(icon);
						icon.set_attribute("class",L"icon");
						icon.set_attribute("name",szButtonName);
					}
				}
				else
				{ // destroy
					if (icon.is_valid())
					{
						icon.destroy();
					}
				}
			}
		}

        ETabCtrl::ETabCtrl( HELEMENT h/*=NULL*/ )
            : ECtrl(h)
        {
            if (NULL != h)
            {
                strip_ = find_first(".strip");
                //					ASSERT(m_strip.is_valid());
            }
        }

        ETabCtrl ETabCtrl::create()
        {
            ETabCtrl c = ECtrl::create("widget");
            c.set_attribute("type", L"tabs");
            return c;
        }

        //////////////////////////////////////////////////////////////////////////
		ETagItem ETagBox::AddTag(LPCWSTR szTagText,LPCWSTR szTagName)
		{
			ETagItem it = ECtrl::create("table");
			append(it);

			CStdString str;
			str.Format(L"<tr><td>%s</td><td .icon>r</td></tr>",szTagText);
			it.SetHtml(str);

			if (NULL != szTagName)
			{
				it.set_attribute("name",szTagName);
			}

            // 关闭按钮
            it.ShowCloseButton( _CloseButtonName() );

			return it;
		}

		BOOL ETagBox::CheckTag( int idx )
		{
			if ( (idx >= 0) && (idx < GetTagCount()) )
			{
				ETagItem(child(idx)).SetCheck(TRUE,TRUE);

				return TRUE;
			}
			return FALSE;
		}

		ETagItem ETagBox::GetCheckedTag()
		{
			return find_first(":checked");
		}

		ETagItem ETagBox::GetTag( int idx )
		{
			if ( (idx < 0) || (idx > GetTagCount()) )
			{
				ATLASSERT(FALSE);
				return NULL;
			}
			return child(idx);
		}

		ETagItem ETagBox::GetTag( LPCWSTR szTagID )
		{
			return find_first("table[id='%S']",szTagID);
		}

		void ETagBox::RemoveAllTag()
		{
			DeleteAllChild();
		}

        inline LPCWSTR ETagBox::_CloseButtonName()
        {
            return get_attribute("show_close_button");
        }

        void ETagBox::ShowCloseButton( LPCWSTR pszCloseBtnName/*=NULL*/ )
        {
            int nCount = GetTagCount();
            for (int i = 0; i < nCount; i++)
            {
                ETagItem(child(i)).ShowCloseButton(pszCloseBtnName);
            }

            if (NULL == pszCloseBtnName)
                remove_attribute("show_close_button");
            else
                set_attribute("show_close_button",pszCloseBtnName);
        }

        int ETagBox::GetAllCheckedTag( __out std::vector<ETagItem>& vtags )
        {
            vtags.clear();
            struct _select_checked : public htmlayout::dom::callback 
            {
                _select_checked(std::vector<ETagItem>* p) : pt(p) {}
                std::vector<ETagItem>* pt;
                virtual bool on_element(HELEMENT he) /* return false to continue enumeration*/
                {
                    pt->push_back(he);
                    return false;
                }
            }_sel(&vtags);
            select_elements(&_sel, "table:checked");
            return vtags.size();
        }

		void ETagItem::SetTag( LPCWSTR szText )
		{
			TextCell().SetText(szText);
		}

        void ETagItem::ShowCloseButton( LPCWSTR pszButtonName/*=NULL*/ )
        {
            ECtrl tTD = CloseButtonCell();
            if ( NULL == pszButtonName )
            {
                tTD.remove_attribute("name");
                tTD.ShowCtrl(SHOW_MODE_COLLAPSE);
            }
            else
            {
                tTD.set_attribute("name",pszButtonName);
                tTD.ShowCtrl(SHOW_MODE_SHOW);
            }
        }

        CStdString ETagItem::GetTag()
        {
            return TextCell().GetText();
        }


		//////////////////////////////////////////////////////////////////////////
#ifdef UIBASE_SUPPORT_FLASH

		void EFlash::SetSwf( LPCWSTR pszFilePath )
		{
			CFlashWnd *pFlash = GetFlashObject();
			if (NULL == pFlash)
			{
				ATLASSERT(FALSE);
				return;
			}

			pFlash->PlaySWF(pszFilePath);
		}

		LPCWSTR EFlash::GetSwfPath()
		{
			return get_attribute("src");
		}

		CFlashWnd * EFlash::GetFlashObject()
		{
			return reinterpret_cast<CFlashWnd*>(
                    QUIQuerySpecialCtrl(he,WIDGET_TYPE_FLASH));
		}

		void EFlash::SetSwfBkgndColor( DWORD dwRGB )
		{
			CFlashWnd *pFlash = GetFlashObject();
			if (NULL == pFlash)
			{
				ATLASSERT(FALSE);
				return;
			}

			pFlash->SetBkgndColor(dwRGB);
		}
#endif
		//////////////////////////////////////////////////////////////////////////
#ifdef UIBASE_SUPPORT_WEBBROWSER

		void EWebBrowser::GotoURL( LPCWSTR psURL )
		{
			CWebBrowser *pHost = GetBrowser();
			if (NULL == pHost)
			{
				ATLASSERT(FALSE);
				return;
			}

			pHost->GotoURL(psURL);
		}

		LPCWSTR EWebBrowser::GetURL()
		{
			return get_attribute("src");
		}

		void EWebBrowser::SetData( LPVOID pData )
		{
			ECtrl::SetData(pData);
		}

		CWebBrowser * EWebBrowser::GetBrowser()
		{
			return reinterpret_cast<CWebBrowser*>(
                        QUIQuerySpecialCtrl(he,WIDGET_TYPE_WEBBROWSER));
		}

#endif

        //////////////////////////////////////////////////////////////////////////
        void ELed::SetOnOff( int idx,BOOL bOn/*=TRUE*/ )
        {
            ECtrl led = GetLed(idx);
            if (led.is_valid())
            {
                led.SetCheck(bOn,FALSE);
            }
        }

        ECtrl ELed::GetLed( int idx )
        {
            return child(idx);
        }

        int ELed::GetCount()
        {
            return children_count();
        }

        void ELed::AddLed( int nCount, BOOL bOn)
        {
            ECtrl li;
            for (int i = 1; i <= nCount; i++)
            {
                li = ECtrl::create("li");
                append(li);
                li.SetCheck(bOn,FALSE);
            }
        }

        void ELed::RemoveLast( int nCount )
        {
            int ib = GetCount() - nCount;
            if ( ib < 0 )
                ib = 0;
            DeleteChild(ib, -1);
        }

        void ELed::RemoveFirst( int nCount )
        {
            DeleteChild(0, nCount-1);
        }

        void ELed::RemoveAll()
        {
            DeleteAllChild();
        }

        BOOL ELed::RegisterStatus( __in LPCSTR szStatus,__in LPCWSTR szImg )
        {
            if (NULL == szStatus)
            {
                ATLASSERT(FALSE);
                return FALSE;
            }
            set_attribute(szStatus,szImg);
            return TRUE;
        }

        BOOL ELed::SetStatus( __in int idx,__in LPCSTR szStatus )
        {
            ECtrl led = GetLed(idx);
            LPCWSTR szImg = get_attribute(szStatus);
            if (led.is_valid() && (NULL != szImg))
            {
                led.SetFrgndImage(szImg);
                return TRUE;
            }
            return FALSE;
        }


        //////////////////////////////////////////////////////////////////////////
        // treelist
        ETreeListItem ETreeList::InsertChild( __in HELEMENT heParent, 
            __in LPCWSTR szCells, __in int idx /*= -1*/ )
        {
            ETreeListItem cItem = ECtrl::create("tr");
            if (m_pTL->InsertChild(heParent, cItem, idx))
            {
                if (-1 == idx)
                    append(cItem);
                else
                    insert(cItem, idx);
                
                cItem.SetHtml(szCells);
                // 默认展开
                //cItem.set_attribute("tl_expand",L"1");
                cItem.toggle_state(STATE_EXPANDED, true);

                if ((NULL == heParent) || (heParent == *this))
                {
                    _SetLevel(cItem);
                }
                else
                {
                    ETreeListItem cParent(heParent);
                    cParent.set_attribute("tl_haschild",L"1");
                    _SetLevel(cParent, cItem);
                }

                return cItem;
            }
            return NULL;
        }

        ETreeListItem ETreeList::InsertChild( __in HELEMENT heParent,
            __in HELEMENT heItem, __in int idx /*= -1*/ )
        {
            if (NULL == heItem)
            {
                ATLASSERT(FALSE);
                return NULL;
            }

            ETreeListItem cItem = heItem;
            if (m_pTL->InsertChild(heParent, cItem, idx))
            {
                if (-1 == idx)
                    append(cItem);
                else
                    insert(cItem, idx);

                // 默认展开
                cItem.toggle_state(STATE_EXPANDED, true);

                if ((NULL == heParent) || (heParent == *this))
                {
                    _SetLevel(cItem);
                }
                else
                {
                    ETreeListItem cParent(heParent);
                    cParent.set_attribute("tl_haschild",L"1");
                    _SetLevel(cParent, cItem);
                }

                return cItem;
            }
            return NULL;
        }

        void ETreeList::_SetLevel( __in ETreeListItem& cParent, __in ETreeListItem& cRow )
        {
            int nLevel = GetItemLevel(cParent) + 1;
            cRow.set_attribute("tl_level", aux::itow(nLevel));
            // cRow.set_attribute("tl_parent", aux::itow(_HE2I(cParent)));
            ETableCell cCell = cRow.find_first("td");
            if (cCell.is_valid())
            {
                // 使用border来模拟margin。
                // padding不是一个好方法，因为foreground-image依然会显示在“0，0”处
                wchar_t sz[32] = {0};
                swprintf_s(sz, 32, L"%dpx solid transparent", _LevelMargin() * nLevel);
                cCell.set_style_attribute("border-left",sz);
            }
        }

        ETreeListItem ETreeList::GetCurrentItem()
        {
            return _Base::GetSelected(0);
        }

        ETreeListItem ETreeList::SetCurrentItem( ETreeListItem &cItem )
        {
            return _Base::SelectRow(cItem, TRUE);
        }

        ETreeListItem ETreeList::SetItemText( ETreeListItem&cItem, LPCWSTR szText )
        {
            ETreeListItem cParent = GetParentItem(cItem);
            cItem.SetHtml(szText);
            if ( cParent.is_valid() )
                _SetLevel(cParent, cItem);
            else
                _SetLevel(cItem);
            return cItem;
        }

        void ETreeList::DeleteItem( ETreeListItem &cItem )
        {
            ETreeListItem cParent = GetParentItem(cItem);
            if (m_pTL->DeleteItem(cItem))
            {
                if (cParent.is_valid() && (GetChildItemCount(cParent) <= 0))
                {
                    cParent.remove_attribute("tl_haschild");
                }
            }
        }

        BOOL ETreeList::CutTo( __in HELEMENT heItem, __in HELEMENT heDest )
        {
            // 先copy
            if (!CopyTo(heItem, heDest))
                return FALSE;

            // 再删除
            DeleteItem(ETreeListItem(heItem));
            return TRUE;
        }

        BOOL ETreeList::CopyTo( __in HELEMENT heItem, __in HELEMENT heDest )
        {
            // 源和目标相同，复制个毛啊？
            if (heItem == heDest)
                return TRUE;

            TreeHE tTree;
            if (  (NULL == heItem) || (NULL == heDest) || (heItem == *this) 
                || IsAncestor( heDest, heItem )  // 源节点不可以是目标节点的祖先
                || !m_pTL->GetNodeTree(heItem, tTree))
            {
                ATLASSERT(FALSE);
                return FALSE;
            }

            // 现在heItem的父亲就是heDest了
            tTree.reparent_root(heDest);
            LPVOID pData;
            ETreeListItem cItem, cNew;
            TreeHE::iterator i = tTree.begin();
            TreeHE::iterator iEnd = tTree.end();
            // 从heItem开始clone节点
            for (++i; i != iEnd; ++i )
            {
                cItem = *i;
                cNew = AppendChild(*tTree.parent(i), cItem.clone());
                if (!cNew.is_valid())
                {
                    ATLASSERT(FALSE);      // 没有恢复机制！理论上一定会成功的
                    break;
                }
                // 貌似关联data不能通过clone复制
                // 所以需要手动复制
                pData = cItem.GetData();
                if (nullptr != pData)
                {
                    cNew.SetData(pData);
                    cNew.remove_attribute("tl_hide");
                }
                // 这个节点被替换掉了 
                *i = (HELEMENT)cNew;
            }
            return TRUE;
        }

        ETreeList& ETreeList::operator=( const ETreeList&o )
        {
            this->he = o.he;
            m_pTL = o.m_pTL;
            return *this;
        }


        
        //////////////////////////////////////////////////////////////////////////
        ECtrl EOutlook::ExpandItem( LPCWSTR szItemSelector,BOOL bExpand/*=TRUE*/ )
        {
            ECtrl itNew = GetItem(szItemSelector);
            if ( itNew.is_valid() )
            {
                if ( !IsMultiExand() )
                {// 非多展开模式，要先collapse当前的item
                    ECtrl itCur = GetCurrentExpandItem();
                    if ((itCur != itNew) && itCur.is_valid())
                    {
                        itCur.toggle_state(STATE_EXPANDED,false);
                    }
                }
                itNew.toggle_state(STATE_EXPANDED,true);
            }
            return itNew;
        }

        void EOutlook::EnableMulitExpand( BOOL bEnable /*= TRUE*/ )
        {
            if (bEnable)
            {
                set_attribute("multi-expand",L"1");
            }
            else
            {
                remove_attribute("multi-expand");
            }
        }

        void EOutlook::ShowItem(LPCWSTR szItemSelector,BOOL bShow)
        {
            ECtrl itNew = GetItem(szItemSelector);
            if ( itNew.is_valid() )
            {
                itNew.ShowCtrl( bShow ? SHOW_MODE_SHOW : SHOW_MODE_COLLAPSE);
            }
        }

        ECtrl EOutlook::InsertItem( __in LPCWSTR szItem, 
            __in LPCWSTR szContTag/* = L"div"*/,__in int idx /*= INT_MAX*/ )
        {
            ATLASSERT(szContTag != NULL);
            ATLASSERT(idx >= 0);
            ECtrl li = create("li");
            insert(li, idx);
            CStdString s;
            s.Format(L"<caption>%s</caption><%s .content />", szItem, szContTag);
            li.SetHtml(s);
            return li;
        }

        ECtrl EOutlook::GetItemPanel( LPCWSTR szSelector )
        {
            ECtrl c = GetItem(szSelector);
            if (c.is_valid())
            {
                return c.find_first(":root>.content");
            }
            return NULL;
        }

        //////////////////////////////////////////////////////////////////////////
        EMenuItem EMenu::EnableMenuItem( LPCWSTR szSelector,BOOL bEnable/*=TRUE*/ )
        {
            EMenuItem eItem = find_first(szSelector);
            if ( eItem.is_valid() )
            {
                eItem.Enable(bEnable);
            }
            return eItem;
        }

        void EMenu::EnableAllMenuItem( BOOL bEnable /*= TRUE*/ )
        {
            struct _EnableCB : public htmlayout::dom::callback 
            {
                BOOL _bEnable;
                _EnableCB(BOOL b) : _bEnable(b){} 
                virtual bool on_element(HELEMENT he) /* return false to continue enumeration*/
                {
                    EMenuItem(he).Enable(_bEnable);
                    return false;
                }
            };
            select_elements(&_EnableCB(bEnable), L"li");
        }

        void EMenuItem::Enable( BOOL bEnable /*= TRUE */ )
        {
            EnableCtrl(bEnable);            
        }

        //////////////////////////////////////////////////////////////////////////
        BOOL ESheet::ShowCellWidget( ETableCell cCell )
        {
            if (cCell.is_valid())
            {
                _METHOD_PARAM_WITHDOM mp("show-cell-widget");
                mp.methodID = QUI_CUSTOM_METHOD_SHEET_SHOWCELLWIDGET;
                mp.h1 = (HELEMENT)*this;
                mp.h2 = cCell;
                call_behavior_method(&mp);

                return TRUE;
            }
            return FALSE;
        }

        void ESheet::CloseCellWidget( BOOL bUpdateContent /*= TRUE*/ )
        {
            _METHOD_PARAM_WITHDOM mp("close-cell-widget");
            mp.methodID = QUI_CUSTOM_METHOD_SHEET_CLOSECELLWIDGET;
            mp.h1 = (HELEMENT)*this;
            mp.h2 = NULL;
            mp.dw = bUpdateContent;
            call_behavior_method(&mp);
        }

        ETableCell ESheet::GetCurEditCell()
        {
            _METHOD_PARAM_WITHDOM mp("close-cell-widget");
            mp.methodID = QUI_CUSTOM_METHOD_SHEET_GETCUREDITCELL;
            mp.h1 = (HELEMENT)*this;
            mp.h2 = NULL;
            call_behavior_method(&mp);
            
            return mp.h2;
        }


        //////////////////////////////////////////////////////////////////////////
        BOOL ERadioGroup::CheckButton( int idx /*= -1*/ )
        {
            int n = GetButtonCount();
            if ((n <= 0) || (idx < -1))
                return FALSE;

            if ((-1 == idx) || (idx >= n))
                idx = n - 1;

            ECtrl btn = child(idx);
            if (btn.is_valid() && !btn.IsChecked())
            {
                btn.SetCheck(TRUE, TRUE);
                return TRUE;
            }
            return FALSE;
        }

        BOOL ERadioGroup::CheckButton( const char *sel )
        {
            ECtrl btn = find_first(sel);
            if (btn.is_valid() && !btn.IsChecked())
            {
                btn.SetCheck(TRUE, TRUE);
                return TRUE;
            }
            return FALSE;
        }

        ECtrl ERadioGroup::GetCheck()
        {
            return find_first(":root>div:checked");
        }


        //////////////////////////////////////////////////////////////////////////
        void ETextImage::SetImage( __in LPCWSTR szImg )
        {
            set_attribute("src", szImg);
            SetFrgndImage(szImg);
        }

        void ETextImage::SetText( LPCWSTR szText )
        {
            ECtrl div = find_first("div");
            if (div.is_valid())
            {
                div.SetText(szText);
            }
        }


        //////////////////////////////////////////////////////////////////////////
        EOption EWordInput::AddWordItem(__in LPCWSTR szItem) 
        {
            wordlst_.InsertItem(szItem);
            return NULL;
        }

        ECtrl EWordInput::FindWord(LPCTSTR szWord) 
        {
            struct foreach_word : public htmlayout::dom::callback
            {
                foreach_word(LPCWSTR wd, BOOL bCaseSentive)
                    :word_(wd), ignore_case_(bCaseSentive), hfond_(NULL)
                {

                }

                BOOL ignore_case_;
                HELEMENT hfond_;
                LPCWSTR word_;
                virtual bool on_element(HELEMENT he)
                {
                    if (ignore_case_)
                    {
                        if (ECtrl(he).GetText().CompareNoCase(word_) == 0)
                        {
                            hfond_ = he;
                            return true;
                        }
                    }
                    else
                    {
                        if (ECtrl(he).GetText() == word_)
                        {
                            hfond_ = he;
                            return true;
                        }
                    }
                    return false;
                }
            }_fi(szWord, IsWordCaseSentive());

            cap_.select_elements(&_fi, L":root>div.word");

            return _fi.hfond_;
        }

        ECtrl EWordInput::AddCaptionItem(__in LPCWSTR szItem) 
        {
            ECtrl c = FindWord(szItem);
            if (!c.is_valid())
            {
                c = cap_.create("div", szItem);
                cap_.append(c);
                c.SetClass(L"word");
            }
            return c;
        }

        void EWordInput::DeleteCheckedCaptionItem() 
        {
            ECtrl c = cap_.find_first("div:checked");
            if (c.is_valid())
            {
                c.destroy();
            }
        }

        void EWordInput::FilterSelectByInput()
        {
            CStdString wd = inpt_.GetText().Trim();

            struct foreach_item : public htmlayout::dom::callback
            {
                LPCWSTR word_;
                BOOL    case_;
                
                foreach_item(BOOL bCase, LPCWSTR wd)
                    :word_(wd), case_(bCase)
                {

                }

                virtual bool on_element(HELEMENT he)
                {
                    EOption op = he;
                    if (nullptr == word_)
                    {
                        op.ShowCtrl(SHOW_MODE_SHOW);
                    }
                    else
                    {
                        op.ShowCtrl( (op.GetText().Find(word_) != -1) 
                            ? SHOW_MODE_SHOW 
                            : SHOW_MODE_COLLAPSE);
                    }
                    return false;
                }

            }_fi(IsWordCaseSentive(), wd.IsEmpty() ? nullptr : (LPCWSTR)wd);

            wordlst_.select_elements(&_fi, "option");
        }

        CStdString EWordInput::GetCaptionItems( __in wchar_t chSep )
        {
            CStdString str;
            int n = cap_.children_count();
            for (int i = 0; i < n; ++i)
            {
                str.append(ECtrl(cap_.child(i)).GetText());
                str.append(1, chSep);
            }
            return str;
        }

        EWordInput::EWordInput( HELEMENT he /*= NULL*/ )
            : ECtrl(he)
        {
            popup_ = find_first("popup");
            cap_ = find_first("caption");
            wordlst_ = popup_.find_first("select");
            inpt_ = popup_.find_first("input");
        }

        void EQTimeCtrl::SetTime( const QTime& t /*= QTime::GetCurrentTime()*/ )
        {
            CStdString s = t.Format(L"%H:%M:%S");
            m_eCaption.set_value(json::value(s));
            //                 set_text(s.c_str());
            // 				set_value(json::value(s));
            m_ePopup.set_attribute("time",s);
        }

        QTime EQTimeCtrl::GetTime()
        {
            QTime tm;
            tm.ParseDateTime(m_eCaption.get_value().to_string().c_str());
            //tm.ParseDateTime(get_value().to_string().c_str());
            return tm;
        }

        EQTimeCtrl::EQTimeCtrl( HELEMENT h/*=NULL*/ ) 
            :ECtrl(h) 
        {
            if (NULL != h)
            {
                ATLASSERT(aux::wcseqi(L"timectrl",get_attribute("type")));
                m_eRoot = h;
                m_eCaption = m_eRoot.find_first("caption");
                ATLASSERT(m_eCaption.is_valid());
                m_ePopup = m_eRoot.find_first("table");
                ATLASSERT(m_ePopup.is_valid());
            }
        }

        void EDate::SetDate( const QTime& t )
        {
            SetValue(json::value(t.Format(L"%Y-%m-%d")));
        }

        QTime EDate::GetDate()
        {
            return QTime(get_value().get_date());
        }

        EDate::EDate( HELEMENT h/*=NULL*/ )
            :ECtrl(h)
        {
#ifdef _DEBUG
            if (NULL != h)
            {
                ATLASSERT(get_ctl_type() == CTL_DATE);
            }
#endif
        }

//////////////////////////////////////////////////////////////////////////
//         EShellTree::EShellTree( HELEMENT h )
//             : ECtrl(h)
//         {
//             ATLASSERT( aux::wcseqi(L"shell-tree", get_attribute("type")) );
//         }
// 
//         //////////////////////////////////////////////////////////////////////////
//         void EShellTreeItem::SetData( htmlayout::PSHELLITEMINFO pData )
//         {
//             _Edata *p = reinterpret_cast<_Edata*>(get_expando());
//             if (NULL == p)
//             {
//                 p = new _Edata(pData);
//                 set_expando(p);
//             }
//             else
//             {
//                 p->_data = pData;
//             }
//         }
// 
//         htmlayout::PSHELLITEMINFO EShellTreeItem::GetData()
//         {
//             _Edata *p = reinterpret_cast<_Edata*>(get_expando());
//             return (NULL != p) ? (p->_data) : NULL;
//         }


        EList::EList( HELEMENT h/*=NULL*/ )
            :EOptionsBox(h)
        {
#ifdef _DEBUG
            if (NULL != h)
            {
                CTL_TYPE t = get_ctl_type();
                ATLASSERT((CTL_SELECT_SINGLE==t) ///< Single select, ListBox or TreeView
                    ||(CTL_SELECT_MULTIPLE==t) ///< Multiselectable select, ListBox or TreeView.
                    /*||(CTL_DD_SELECT==t)*/	/// drop down select
                    );
            }
#endif
        }



        ETooltip::ETooltip( HELEMENT h/*=NULL*/ )
            :ECtrl(h)
        {
#ifdef _DEBUG
            if (NULL != h)
            {
                ATLASSERT(aux::wcseqi(get_attribute("class"),L"qtooltip"));
            }
#endif
        }

        //////////////////////////////////////////////////////////////////////////
        _qTLStruct::_qTLStruct( HELEMENT heRoot )
        {
            m_heTL = heRoot;
            ATLASSERT(ECtrl(m_heTL).is_valid());
            m_t.set_head(m_heTL);
        }

#ifdef _DEBUG
        void _qTLStruct::_DebugoutputTreeStruct()
        {
            debug_output_console dc;
            pre_itr iEnd = m_t.end();
            int nDepth = 0;
            for (pre_itr i = m_t.begin(); i != iEnd; ++i)
            {
                nDepth = m_t.depth(i);
                for (int d = 0; d < nDepth; ++d)
                {
                    dc.printf("   ");
                }
                dc.printf("%S\n", ETreeListItem(*i).GetCellText(0).c_str());
            }
        }
#endif

        int _qTLStruct::GetChildren( __in HELEMENT heParent, __out ArrayHE &arC )
        {
            arC.clear();

            pre_itr iPos;
            if ((NULL == heParent) || (heParent == m_heTL))
            {
                iPos = m_t.root();
            }
            else
            {
                if ( !_FindItem(heParent, iPos) )
                    return -1;
            }

            children_itr iEnd = m_t.end_child(iPos);
            for (children_itr i = m_t.begin_child(iPos); i != iEnd; ++i)
            {
                arC.push_back(*i);
            }
            return arC.size();
        }

        BOOL _qTLStruct::GetNodeTree( __in HELEMENT heItem, __out TreeHE& t )
        {
            pre_itr itr;
            if (!_FindItem(heItem, itr))
                return FALSE;

            m_t.subtree(t, itr);
            return TRUE;
        }

        HELEMENT _qTLStruct::GetNthChild( __in HELEMENT heParent, __in int idx /*= -1 */ )
        {
            pre_itr iPos;
            if (NULL == heParent)
                iPos = m_t.root();
            else if ( !_FindItem(heParent, iPos) )
                return NULL;

            int nChild = m_t.number_of_children(iPos);
            if ((nChild <= 0) || (idx >= nChild))
                return NULL;

            if (idx <= -1)  // 最后一个孩子，特殊优化
            {
                return *--m_t.end_child(iPos);
            }
            return *m_t.sibling(m_t.begin_child(iPos), idx);
        }

        HELEMENT _qTLStruct::GetNextSiblingItem( __in HELEMENT heItem )
        {
            ATLASSERT(IsValidTreeListItem(heItem));
            pre_itr iPos;
            if (_FindItem(heItem, iPos))
            {
                iPos = m_t.next_sibling(iPos);
                if (m_t.is_valid(iPos))
                {
                    return *iPos;
                }
            }
            return NULL;
        }

        HELEMENT _qTLStruct::GetPrevSiblingItem( __in HELEMENT heItem )
        {
            ATLASSERT(IsValidTreeListItem(heItem));
            pre_itr iPos;
            if (_FindItem(heItem, iPos))
            {
                iPos = m_t.previous_sibling(iPos);
                if (m_t.is_valid(iPos))
                {
                    return *iPos;
                }
            }
            return NULL;
        }

        int _qTLStruct::GetChildrenCount( __in HELEMENT heItem /*= NULL*/ )
        {
            pre_itr itr;
            if ((heItem == m_heTL) || (NULL == heItem))
                itr = m_t.root();
            else if (!_FindItem(heItem, itr))
                return -1;
            return m_t.number_of_children(itr);
        }

        HELEMENT _qTLStruct::GetParentItem( __in HELEMENT heItem )
        {
            ATLASSERT(IsValidTreeListItem(heItem));
            pre_itr itr;
            if ( _FindItem(heItem, itr) )
            {
                itr = m_t.parent(itr);
                if (m_t.is_valid(itr))
                {
                    HELEMENT he = *itr;
                    if (m_heTL != he)
                    {
                        // 根节点为TreeList本身的DOM
                        return he;
                    }
                }
            }
            return NULL;
        }

        BOOL _qTLStruct::InsertChild( __in HELEMENT heParent, __in HELEMENT heItem, __inout int& idx )
        {
            ATLASSERT(idx >= -1);

            if (heParent == m_heTL)
                heParent = NULL;

            pre_itr itr;
            if (NULL == heParent)
                itr = m_t.root();
            else if (!_FindItem(heParent, itr))
                return FALSE;  // 无效节点

            int nChild = m_t.number_of_children(itr);
            if ((nChild <= 0) || (idx >= nChild))
                idx = -1;   // 没有孩子，或者索引比孩子数多

            // 插入到的目标节点位置
            HELEMENT heTarget = NULL;
            if (-1 >= idx)      // 插入到最后位置
            {
                if (NULL == heParent) // 顶级节点，不必费劲找了，肯定是最后的位置
                {
                    m_t.append_child(itr, heItem);
                    return TRUE;
                }

                // 如果不是顶级节点
                // 在界面表示上，应该插入到父亲的兄弟节点位置
                while (true)
                {
                    heTarget = GetNextSiblingItem(heParent);
                    if (NULL == heTarget)
                    {
                        // 向上回溯
                        heParent = GetParentItem(heParent);
                        if (NULL == heParent)
                        {
                            break;
                        }
                    }
                    else
                    {
                        idx = ETreeListItem(heTarget).index();
                        break;
                    }
                }
                m_t.append_child(itr, heItem);
                return TRUE;
            }
            else
            {
                // 直接找到父亲的第x个孩子
                heTarget = *m_t.sibling(m_t.begin_child(itr), idx);
                idx = ETreeListItem(heTarget).index();
                return TRUE;
            }
            return FALSE;
        }

        int _qTLStruct::GetChildItemCount( __in HELEMENT heItem /*= NULL*/ )
        {
            if (NULL == heItem)
            {
                return m_t.number_of_children(m_t.root());
            }
            else
            {
                pre_itr i ;
                if (_FindItem(heItem, i))
                {
                    return m_t.number_of_children(i);
                }
                return -1;
            }
        }

        BOOL _qTLStruct::DeleteItem( __in HELEMENT heItem )
        {
            ATLASSERT(IsValidTreeListItem(heItem));
            ATLASSERT(heItem != m_heTL);
            // 删除所有的孩子节点
            pre_itr itr;
            if (_FindItem(heItem, itr))
            {
                pre_itr iEnd = m_t.end_pre(itr);
                for (pre_itr i = m_t.begin_pre(itr); i != iEnd; ++i)
                {
                    // 销毁节点
                    ETreeListItem(*i).destroy();
                }
                m_t.erase(itr);
                return TRUE;
            }
            return FALSE;
        }

        void _qTLStruct::ExpandItem( __in HELEMENT heItem )
        {
            pre_itr itr;
            if (!IsValidTreeListItem(heItem) || !_FindItem(heItem, itr))
            {
                ATLASSERT(FALSE);
                return ;
            }

            ETreeListItem cItem(heItem);
            if (IsItemExpand(cItem))
                return;

            // 父亲一定要展开
            pre_itr iEnd = m_t.root();
            pre_itr i = itr;
            for (; (iEnd != i) && m_t.is_valid(i); i = m_t.parent(i))
            {
                cItem = *i;
                // 自己设置展开标志
                cItem.ShowCtrl(SHOW_MODE_SHOW);
                cItem.toggle_state(STATE_EXPANDED, true);
                cItem.remove_attribute("tl_hide");
            }

            // 再根据需要展开自己所有的孩子
            iEnd = m_t.end_pre(itr);
            i = m_t.begin_pre(itr); // 指向的是实际是itr节点
            if (iEnd != i)
                ++i;
            for (; (iEnd != i) && m_t.is_valid(i); ++i)
            {
                cItem = *i;
                cItem.ShowCtrl(SHOW_MODE_SHOW);
                // 如果孩子以前就是展开的，那么展开之
                if ( IsItemHidingExpand(cItem))
                    cItem.toggle_state(STATE_EXPANDED, true);
                else // 跳过孩子节点的检查
                    i.skip_children();
                cItem.remove_attribute("tl_hide");
            }
        }

        void _qTLStruct::ExpandLeaf( __in HELEMENT heItem )
        {
            pre_itr iPos;
            if (!_FindItem(heItem, iPos))
                return;

            ExpandItem(heItem);

            // 孩子节点也要都展开
            pre_itr i = m_t.begin_pre(iPos);
            pre_itr iEnd = m_t.end_pre(iPos);
            ETreeListItem cItem;
            for ( ; i != iEnd; ++i)
            {
                cItem = *i;
                cItem.toggle_state(STATE_EXPANDED, true);
                cItem.ShowCtrl(SHOW_MODE_SHOW);
                cItem.remove_attribute("tl_hide");
            }
        }

        void _qTLStruct::ExpandAllLeaf()
        {
            pre_itr i = m_t.begin();
            pre_itr iEnd = m_t.end();
            ETreeListItem cItem;
            for ( ++i; i != iEnd; ++i)
            {
                cItem = *i;
                cItem.toggle_state(STATE_EXPANDED, true);
                cItem.ShowCtrl(SHOW_MODE_SHOW);
                cItem.remove_attribute("tl_hide");
            }
        }

        void _qTLStruct::CollapseItem( __in HELEMENT heItem )
        {
            ATLASSERT(IsValidTreeListItem(heItem));

            ETreeListItem cItem(heItem);
            pre_itr itr;
            if (!IsItemExpand(cItem) || !_FindItem(heItem, itr))
                return ;

            cItem.toggle_state(STATE_EXPANDED, false);

            pre_itr i = m_t.begin_pre(itr);
            pre_itr iEnd = m_t.end_pre(itr);
            if (i != iEnd)
                ++i;
            while ((i != iEnd) && m_t.is_valid(i))
            {
                cItem = *i;
                cItem.set_attribute("tl_hide",L"1");
                cItem.ShowCtrl(SHOW_MODE_COLLAPSE);

                ++i;
            }
        }

        void _qTLStruct::CollapseAll( __in int nLevel )
        {
            TreeHE::fixed_depth_iterator i = m_t.begin_fixed(m_t.root(), nLevel);
            for ( ; m_t.is_valid(i); ++i)
            {
                CollapseItem(*i);
            }
        }

        BOOL _qTLStruct::IsAncestor( __in HELEMENT heItem, __in HELEMENT heAncestor )
        {
            ATLASSERT(m_heTL != heAncestor);
            ATLASSERT((NULL != heItem) && (NULL != heAncestor));

            if (heItem == heAncestor)
                return FALSE;

            pre_itr itr, itrAnces;
            if (!_FindItem(heItem, itr) || !_FindItem(heAncestor, itrAnces))
            {
                ATLASSERT(FALSE);
                return FALSE;
            }

            return m_t.is_ancestor(itr, itrAnces);
        }

        BOOL _qTLStruct::CutTo( __in HELEMENT heItem, __in HELEMENT heDest )
        {
            if (  (NULL == heItem) || (NULL == heDest) || (heItem == m_heTL)
                || IsAncestor(heItem, heDest))  // 源节点不可以是目标节点的祖先
            {
                ATLASSERT(FALSE);
                return FALSE;
            }

            // 源和目标相同，剪切个毛啊？
            if (heItem == heDest)
                return TRUE;

            pre_itr itItem;
            pre_itr itDest;
            if (!_FindItem(heItem, itItem) || !_FindItem(heDest, itDest))
            {
                ATLASSERT(FALSE);
                return FALSE;
            }

            m_t.reparent(itDest, m_t.begin_pre(itItem), m_t.end_pre(itItem));
            return TRUE;
        }

        //////////////////////////////////////////////////////////////////////////
        _qTLStruct* _qTLStructMan::GetTLStruct( HELEMENT heRoot )
        {
            _qTLStructMan *p = _qTLStructMan::GetInstance();
            MapTLItr i = p->_Find(heRoot);
            if (p->_End() != i)
            {
                return i->second;
            }
            return nullptr;
        }

        _qTLStruct* _qTLStructMan::RegisterStruct( HELEMENT heRoot )
        {
            _qTLStructMan* pInst = _qTLStructMan::GetInstance();
            MapTLItr i = pInst->_Find(heRoot);
            if (_End() == i)
            {
                _qTLStruct *pRet = new _qTLStruct(heRoot);
                m_TLs[heRoot] = pRet;
                return pRet;
            }
            return i->second;
        }

        void _qTLStructMan::UnregisterStruct( HELEMENT heRoot )
        {
            _qTLStructMan* pInst = _qTLStructMan::GetInstance();
            MapTLItr i = pInst->_Find(heRoot);
            if (_End() != i)
            {
                delete i->second;
                m_TLs.erase(i);
            }
        }


        EEdit::EEdit( HELEMENT h/*=NULL*/ )
            :ECtrl(h)
        {
#ifdef _DEBUG
            if (NULL != h)
            {
                CTL_TYPE t = get_ctl_type();
                ATLASSERT((CTL_EDIT == t)
                    || (CTL_NUMERIC == t)
                    || (CTL_PASSWORD == t)
                    || (CTL_DECIMAL == t)
                    || (CTL_CURRENCY == t)
                    || (CTL_RICHTEXT == t)
                    || (CTL_TEXTAREA == t));
            }
#endif
        }

        bool EEdit::GetSelection( int& start, int& end )
        {
            TEXT_EDIT_SELECTION_PARAMS sp(false);
            if(!call_behavior_method(&sp))
                return false;
            start = sp.selection_start;
            end = sp.selection_end;
            return true;
        }

        bool EEdit::SelectText( int start /*= 0*/, int end /*= 0xFFFF */ )
        {
            TEXT_EDIT_SELECTION_PARAMS sp(true);
            sp.selection_start = start;
            sp.selection_end = end;
            return call_behavior_method(&sp);
        }

        bool EEdit::ReplaceSelection( const wchar_t* text, size_t text_length )
        {
            TEXT_EDIT_REPLACE_SELECTION_PARAMS sp;
            sp.text = text;
            sp.text_length = text_length;
            return call_behavior_method(&sp);
        }

        CStdString EEdit::GetText() const
        {
            TEXT_VALUE_PARAMS sp(false);
            if( const_cast<EEdit*>(this)->call_behavior_method(&sp) && sp.text && sp.length)
            {
                return CStdString(sp.text, sp.length);
            }
            return CStdString();
        }

        void EEdit::SetText( const wchar_t* text, size_t length )
        {
            TEXT_VALUE_PARAMS sp(true);
            sp.text = text;
            sp.length = length;
            call_behavior_method(&sp);
            update();
        }

        void EEdit::SetText( const wchar_t* text )
        {
            TEXT_VALUE_PARAMS sp(true);
            sp.text = text;
            sp.length = text?wcslen(text):0;
            call_behavior_method(&sp);
            update();
        }

        void EEdit::SetInt( int v )
        {
            wchar_t buf[64]  = {0};
            int n = _snwprintf(buf,63,L"%d", v);
            buf[63] = 0;
            SetText(buf,n);
        }

        void EEdit::SetDouble( double v )
        {
            wchar_t buf[64] = {0};
            int n = _snwprintf(buf, 63, L"%lf", v);
            buf[63] = 0;
            SetText(buf, n);
        }

        int EEdit::GetCharPos( int x, int y ) const
        {
            TEXT_EDIT_CHAR_POS_AT_XY_PARAMS sp;
            sp.x = x;
            sp.y = y;
            if(!const_cast<EEdit*>(this)->call_behavior_method(&sp))
                return -1;
            return sp.char_pos;
        }

        EPassword::EPassword( HELEMENT h/*=NULL*/ )
            :EEdit(h)
        {
#ifdef _DEBUG
            if (NULL != h)
            {
                ATLASSERT(get_ctl_type() == CTL_PASSWORD);
            }
#endif
        }

        ECheck::ECheck( HELEMENT h/*=NULL*/ )
            :ECtrl(h)
        {
#ifdef _DEBUG
            if (NULL != h)
            {
                ATLASSERT(get_ctl_type() == CTL_CHECKBOX);
            }
#endif
        }

        ERadio::ERadio( HELEMENT h/*=NULL*/ )
            :ECtrl(h)
        {
#ifdef _DEBUG
            if (NULL != h)
            {
                ATLASSERT(get_ctl_type() == CTL_RADIO);
            }
#endif
        }


        EStarBox::EStarBox( HELEMENT h/*=NULL*/ )
            :ECtrl(h)
        {
#ifdef _DEBUG
            if (NULL != h)
            {
                ATLASSERT(aux::wcseqi(L"starbox",get_attribute("type")));
            }
#endif
        }

        void EStarBox::SetCurSel( int nPri )
        {
            int nCount = children_count();
            nPri = (nPri>nCount)?nCount:nPri;
            set_attribute("index",aux::itow(nPri));
            refresh();
        }

    }
}

