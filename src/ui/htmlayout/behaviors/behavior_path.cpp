#include "behavior_aux.h"
#include <CommCtrl.h>
#include <math.h>

#ifdef _WIN32_WCE
#include <atlbase.h>
#include <atlapp.h>
#include <atlmisc.h>
#endif

namespace htmlayout 
{

	/*
	BEHAVIOR: path
	goal: Renders content of the element with path ellipsis.
	TYPICAL USE CASE:
	<td style="behavior:path; overflow-x:hidden; white-space:nowrap;text-overflow:ellipsis; ">
	SAMPLE:
	*/
    
	// Source: /Program Files/Test/Test/test.wav
	// Dest:   /Program Files/Te.../test.wav

#ifdef _WIN32_WCE

	static int StringWidth(LPCWSTR s, HDC dc)
	{
		SIZE size;
		GetTextExtentPoint32W(dc, s, wcslen(s), &size);
		return size.cx;
	}

	static int CharWidth(wchar_t c, HDC dc)
	{
		SIZE size;
		GetTextExtentPoint32W(dc, &c, 1, &size);
		return size.cx;
	}

	void FormatFileName(HDC dc, const QString &fileName, QString &dottedFileName, int boxWidth)
	{
		int stringWidth = StringWidth(fileName, dc);

		dottedFileName = fileName;

		// too small file name
		if( stringWidth < boxWidth ) return;

		// extract filename and path
		QString sPath = fileName;
		QString sFile = fileName;
		int i = sPath.ReverseFind('\\');
		if (i != -1)
		{
			sPath = sPath.Left(i);
			sFile.Delete(0, i);
		}
		else return; // nothing to format

		int pathWidth = StringWidth(sPath, dc);
		int fileWidth = stringWidth - pathWidth;
		if( fileWidth >= boxWidth )
		{
			dottedFileName = sFile;
			return; // too big file name
		}

		int dotsWidth = StringWidth(QString(L"...\\"), dc);
		int curWidth = dotsWidth;
		i = 0;
		while( (curWidth < (boxWidth - fileWidth)) && (i < sPath.GetLength()) )
		{
			curWidth += CharWidth(sPath[i++], dc);
		}
		if(curWidth >= boxWidth && i > 0)
			i--;

		sPath = sPath.Left(i);
		dottedFileName = sPath + L"..." + sFile;
	}

#endif

	struct path: public behavior
	{
		// ctor
		path(): behavior(HANDLE_BEHAVIOR_EVENT, "path") {}

        virtual BOOL on_event(HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason )
        {
            if (EDIT_VALUE_CHANGED == type)
            {
                dom::element el(he);
                json::string stext = el.text();
                el.set_attribute("filename", stext.c_str());
                el.set_attribute("title", stext.c_str());

                dom::element eparent = el.parent();
                eparent.set_attribute("filename", stext.c_str());
            }
            return false;
        }
        bool is_fullpath(dom::element &el)
        {
            BOOL bFullPath = (el.get_attribute("fullpath") != NULL);
            if ( !bFullPath )
            {
                dom::element ep = el.parent();
                bFullPath = (ep.is_valid())?(ep.get_attribute("fullpath") != NULL):FALSE;
            }
            return bFullPath;
        }

		virtual BOOL on_draw   (HELEMENT he, UINT draw_type, HDC hdc, const RECT& rc ) 
		{ 
			if( draw_type != DRAW_CONTENT )
				return FALSE; /*do default draw*/ 

			dom::element el = he;
			const wchar_t* filename = el.get_attribute("filename");
			if( !filename )
				return FALSE;  // no such attribute at all.

			// 先画图标
			SHFILEINFOW sfi;
			HIMAGELIST hlist = (HIMAGELIST) SHGetFileInfoW( filename,
				0, &sfi, sizeof(SHFILEINFO), 
				/*SHGFI_USEFILEATTRIBUTES |*/ SHGFI_SYSICONINDEX | SHGFI_SMALLICON );
			if(!hlist)
				return FALSE;
			int szx = 16;
			int szy = 16;
			ImageList_GetIconSize(hlist, &szx, &szy);
			int x = rc.left - 19; // 水平居左 // + (rc.right - rc.left - szx) / 2; 
			int y = rc.top + (rc.bottom - rc.top - szy) / 2; // 垂直居中

			//draw_file_icon(hdc, x, y, filename);
			ImageList_Draw(hlist, sfi.iIcon, hdc, x, y, ILD_TRANSPARENT);

			// 输出文字
			UINT pta = GetTextAlign(hdc);
			SetTextAlign(hdc, TA_LEFT | TA_TOP |TA_NOUPDATECP); 
			DrawTextW(hdc,  (is_fullpath(el) && (wcslen(filename) > 1)) ? filename : el.text(),
				-1,const_cast<RECT*>(&rc), 
				DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_PATH_ELLIPSIS | DT_NOPREFIX);
			SetTextAlign(hdc, pta); 

			return TRUE; /*skip default draw as we did it already*/ 
		}

	};


//     struct sysimg_n_text: public behavior
//     {
//         // ctor
//         sysimg_n_text(): behavior(HANDLE_DRAW, "sysimg-n-text") {}
// 
//         virtual BOOL on_draw(HELEMENT he, UINT draw_type, HDC hdc, const RECT& rc ) 
//         { 
//             if( draw_type != DRAW_CONTENT )
//                 return FALSE; /*do default draw*/ 
// 
//             dom::element el = he;
//             int idx = el.get_attribute_int("img_idx");
//             if( idx < 0 )
//                 return FALSE;  // no such attribute at all.
// 
//             // 先画图标
//             POINT pt;
//             pt.x = rc.left + 2;
//             pt.y = rc.top + (rc.bottom - rc.top - 16) / 2;
//             //CSystemImageList::GetInstance()->DrawSmallIcon(hdc, idx, pt);
//             
//             // 输出文字
//             WTL::CRect rcText = rc;
//             rcText.left += 20;
//             UINT pta = GetTextAlign(hdc);
//             SetTextAlign(hdc, TA_LEFT | TA_TOP | TA_NOUPDATECP); 
//             DrawTextW(hdc, el.text(), -1, &rcText, 
//                 DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_PATH_ELLIPSIS | DT_NOPREFIX);
//             SetTextAlign(hdc, pta); 
// 
//             return TRUE; /*skip default draw as we did it already*/ 
//         }
//     };
	// instantiating and attaching it to the global list
	path path_instance;
//    sysimg_n_text sysimg_n_text_instance;

} // htmlayout namespace





