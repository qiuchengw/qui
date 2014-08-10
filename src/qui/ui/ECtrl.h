#ifndef ECtrl_h__
#define ECtrl_h__

#pragma once

//forcing value to bool 'true' or 'false' (performance warning)
// 4800:    “type”: 将值强制为布尔值“true”或“false”(性能警告)
// 4927:    转换非法；隐式应用了多个用户定义的转换，ECtrl -> ETooltip
//          通过添加Etooltip(ECtrl& t) 可以解决这个问题。
//          但是代码太多了。等vs支持了委托构造函数的时候再修改代码
#pragma warning(disable:4800 4927)

// // #include "QUI.h"

#include <atldef.h>
#include "time/QTime.h"
#include "BaseType.h"
#include "htmlayout/htmlayout.h"
#include "htmlayout/htmlayout_dom.h"
#include "htmlayout/htmlayout_behavior.h"
// #include <shlobj.h>
// #include <atlshellext.h>
#include "tree.hh"

// 用于支持可拖拽dockpane的tab名
#define QUI_DOCKPANE_TAB_NAME   L"dockpane_tab__qui_"

class CFlashWnd;
class QView;
class CWebBrowser;

/** 从COLORREF转换到html能够直接使用的颜色
*      Dword -> #xxxxxx
*	return
*      Stirng
*	param
*		-[in]
*          dwColor         颜色值
**/
inline CStdString COLORREF2HtmlColor(__in COLORREF dwColor)
{
    CStdString sRet;
    sRet.Format(L"#%02X%02X%02X",
        GetRValue(dwColor),
        GetGValue(dwColor),
        GetBValue(dwColor));
    return sRet;
}

/** 从ARGB转换到html能够直接使用的颜色
*      Dword -> #xxxxxx
*	return
*      Stirng
*	param
*		-[in]
*          dwColor         颜色值
**/
inline CStdString ColorARGB2HtmlColor(__in COLORREF dwColor)
{
    CStdString sRet;
    sRet.Format(L"#%02X%02X%02X",
        (dwColor>>16)&0xff,
        (dwColor>>8)&0xff,
        dwColor&0xff);
    return sRet;
}

namespace htmlayout
{

    struct treelist;
    class CHLShellTree;
    // The PARAM data of all controls
    // 用于shell tree
//     typedef struct
//     {
//         CComPtr<IShellFolder> spFolder;
//         CPidl pidlFull;
//         CPidl pidlNode;
//         DWORD dwAttribs;
//     } SHELLITEMINFO, *PSHELLITEMINFO;

    struct _METHOD_PARAM_WITHDOM : public XCALL_PARAMS
    {
        _METHOD_PARAM_WITHDOM(LPCSTR name)
            :XCALL_PARAMS(name),h1(NULL),h2(NULL),h3(NULL)
        {
        }

        HELEMENT    h1;
        HELEMENT    h2;
        HELEMENT    h3;
        DWORD       dw;     // 用户自定义flag
    };

    enum _ENUM_QUI_CUSTOM_METHOD
    {
        /** behavior：sheet
        *      显示cell-widget
        *	param
        *		-[in]
        *              _METHOD_PARAM_WITHDOM*
        *                  h1      table
        *                  h2      cell
        **/
        QUI_CUSTOM_METHOD_SHEET_SHOWCELLWIDGET = FIRST_APPLICATION_METHOD_ID,
        /** behavior：sheet
        *      关闭cell-widget
        *	param
        *		-[in]
        *              _METHOD_PARAM_WITHDOM*
        *                  h1      table
        *                  h2      cell
        *                  dw      TRUE    更新显示
        **/
        QUI_CUSTOM_METHOD_SHEET_CLOSECELLWIDGET,
        /** behavior：sheet
        *      获取当前正在显示cell-widget的那个cell
        *	param
        *		-[in]
        *              _METHOD_PARAM_WITHDOM*
        *                  h1      table
        *      -[out]
        *                  h2      cell    ,NULL, 如果为空
        **/
        QUI_CUSTOM_METHOD_SHEET_GETCUREDITCELL,

    };

    enum
    {
        // 用于闪烁DOM元素边框的定时器
        ID_TIMER_ECTRL_FLASHBORDER = 0x0000FFF1,
    };

    namespace dom
    {
        enum _CTRL_SHOW_MODE
        {
            SHOW_MODE_SHOW, // visibility: visible
            SHOW_MODE_HIDE,// visibility: hidden
            SHOW_MODE_COLLAPSE,	 // visibility: collapse
        };

        class ECtrl : public element
        {
            typedef element base;
            class _Edata : public htmlayout::dom::expando
            {
            public:
                _Edata(LPVOID _d) : _data(_d) {}
                virtual ~_Edata() { }
                virtual void finalize()
                {
                    delete this;
                }

            public:
                LPVOID	_data;
            };
        public:
            ECtrl(HELEMENT h=NULL):element(h) {};

            ECtrl& operator=(HELEMENT h);

            ECtrl& operator=(const ECtrl& e);

            ~ECtrl(void) {};

            CStdString ID()
            {
                return get_attribute("id");
            }

            CStdString Name()
            {
                return get_attribute("name");
            }

            inline void SetName(LPCWSTR pszName)
            {
                set_attribute("name",pszName);
            }

            inline void SetID(LPCWSTR pszID)
            {
                set_attribute("id",pszID);
            }
            // 创建实体窗口，<widget type="windowed">
            static ECtrl CreateWindowedWidget();

            BOOL SetFocus();

            // nWay 插入方式，参考HTMLayoutSetElementHtml
            void SetHtml(LPCWSTR pszHtml,int nWay = SIH_REPLACE_CONTENT);

            // pData 关联数据
            void SetData(LPVOID pData);

            // 移除关联数据
            void RemoveData();

            LPVOID GetData()const ;

            void FlashBorder(int nFlashTime = 3,	// 闪烁次数
                UINT nMillsecods = 500,	// 闪烁频率，ms
                LPCWSTR sBorderStyle=L"1px solid red");

            ECtrl FindFirstWithData(LPVOID pData);

            void SetWidth(int nx,LPCWSTR unit=L"px");

            void SetHeight(int ny,LPCWSTR unit=L"px");

            void SetSize(int nx,int ny,LPCWSTR unit=L"px");

            void EnableCtrl(BOOL bEnable = TRUE);
            //
            // nDir 为方向
            //   0-自动计算,偏向于下方

            /** 直接在控件周围显示提示内容，
            *	-params:
            *		-[in]
            *			sTip	可以为html代码
            *			nDir	显示在控件哪儿？
            *			    2 - popup ECtrl below of anchor
            *				8 - popup ECtrl above of anchor
            *				4 - popup ECtrl on left side of anchor
            *				6 - popup ECtrl on right side of anchor
            **/
            void ShowTooltip(LPCWSTR sTip,int nDir=2);

            /** 设置鼠标悬浮时自动显示的文本内容
            *		-[in]
            *			sTipText	纯文本，非html代码
            *			如果为NULL则删除提示
            **/
            void SetTooltipText(LPCWSTR sTipText=NULL);

            BOOL IsChecked()const
            {
                return get_state(STATE_CHECKED);
            }

            // bCheck=TRUE 且 bUncheckSibling = TRUE ，
            //	选中本身的时候将当前具有state_checked的兄弟节点反选掉
            //  这对于父亲具有behavior：switch是很方便的
            void SetCheck(BOOL bCheck=TRUE,BOOL bUncheckSibling=FALSE);

            BOOL IsHasAttribute(LPCSTR satti)
            {
                return get_attribute(satti) != NULL;
            }

            void SetText(LPCWSTR szFormat, ...);

            // 这个方法获取的数据可能是不正确的
            // 不同的控件应该根据控件的特点实现自己的GetText();
            virtual CStdString GetText()
            {
                return text().c_str();
            }

            void SetValue(const json::value& v)
            {
                set_value(v);
            }

            /** 显示或者隐藏窗口
            *		-[in]	_CTRL_SHOW_MODE
            **/
            void ShowCtrl(_CTRL_SHOW_MODE nMode);

            void DeleteAllChild();

            // 删除索引在[ibegin,iend]的孩子
            // iend:-1 ，最后一个
            void DeleteChild(int ibegin,int iend=-1);

            void ClearBkgndStyle();

            void ClearFrgndStyle();

            void SetCOLORREF(LPCWSTR sColor);

            void SetBkgndColor(LPCWSTR sColor);

            void SetFrgndColor(LPCWSTR sColor);

            void SetBkgndColor(LPCWSTR sTop,LPCWSTR sRight,
                LPCWSTR sBottom,LPCWSTR sLeft);

            void SetFrgndColor(LPCWSTR sTop,LPCWSTR sRight,
                LPCWSTR sBottom,LPCWSTR sLeft);

            void SetBkgndImage(LPCWSTR  sImg);

            void SetFrgndImage(LPCWSTR  sImg);

            // repeat/no-repeat/repeat-x/repeat-y/expand/stretch;
            void SetBkgndRepeat(LPCWSTR sRepeat);

            void SetFrgndRepeat(LPCWSTR sRepeat);

            void SetBkgndPosition(int x,int y);

            void SetFrgndPosition(int x,int y);

            void SetBkgndPosition(int t,int r,int b,int l);

            void SetFrgndPosition(int t,int r,int b,int l);

            void SetBkgndPosition(LPCWSTR sPt);

            void SetFrgndPosition(LPCWSTR sPt);

            // class
            inline CStdString GetClass()
            {
                return get_attribute("class");
            }

            void SetClass(LPCWSTR szClsName = NULL);

            void AddClass(LPCWSTR szClsName);

            void RemoveClass(LPCWSTR szClsName);

            BOOL IsHasClass(LPCWSTR szClsName);

            // 0 不透明， 100 全透明
            void SetTransparent(int nAlpha = 0);

            ECtrl LastChild();
            /** 模拟鼠标事件
            *	return
            *      TRUE    成功
            *	param
            *		-[in]
            *          uMouseBtn       MOUSE_BUTTONS
            *          ptMouse         鼠标点击位置
            *          uKeybord        KEYBOARD_STATES
            **/
            BOOL SimulateMouseEvent(int cmd,UINT uMouseBtn = MAIN_MOUSE_BUTTON,
                POINT ptMouse = POINT(), UINT uKeybord = 0);

            /** 模拟键盘事件
            *	return
            *      TRUE    成功
            *	param
            *		-[in]
            *          cmd         KEY_EVENTS
            *          uKeycode    key scan code, or character unicode for KEY_CHAR
            *          uAltState   KEYBOARD_STATES
            **/
            BOOL SimulateKeybordEvent(UINT uKeycode = 0,UINT cmd = KEY_DOWN, UINT uAltState = 0);
        };

        typedef ECtrl ETableCell;

        class ETableRow : public ECtrl
        {
        public:
            ETableRow(HELEMENT h=NULL):ECtrl(h) {}

            void SetCellText(__in int iCol, LPCWSTR szText);

            void SetCellHtml(__in int iCol, LPCWSTR szHtml);

            CStdString GetCellText(__in int iCol);

            // 索引范围 [0, col_count)
            ETableCell GetCell(__in int iCol);

            inline int GetColCount()const
            {
                return children_count();
            }
        };

        class ETable : public ECtrl
        {
        public:
            ETable(HELEMENT h=NULL);

        public:
            int GetSelectedItemCount();

            // iBeginRow 从第几行开始搜索，0 开始的索引
            // 无论iBeginRow为什么值，都会跳过表头搜索
            ETableRow GetSelected( int iBeginRow = 0 );

            /** 获取所有被选中的行
            *	return:
            *      选中的行数
            *	params:
            *		-[out]
            *          vItems  选中的行
            **/
            int GetAllSelected(__out std::vector<HELEMENT> & vItems);

            // iRow , 0 开始的索引号
            ETableRow GetRow(int iRow);

            inline int GetRowIndex( ETableRow &cRow )
            {
                ATLASSERT(cRow.is_valid() && (cRow.parent() == he));
                return cRow.index() - GetFixedRowCount();
            }

            LPVOID GetRowData(int iRow);

            void SetRowData(int iRow,LPVOID p);
            // iRow , 0 开始
            // iCol, 0 开始
            ETableCell GetCell( int iRow,int iCol );

            LPVOID GetCellData(int iRow,int iCol);

            void SetCellData(int iRow,int iCol,LPVOID p);

            void SetCellText(int iRow,int iCol,LPCWSTR pszText);

            void SetCellText( ETableRow &cRow, int iCol,LPCWSTR pszText );

            CStdString GetCellText(int iRow,int iCol);

            CStdString GetCellText( ETableRow &cRow ,int iCol );

            inline int GetFixedRowCount()const
            {
                return get_attribute_int("fixedrows");
            }

            inline int GetRowCount()const
            {
                return children_count() - GetFixedRowCount();
            }

            // -1 最后一行
            ETableRow InsertRow( LPCWSTR szCells,int iRow = INT_MAX );

            void DeleteRow(int iRow);

            void DeleteAllSelectedRow();

            void DeleteAllRow()
            {
                DeleteChild(GetFixedRowCount());
            }

            /** 选中项目
            *	return
            *      之前选中的项目
            *	param
            *		-[in]
            *          idx     需要被选中的项目索引
            *          cRow
            *          bSelect TRUE 选中
            **/
            ETableRow SelectRow( int idx,BOOL bSelect = TRUE );

            ETableRow SelectRow( ETableRow cRow,BOOL bSelect = TRUE );

            inline BOOL IsMultiSelect()
            {
                return get_attribute("multiple") != nullptr;
            }

        protected:
            inline int _RealRowIndex(int iRow)
            {
                return iRow + GetFixedRowCount();
            }
        };

        class ETooltip : public ECtrl
        {
        public:
            ETooltip(HELEMENT h=NULL);

        };

        class EOption : public ECtrl
        {
        public:
            EOption(HELEMENT h=NULL);

        public:
            EOption InsertItem(LPCWSTR szItem, int idx = -1);
            /** 对于有关联ImageList，需要自绘图标的才有用
            *	return:
            *      插入的Item
            *	params:
            *		-[in]
            *          szItem      item text
            *          iImageIdx   关联ImageList的索引
            *          idx         插入位置
            **/
            EOption InsertItem_WithImageIdx(LPCWSTR szItem,int iImageIdx, int idx = -1);
            /** 使用InsertItem_WithImageIdx插入时的图像索引
            *	return:
            *      图像索引值
            *      -1      无效（可能没有关联的img）
            *      >=0     有效值
            **/
            int GetItemImageIndex();
        };

        // 容纳<options>/<option>
        // 例如：dropdown 的popup
        class EOptionsBox : public ECtrl
        {
        public:
            EOptionsBox(HELEMENT h=NULL);

        public:
            int GetItemCount()
            {
                return children_count();
            }

            // idx: -1 last item, 0 first item
            // return : >= 0 选中的项目的索引
            //			-1	未找到索引项
            int SetCurSel( __in int idx = -1 )
            {
                return SetCurSel(GetItem(idx));
            }

            int SetCurSel(__in EOption& op);

            // index
            inline int GetItemIndex(__in EOption& op)
            {
                if (!op.is_valid())
                {
                    ATLASSERT(FALSE);
                    return -1;
                }
                return op.index();
            }

            // -1 none selected
            int GetCurSelIndex();

            inline EOption GetCurSelItem()
            {
                return find_first("option:checked,options:checked");
            }

            // 元素类型是否是options
            BOOL IsItemGroup(int idx);

            EOption GetItem( int idx );

            CStdString GetItemText(int idx);

            void SetItemText(int idx,LPCWSTR sText);

            CStdString GetItemAttribute(int idx,const char* atti);

            void SetItemAttribute(int idx,LPCSTR satti,LPCWSTR sValue);

            CStdString GetItemStyleAttribute( int idx,LPCSTR satti );

            void SetItemStyleAttribute(int idx,LPCSTR satti,LPCWSTR sValue);

            CStdString GetCurSelItemAttribute(const char* atti);

            EOption SelectItem_IDorName(LPCWSTR sIDorName );

            EOption SelectItem_Attribute(char* atti,LPCWSTR sValue = NULL );

            EOption SelectItem_Text( LPCWSTR sValue );

            EOption SelectItem_ItemData(LPVOID dwData);

            EOption InsertItem( LPCWSTR sItem, int idx = INT_MAX );

            void DeleteItem(int iItem);

            void DeleteAllItem();

            BOOL SetItemData( int idx,LPVOID dwData );

            inline void SetItemData( __in EOption& op, LPVOID dwData )
            {
                op.SetData(dwData);
            }

            LPVOID GetItemData(int idx);

            inline LPVOID GetItemData(__in EOption& op)
            {
                return op.GetData();
            }

        };

        class EProgress : public ECtrl
        {
        public:
            EProgress(HELEMENT h=NULL);

            int GetPos();

            void SetPos(int nPos);

            void SetRange(int nLow,int nHigh);

            void GetRange(__out int &nLow,__out int &nHigh);
        };

        //////////////////////////////////////////////////////////////////////////
        class EList : public EOptionsBox
        {
        public:
            EList(HELEMENT h=NULL);

        };

        class ETree : public EOptionsBox
        {
        public:
            ETree(HELEMENT h=NULL);

        public:
            // 选中
            EOption SetCurSel(EOption ti);
            EOption GetCurSelIndex();
            void Expand(EOption tItem);
            // 在顶级插入
            EOption InsertItem( LPCWSTR sItem ,int idx = -1 );
            // 在tiParent下插入
            EOption InsertItem( EOption tiParent, LPCWSTR sItem,int idx = -1);

            EOption GetChildItem(EOption tiParent);
            EOption GetRootItem();
            EOption GetChildItem_WithData( EOption tiParent , LPVOID pData);
        };

//         class EShellTreeItem : public EOption
//         {
//             friend class htmlayout::CHLShellTree;
//             class _Edata : public htmlayout::dom::expando
//             {
//             public:
//                 _Edata(htmlayout::PSHELLITEMINFO _d) : _data(_d) {}
//                 virtual ~_Edata()
//                 {
//                     delete _data;
//                 }
//                 virtual void finalize()
//                 {
//                     delete this;
//                 }
//             public:
//                 htmlayout::PSHELLITEMINFO	_data;
//             };
// 
//         public:
//             EShellTreeItem(HELEMENT h): EOption(h)
//             {
// 
//             }
// 
//             CStdString GetFullPath()
//             {
//                 std::wstring s;
//                 GetData()->pidlFull.GetPath(s);
//                 return s;
//             }
// 
//         protected:
//             void SetData(htmlayout::PSHELLITEMINFO pData);
// 
//             htmlayout::PSHELLITEMINFO GetData();
//         };
// 
//         class EShellTree : public ECtrl
//         {
//         public:
//             EShellTree(HELEMENT h);
// 
//         public:
//             /** 设置根目录
//             *	return:
//             *      TRUE    设置成功
//             *	params:
//             *		-[in]
//             *          pszRootPath  目录路径，必须是根路径，可以为classid
//             *                       为NULL时，根目录为桌面
//             *          nClsID       CSIDL_DESKTOP....CSIDL_CONTROLS..
//             **/
//             BOOL SetRoot(__in LPCWSTR pszRootPath);
//             BOOL SetRoot(__in int nClsID);
// 
//         };

        class ECombobox : public ECtrl
        {
        protected:
            EOptionsBox		op_box_;
            ECtrl			caption_;

        public:
            ECombobox(HELEMENT h=NULL);

            int GetItemCount()
            {
                return op_box_.GetItemCount();
            }

            // -1 last item, 0 first item
            BOOL SetCurSel( int idx );

            // -1 none selected
            int GetCurSelIndex()
            {
                return op_box_.GetCurSelIndex();
            }

            EOption GetCurSelItem()
            {
                return op_box_.GetCurSelItem();
            }

            // 元素类型是否是options
            BOOL IsItemGroup(int idx)
            {
                return op_box_.IsItemGroup(idx);
            }

            ECtrl GetItem(int idx)
            {
                return op_box_.GetItem(idx);
            }

            CStdString GetItemText(int idx)
            {
                return op_box_.GetItemText(idx);
            }

            CStdString GetCurItemText();

            void SetItemText(int idx,LPCWSTR sText)
            {
                op_box_.SetItemText(idx,sText);
            }

            CStdString GetItemAttribute(int idx,const char* atti)
            {
                return op_box_.GetItemAttribute(idx,atti);
            }

            void SetItemAttribute(int idx,LPCSTR satti,LPCWSTR sValue)
            {
                op_box_.SetItemAttribute(idx,satti,sValue);
            }

            CStdString GetItemStyleAttribute( int idx,LPCSTR satti )
            {
                return op_box_.GetItemStyleAttribute(idx,satti);
            }

            void SetItemStyleAttribute(int idx,LPCSTR satti,LPCWSTR sValue)
            {
                op_box_.SetItemStyleAttribute(idx,satti,sValue);
            }

            CStdString GetCurSelItemAttribute(const char* atti)
            {
                return op_box_.GetCurSelItemAttribute(atti);
            }

            EOption SelectItem_IDorName(LPCWSTR sIDorName )
            {
                return op_box_.SelectItem_IDorName(sIDorName);
            }

            EOption SelectItem_Attribute(char* satti,LPCWSTR sValue )
            {
                return op_box_.SelectItem_Attribute(satti,sValue);
            }

            EOption SelectItem_Text( LPCWSTR sValue )
            {
                return op_box_.SelectItem_Text(sValue);
            }

            EOption SelectItem_ItemData(LPVOID dwData)
            {
                return op_box_.SelectItem_ItemData(dwData);
            }

            // idx:-1 last
            // ret: 实际插入的idx
            EOption InsertItem( LPCWSTR sItem ,int idx = INT_MAX)
            {
                return op_box_.InsertItem(sItem, idx);
            }

            void DeleteAllItem()
            {
                return op_box_.DeleteAllItem();
                caption_.set_text(L"");
            }

            BOOL SetItemData( int idx, LPVOID dwData )
            {
                return op_box_.SetItemData(idx,dwData);
            }

            inline void SetItemData( EOption& op, LPVOID dwData )
            {
                op_box_.SetItemData(op, dwData);
            }

            LPVOID GetItemData(int idx)
            {
                return op_box_.GetItemData(idx);
            }
        };

        class EEditCmb : public ECombobox
        {
        public:
            EEditCmb(HELEMENT h=NULL);

            void SetValue(LPCWSTR sValue);

            inline CStdString GetCaptionText()
            {
                return caption_.GetText();
            }
        };

        class EDate : public ECtrl
        {
        public:
            EDate(HELEMENT h=NULL);

            void SetDate(const QTime& t);

            QTime GetDate();
        };

        class ETime : public ECtrl
        {
        public:
            ETime(HELEMENT h=NULL);

            void SetTime(const QTime& t);

            QTime GetTime();

        };

        class EQTimeCtrl : public ECtrl
        {
        private:
            ECtrl		m_eCaption;
            ECtrl		m_eRoot;
            ECtrl		m_ePopup;

        public:
            EQTimeCtrl(HELEMENT h=NULL);

            void SetTime(const QTime& t);

            QTime GetTime();

        };

        class EFilePath : public ECtrl
        {
        public:
            EFilePath(HELEMENT h=NULL):ECtrl(h)
            {
                //				CTL_TYPE c = get_ctl_type();
                //				ASSERT(get_ctl_type() == CTL_PASSWORD)
            }
        public:
            CStdString GetFileName();
            CStdString GetFilePath();
            void SetFilePath(LPCWSTR pszPath);
        };

        class EFolderPath : public ECtrl
        {
        public:
            EFolderPath(HELEMENT h=NULL):ECtrl(h)
            {

            }
            CStdString GetFolderName();
            CStdString GetFolderPath();
            void SetFolderPath(LPCWSTR pszPath);
        };

        //////////////////////////////////////////////////////////////////////////
        class EEdit : public ECtrl
        {
        public:
            EEdit(HELEMENT h=NULL);
            
        public:
            bool GetSelection( int& start, int& end );
            
            bool SelectText( int start = 0, int end = 0xFFFF );

            bool ReplaceSelection(const wchar_t* text, size_t text_length);

            CStdString GetText() const;

            void SetText(const wchar_t* text, size_t length);

            void SetText(const wchar_t* text);

            void SetInt( int v );

            int GetInt( ) const
            {
                return _wtoi( GetText() );
            }

            void SetDouble( double v );

            double GetDouble()
            {
                return _wtof(GetText());
            }

            int GetCharPos(int x, int y) const;
        };

        class ENumber : public EEdit
        {
        public:
            ENumber(HELEMENT h=NULL);
            
            void SetLimit(int nMin,int nMax);

            
            void SetStep(int nStep);

            int GetNum();
        };

        class EPassword : public EEdit
        {
        public:
            EPassword(HELEMENT h=NULL);
            
        };

        //////////////////////////////////////////////////////////////////////////
        class ECheck : public ECtrl
        {
        public:
            ECheck(HELEMENT h=NULL);
            
        };

        class ERadio : public ECtrl
        {
        public:
            ERadio(HELEMENT h=NULL);
            
        };

        class ERadioGroup : public ECtrl
        {
        public:
            ERadioGroup(HELEMENT h=NULL):ECtrl(h)
            {
            }

            // 获取当前被选中的按钮
            ECtrl GetCheck();

            // button 个数
            int GetButtonCount()
            {
                return children_count();
            }

            /** 选中第几个button
            *	return
            *      TRUE        成功
            *	param
            *		-[in]
            *          idx     第几个按钮，基数索引为0
            *                  -1  最后一个
            *          sel     选择子
            **/
            BOOL CheckButton(int idx = -1);
            BOOL CheckButton(const char *sel);
        };

        //////////////////////////////////////////////////////////////////////////
        // <div .pagenav>
        //		<table page><tr><td>1</td><td>2</td></tr></table>
        // </div>
        class EPageCtrl : public ECtrl
        {
        private:
            ECtrl m_root;
        public:
            EPageCtrl(HELEMENT h);

        public:
            void SetPageNum(int nPage);
            
            // 获取当前选中的index，
            // 返回值： == 0 无选中项
            //		> 0 选中的页索引
            int GetCurPage();
            
            int GetPageNum();
          
            void EPageCtrl::SetCurPage(int iPage);
        };
        //////////////////////////////////////////////////////////////////////////
        class EColorPicker : public ECtrl
        {
        private:
            ECtrl root_;
            ECtrl popup_;

        public :
            EColorPicker(HELEMENT h=NULL);
            
        public:
            CStdString GetColor();
            
            /**
             *	！获取到的颜色值是ARGB，和COLORREF的RGB顺序正好是相反的
             *
            **/
            DWORD GetColorARGB();
            
            DWORD GetCOLORREF();
            
            void SetCOLORREF(DWORD dwColor);
            
            /**
             * 格式：#xxxxxx
             *
            **/
            void SetCOLORREF(LPCWSTR sColor);

            void SetColorARGB( DWORD dwColor );
        };

        //////////////////////////////////////////////////////////////////////////
        // ESlider
        class ESlider : public ECtrl
        {
        public:
            ESlider(HELEMENT h):ECtrl(h)
            {

            }

        public:
            int GetValue()
            {
                return aux::wtoi(get_value().to_string());
            }
            
            int GetMin()
            {
                return aux::wtoi(get_attribute("min"));
            }
            
            int GetMax()
            {
                return aux::wtoi(get_attribute("max"));
            }
            
            void SetRange(int nMin,int nMax)
            {
                set_attribute("min",aux::itow(nMin));
                set_attribute("max",aux::itow(nMax));
            }

            void SetPos(int nPos)
            {
                set_value(json::value(nPos));
            }
        };

        //////////////////////////////////////////////////////////////////////////
        class EStarBox : public ECtrl
        {
        public:
            EStarBox(HELEMENT h=NULL);
            
        public:
            void SetCurSel( int nPri );
            
            // Base Index : 1
            int GetCurSel()
            {
                return aux::wtoi(get_attribute("index"));
            }
        };

        //////////////////////////////////////////////////////////////////////////
        class ETabCtrl : public ECtrl
        {
        public:
            ETabCtrl(HELEMENT h=NULL);

        public:
            static ETabCtrl create();

            int GetTabCount();

            ECtrl GetTab(int idx);

            // 使用 [panel="szPanel"] 在strip_下查找
            ECtrl GetTab(LPCWSTR szPanel);

            // 直接使用selector在strip_下查找
            ECtrl GetTabWithSelector( const char* szSelector );

            ECtrl GetTabPage(int idx);

            ECtrl GetTabPage(LPCWSTR szPanel);

            ECtrl GetTabPage( ECtrl& eTabItem );

            ECtrl GetPageTab(__in ECtrl &ePage);

            ECtrl InsertTab(LPCWSTR szPanel,LPCWSTR szTab,int idx = -1);

            /**
            *	已经存在page，但是不存在相关联的tab，现在为其创建一个tab
            *
            *  return
            *      新创建的tab，如果失败is_valid 为False
            *	param
            *		-[in]
            *          ePage       已经存在的页面
            *
            **/
            ECtrl CreateTabOfPage(__in ECtrl ePage, __in LPCWSTR szTabCaption, __in int idx = -1);

            ECtrl GetCurrentTab();

            ECtrl GetTabWithData(LPVOID pData);

            void RemoveTab(LPCWSTR szPanel);

            void RemoveTab(int idx);

            void RemoveTab( ECtrl tab );

            void RemoveAllTabs();

            void SelectTab(int idx);

            void SelectTab(LPCWSTR szPanel);

            void SelectTab(ECtrl ctlTabItem);

            void ShowCloseButton(BOOL bShow=TRUE,LPCWSTR szButtonName=NULL);

            void ShowItemCloseButton(int iItem,BOOL bShow=TRUE,LPCWSTR szButtonName=NULL);

            void ShowItemCloseButton( ECtrl&tabItem,BOOL bShow=TRUE,LPCWSTR szButtonName=NULL );

            BOOL IsShowCloseButton();

        protected:
            ECtrl	strip_;
        };

        //////////////////////////////////////////////////////////////////////////
        // tagbox
        class ETagItem : public ETable
        {
        public:
            ETagItem(HELEMENT h=NULL) : ETable(h) {}

        public:
            CStdString GetTag();

            void SetTag(LPCWSTR szText);
            /** 设置关闭按钮的名字
            *	-param
            *		-[in]   pszButtonName   关闭按钮名字
            *                              NULL，不显示关闭按钮
            **/
            void ShowCloseButton(LPCWSTR pszButtonName=NULL);

        protected:
            ECtrl TextCell()
            {
                return find_first("tr>td:nth-child(1)");
            }

            ECtrl CloseButtonCell()
            {
                return find_first("tr>td:nth-child(2)");
            }
        };

        class ETagBox : public ECtrl
        {
        public:
            ETagBox(HELEMENT h=NULL) : ECtrl (h)
            {
            }

        public:
            ETagItem AddTag(LPCWSTR szTagText,LPCWSTR szTagName);

            // 选中一个tag
            BOOL CheckTag(int idx);

            int GetTagCount()
            {
                return children_count();
            }

            ETagItem GetCheckedTag();

            int GetAllCheckedTag(__out std::vector<ETagItem>& vtags);

            ETagItem GetTag(int idx);

            ETagItem GetTag(LPCWSTR szTagID);

            void RemoveAllTag();

            /** 显示或不显示关闭按钮
            *	-param
            *		-[in]   pszCloseBtnName   关闭按钮名字
            *                                NULL，不显示关闭按钮
            **/
            void ShowCloseButton(LPCWSTR pszCloseBtnName=NULL);

        protected:
            inline LPCWSTR _CloseButtonName();
        };

        //////////////////////////////////////////////////////////////////////////
        // menu
        class EMenuItem : public ECtrl
        {
        public:
            EMenuItem(HELEMENT h=NULL):ECtrl(h)
            {

            }

            void Enable(BOOL bEnable = TRUE );
        };

        class EMenu : public ECtrl
        {
        public:
            EMenu(HELEMENT h=NULL):ECtrl(h)
            {

            }

            /** 禁用、启用某个菜单项
            *	return:
            *      被操作的菜单项
            *	params:
            *		-[in]
            *          szSelector      find_first(szSelector);
            *          bEnable         FALSE 禁用
            **/
            EMenuItem EnableMenuItem(LPCWSTR szSelector,BOOL bEnable=TRUE);

            void EnableAllMenuItem(BOOL bEnable = TRUE);
        };

        //////////////////////////////////////////////////////////////////////////
        class EPopup : public ECtrl
        {
        public:
            EPopup(HELEMENT h = NULL):ECtrl(h)
            {

            }
        };

        //////////////////////////////////////////////////////////////////////////
        class EButton : public ECtrl
        {
        public:
            EButton(HELEMENT h):ECtrl(h)
            {

            }
        };

        //////////////////////////////////////////////////////////////////////////
        class EHyperLink : public ECtrl
        {
        public:
            EHyperLink(HELEMENT h):ECtrl(h)
            {

            }
        };

        //////////////////////////////////////////////////////////////////////////
#ifdef UIBASE_SUPPORT_FLASH
        class EFlash : public ECtrl
        {
            friend class QView;
        public:
            EFlash(HELEMENT h) : ECtrl(h)
            {
                ATLASSERT( aux::wcseqi(L"flash",get_attribute("type")) );
            }

        public:
            // 加载新的swf文件，必须是本地全路径
            void SetSwf(LPCWSTR pszFilePath);

            // 获取当前swf文件地址
            LPCWSTR GetSwfPath();

            // 获取关联的CFlashWnd对象指针
            CFlashWnd *GetFlashObject();

            // 设置播放器的背景，非DOM元素背景
            void SetSwfBkgndColor(DWORD dwRGB);
        };
#endif
        //////////////////////////////////////////////////////////////////////////
#ifdef UIBASE_SUPPORT_WEBBROWSER
        class EWebBrowser : public ECtrl
        {
            friend class QView;
        public:
            EWebBrowser(HELEMENT h) : ECtrl(h)
            {
                ATLASSERT( aux::wcseqi(L"WebBrowser",get_attribute("type")) );
            }

        public:
            // 导航到URL
            void GotoURL(LPCWSTR psURL);

            // 获取当前URL
            LPCWSTR GetURL();

            // 获取关联的CBrowserHostPtr对象指针
            CWebBrowser *GetBrowser();

            void SetData( LPVOID pData );
        };
#endif

        //////////////////////////////////////////////////////////////////////////
        // LED
        class ELed : public ECtrl
        {
        public:
            ELed(HELEMENT h) : ECtrl(h)
            {

            }

        public:
            /** 设置灯的状态
            *	-param
            *		-[in]
            *          idx     索引值[0, count)
            *          bOn     设置点亮状态，checked
            **/
            void SetOnOff(int idx,BOOL bOn=TRUE);

            /**注册状态，并且设置此状态对应的img
            *  （当设定此状态的时候，会自动设定注册的img）
            *	return:
            *      TRUE        成功
            *	params:
            *		-[in]
            *          szStatus    状态名，非NULL
            *          szImg       状态对应的图像表示
            **/
            BOOL RegisterStatus(__in LPCSTR szStatus,__in LPCWSTR szImg);

            /** 设定其已注册的状态
            *	return:
            *      FALSE        失败，也许是状态未注册
            *	params:
            *		-[in]
            *          idx             led索引
            *          szStatus        已注册状态
            *
            **/
            BOOL SetStatus( __in int idx,__in LPCSTR szStatus );

            /** 获取索引为idx的灯控件
            *	-return
            *      ECtrl，需要is_valid判断其是否有效
            *	-param
            *		-[in]
            *          idx     [0,count)
            **/
            ECtrl GetLed(int idx);

            /** 获取LED个数
            *	-return
            *      int     获取LED个数 [0,n]
            **/
            int GetCount();

            /** 添加nCount个LED灯
            *	-param
            *		-[in]
            *          nCount  需要添加的个数 [1,n)
            *          bOn     初始化状态
            **/
            void AddLed( int nCount, BOOL bOn=TRUE );

            /** 移除最后的nCount个LED
            *	-param
            *		-[in]
            *          nCount  需要移除的个数
            **/
            void RemoveLast(int nCount);

            /** 移除最前面的nCount个LED
            *	-param
            *		-[in]
            *          nCount  需要移除的个数
            **/
            void RemoveFirst(int nCount);

            /** 移除所有LED
            *		-[]
            **/
            void RemoveAll();
        };

        //////////////////////////////////////////////////////////////////////////
        // treelist
        // treelist 控件的管理数据
        typedef ETableRow ETreeListItem;
        typedef std::vector<HELEMENT> ArrayHE;
        typedef tree<HELEMENT> TreeHE;
        class _qTLStructMan;
        class _qTLStruct        //
        {
            typedef TreeHE::iterator_base base_itr;
            typedef TreeHE::pre_order_iterator pre_itr;
            typedef TreeHE::children_iterator children_itr;

            friend class _qTLStructMan;
        protected:
            // 仅能通过_qTLStructMan注册创建
            _qTLStruct(HELEMENT heRoot);

            ~_qTLStruct()
            {

            }

        public:

#ifdef _DEBUG
            void _DebugoutputTreeStruct();
#endif
            /** 获取heParent下的直接孩子节点
            *	return:
            *      孩子个数，-1 错误
            *	params:
            *		-[in]
            *          heParent        父亲节点
            *                          如果为NULL，则获取第一级节点
            *		-[out]
            *          arC             孩子节点（按先后顺序）
            **/
            int GetChildren(__in HELEMENT heParent, __out ArrayHE &arC);

            /*
            *	选择hParent下的子节点
            *      *_Callback 返回true停止遍历
            */
            template<class _Callback>
            void SelectChildItem(__in HELEMENT heParent, _Callback&& cb)
            {
                pre_itr iPos;
                if ((NULL == heParent) || (heParent == m_heTL))
                    iPos = m_t.root();
                else if ( !_FindItem(heParent, iPos) )
                    return;

                children_itr iEnd = m_t.end_child(iPos);
                for (children_itr i = m_t.begin_child(iPos); i != iEnd; ++i)
                {
                    if (cb(*i))
                    {
                        break;
                    }
                }
            }

            /** 获取以heItem为根节点的节点树
            *	return
            *      TRUE        成功
            *	param
            *		-[in]
            *          heItem      节点
            *		-[out]
            *			t           节点树
            **/
            BOOL GetNodeTree(__in HELEMENT heItem, __out TreeHE& t);

            /** 获取heParent的第iPos个孩子
            *	return:
            *      孩子节点，NULL，没有找到这个孩子
            *	params:
            *		-[in]
            *          heParent        父亲节点
            *                          如果为NULL，则获取第一级节点
            *          iPos            孩子位置索引，基索引为0
            *                          -1      最后一个孩子
            *
            **/
            HELEMENT GetNthChild(__in HELEMENT heParent, __in int idx = -1 );

            /** 获取下一个兄弟节点
            *	return
            *      下一个兄弟节点
            *	param
            *		-[in]
            *          heItem      节点
            *
            **/
            HELEMENT GetNextSiblingItem(__in HELEMENT heItem);

            /** 获取上一个兄弟节点
            *	return
            *      上一个兄弟节点
            *	param
            *		-[in]
            *          heItem      节点
            *
            **/
            HELEMENT GetPrevSiblingItem(__in HELEMENT heItem);

            /** 获取孩子节点的个数（只包括直接孩子）
            *	return
            *      -1  错误
            *	param
            *		-[in]
            *          heItem      节点
            *                      如果为NULL， 则认为是获取第一级节点的个数
            *
            **/
            int GetChildrenCount(__in HELEMENT heItem = NULL);

            /** 获取父亲节点
            *	return
            *      NULL 没父亲
            *	param
            *		-[in]
            *          heItem      谁的父亲？
            *
            **/
            HELEMENT GetParentItem(__in HELEMENT heItem);

            /** 插入孩子节点
            *	return
            *      是否成功插入孩子节点？
            *	param
            *		-[in]
            *          heParent        父亲节点
            *                          如果为NULL，插入为顶级节点（树的第一级节点）
            *          heItem          孩子节点
            *          iPos            父亲的第几个孩子，基数为0
            *                          真正应该被插入到的位置
            **/
            BOOL InsertChild(__in HELEMENT heParent, __in HELEMENT heItem, __inout int& idx);

            /** 获取子节点个数
            *	return
            *      子节点个数
            *	param
            *		-[in]
            *          heItem      父亲节点，
            *                      如果为NULL，获取第一级子节点
            *
            **/
            inline int GetChildItemCount(__in HELEMENT heItem = NULL);

            /** 删除节点
            *	return
            *      TRUE        成功
            *	param
            *		-[in]
            *          heItem      目标节点
            *
            **/
            BOOL DeleteItem(__in HELEMENT heItem);

            void DeleteAllItem()
            {
                m_t.erase_children(m_t.root());
            }

            void ExpandItem(__in HELEMENT heItem);

            /** 展开 heItem 的整个分支（从根到叶子）
            *	param
            *		-[in]
            *          heItem      分支中的一个节点
            **/
            void ExpandLeaf(__in HELEMENT heItem);

            // 展开所有的分支
            void ExpandAllLeaf();

            void CollapseItem(__in HELEMENT heItem);

            /** 收起所有第【nLevel】层的节点
            *	param
            *		-[in]
            *          nLevel      第几层的节点， 取值范围[1, n)
            **/
            void CollapseAll(__in int nLevel);

            inline BOOL IsItemExpand(ETreeListItem& cRow)
            {
                return cRow.get_state(STATE_EXPANDED) // cRow.IsHasAttribute("tl_expand")
                    && !cRow.IsHasAttribute("tl_hide");
            }

            inline BOOL IsItemHidingExpand(ETreeListItem& cRow)
            {
                return cRow.get_state(STATE_EXPANDED) //cRow.IsHasAttribute("tl_expand")
                    && cRow.IsHasAttribute("tl_hide");
            }

            /** 判断heAncestor是否是heItem的祖先（父亲/祖父/。。。）
            *      根节点（treeelist 本身）虽然在所有节点的顶端
            *      但是它不是任何节点的祖先。
            *	return
            *      TRUE    是祖先
            *	param
            *		-[in]
            *          heItem
            *          heAncestor      祖先节点
            *
            **/
            BOOL IsAncestor(__in HELEMENT heItem, __in HELEMENT heAncestor);

            /** 将节点（包含子树）剪切到目标节点下
            *	return
            *      TRUE        成功
            *                  源节点不可以是目标节点的祖先
            *	param
            *		-[in]
            *          heItem      源节点
            *          heDest      目标节点
            **/
            BOOL CutTo(__in HELEMENT heItem, __in HELEMENT heDest);

        protected:
            template<typename T_itr>
            BOOL _FindItem(HELEMENT heItem, __out T_itr &iPos)
            {
                ATLASSERT(NULL != heItem);
                ATLASSERT(m_heTL != heItem);

                TreeHE::fixed_depth_iterator i = m_t.begin_fixed(m_t.root(),
                    ECtrl(heItem).get_attribute_int("tl_level") + 1);   // 第0层是treelist，不是节点
                for ( ; m_t.is_valid(i); ++i)
                {
                    if (*i == heItem)
                    {
                        iPos = i;
                        return TRUE;
                    }
                }
                return FALSE;
            }

            inline BOOL IsValidTreeListItem(__in HELEMENT heItem)
            {
                ETreeListItem cTmp(heItem);
                return cTmp.is_valid() && (m_heTL == cTmp.parent());
            }

        private:
            TreeHE       m_t;
            HELEMENT    m_heTL; // treelist 的DOM
        };

        // 整个进程（其实仅限于UI线程）的TreeList的数据都会注册到这个
        // 管理类下面
        class _qTLStructMan
        {
            typedef std::map<HELEMENT, _qTLStruct*> MapTL;
            typedef MapTL::iterator MapTLItr;

            SINGLETON_ON_DESTRUCTOR(_qTLStructMan)
            {
                MapTLItr iEnd = m_TLs.end();
                for (MapTLItr i = m_TLs.begin(); i != iEnd; ++i)
                {
                    delete i->second;
                }
                m_TLs.clear();
            }

            friend struct treelist;
        public:
            static _qTLStruct* GetTLStruct(HELEMENT heRoot);

        protected:
            _qTLStruct* RegisterStruct(HELEMENT heRoot);

            void UnregisterStruct(HELEMENT heRoot);

            inline MapTLItr _Find(HELEMENT heRoot)
            {
                return m_TLs.find(heRoot);
            }

            inline MapTLItr _End()
            {
                return m_TLs.end();
            }

        private:
            MapTL       m_TLs;
        };

        class ETreeList : protected ETable
        {
            typedef ETable _Base;

        private:
            _qTLStruct *    m_pTL;

        public:
            ETreeList(HELEMENT h=NULL)
                :ETable(h)
            {
                m_pTL = _qTLStructMan::GetTLStruct(h);
            }

            ETreeList& operator=(const ETreeList&o);

            operator HELEMENT()const
            {
                return he;
            }

        public:
            inline BOOL IsValid()
            {
                return _Base::is_valid();
            }

#ifdef _DEBUG
            void _DebugoutputTreeStruct()
            {
                m_pTL->_DebugoutputTreeStruct();
            }
#endif
            /** 插入一个孩子节点
            *	return:
            *      孩子item
            *	params:
            *		-[in]
            *          cParent     插入在哪个节点下
            *          szCells     节点内容
            *          idx         插在节点下的第几个位置
            *
            **/
            ETreeListItem InsertChild(__in HELEMENT heParent,
                __in LPCWSTR szCells, __in int idx = -1);

            ETreeListItem AppendChild(__in HELEMENT heParent,
                __in LPCWSTR szCells)
            {
                ATLASSERT(NULL != szCells);
                return InsertChild(heParent, szCells, -1);
            }

            ETreeListItem InsertChild(__in HELEMENT heParent,
                __in HELEMENT heItem, __in int idx = -1);

            ETreeListItem AppendChild(__in HELEMENT heParent,
                __in HELEMENT heItem)
            {
                ATLASSERT(NULL != heItem);
                return InsertChild(heParent, heItem, -1);
            }

            /** 插入顶层item
            *	return:
            *      插入的行
            *	params:
            *		-[in]
            *          szCells 行的内容 格式： <td>1</td><td>2</td>
            *          idx     插入到第几行？
            *                  = -1， 最后一行
            **/
            ETreeListItem InsertItem(__in LPCWSTR szCells, __in int idx = -1)
            {
                return InsertChild(NULL, szCells, idx);
            }

            ETreeListItem GetNextSiblingItem(__in ETreeListItem& cItem)
            {
                return m_pTL->GetNextSiblingItem(cItem);
            }

            ETreeListItem GetPrevSiblingItem(__in ETreeListItem& cItem)
            {
                return m_pTL->GetPrevSiblingItem(cItem);
            }

            /** 下一级的第几个孩子
            *	return:
            *      孩子item
            *	params:
            *		-[in]
            *          cParent     哪个节点下的孩子
            *          idx         第几个孩子，[-1, size), = -1, 最后一个孩子
            **/
            ETreeListItem GetNthChild( __in ETreeListItem& cParent, __in int idx = -1 )
            {
                return m_pTL->GetNthChild(cParent, idx);
            }

            // 展开、收缩顶级项目
            inline BOOL IsItemExpand(ETreeListItem& cItem)
            {
                return m_pTL->IsItemExpand(cItem);
            }

            inline BOOL IsItemHidingExpand(ETreeListItem& cItem)
            {
                return m_pTL->IsItemHidingExpand(cItem);
            }

            void ExpandItem(ETreeListItem& cItem)
            {
                m_pTL->ExpandItem(cItem);
            }

            // 展开所有的分支
            void ExpandAll()
            {
                m_pTL->ExpandAllLeaf();
            }

            /** 展开 heItem 的整个分支（从根到叶子）
            *	param
            *		-[in]
            *          heItem      分支中的一个节点
            **/
            void ExpandLeaf(__in HELEMENT heItem)
            {
                m_pTL->ExpandLeaf(heItem);
            }

            void CollapseItem( ETreeListItem& cItem )
            {
                m_pTL->CollapseItem(cItem);
            }

            /** 收起所有第【nLevel】层的节点
            *	param
            *		-[in]
            *          nLevel      第几层的节点， 取值范围[1, n)
            **/
            void CollapseAll(__in int nLevel = 1)
            {
                m_pTL->CollapseAll(nLevel);
            }

            int GetItemCount()const
            {
                return m_pTL->GetChildItemCount(NULL);
            }

            int GetChildItemCount(ETreeListItem &cParent)
            {
                return m_pTL->GetChildItemCount(cParent);
            }

            int GetChildItem(__in ETreeListItem& cParent, __out ArrayHE& vI)
            {
                return m_pTL->GetChildren(cParent, vI);
            }

            /*
            *	选择hParent下的子节点
            *      *_Callback 返回true停止遍历
            */
            template<class _Callback>
            void SelectChildItem(__in ETreeListItem& cParent, _Callback cb)
            {
                return m_pTL->SelectChildItem(cParent, cb);
            }

            // 顶层item
            int GetItems(__out ArrayHE& vI)
            {
                return m_pTL->GetChildren(NULL, vI);
            }

            // == -1, 最后一个孩子
            ETreeListItem GetItem(__in int idx = -1)
            {
                return m_pTL->GetNthChild(NULL, idx);
            }

            // 获取父亲节点
            ETreeListItem GetParentItem(ETreeListItem& cItem)
            {
                return m_pTL->GetParentItem(cItem);
            }

            // 当前选中的节点
            ETreeListItem GetCurrentItem();

            // 设置当前选中的项目
            //  返回之前选中的项目
            ETreeListItem SetCurrentItem(ETreeListItem &cItem);

            // 具有某关联数据的节点
            ETreeListItem GetItemWithData(LPVOID pData)
            {
                return _Base::FindFirstWithData(pData);
            }

            // 设置item
            ETreeListItem SetItemText(ETreeListItem&cItem, LPCWSTR szText);

            void DeleteAllItem()
            {
                m_pTL->DeleteAllItem();
                _Base::DeleteAllRow();
            }

            void DeleteItem(ETreeListItem &cItem);

            /** 判断heAncestor是否是heItem的祖先（父亲/祖父/。。。）
            *      根节点（treeelist 本身）虽然在所有节点的顶端
            *      但是它不是任何节点的祖先。
            *	return
            *      TRUE    是祖先
            *	param
            *		-[in]
            *          heItem
            *          heAncestor      祖先节点
            *
            **/
            BOOL IsAncestor(__in HELEMENT heItem, __in HELEMENT heAncestor)
            {
                return m_pTL->IsAncestor(heItem, heAncestor);
            }

            /** 将节点（包含子树）剪切到目标节点下
            *	return
            *      TRUE        成功
            *                  源节点不可以是目标节点的祖先
            *	param
            *		-[in]
            *          heItem      源节点
            *          heDest      目标节点
            **/
            BOOL CopyTo(__in HELEMENT heItem, __in HELEMENT heDest);
            BOOL CutTo(__in HELEMENT heItem, __in HELEMENT heDest);

            // 获取第几层
            inline int GetItemLevel(__in ETreeListItem& cItem)
            {
                return cItem.get_attribute_int("tl_level");
            }

        protected:
            inline void _SetLevel(__in ETreeListItem& cParent,
                __in ETreeListItem& cItem);

            // 顶级level
            inline void _SetLevel(__in ETreeListItem& cItem)
            {
                cItem.set_attribute("tl_level", L"0");
            }

            inline int _HE2I(HELEMENT h)
            {
                return (int)h;
            }

            inline int _LevelMargin()
            {
                int n = get_attribute_int("level_margin");
                return n > 10 ? n : 10;  // 最小10px
            }
        };

        //////////////////////////////////////////////////////////////////////////
        class EOutlook : public ECtrl
        {
        public:
            EOutlook(HELEMENT he = NULL):ECtrl(he)
            {

            }

        public:
            /** 展开、收缩一个item
            *	return:
            *      szItemSelector 指向的item
            *	params:
            *		-[in]
            *          szItemSelector      find_first("li<szItemSelector>")
            *          bExpand             TRUE 展开
            **/
            ECtrl ExpandItem(LPCWSTR szItemSelector,BOOL bExpand=TRUE);

            /** 是否支持多个item同时展开
            *      使用 multi-expand=1 启用多项目展开模式
            *	return:
            *      TRUE        是
            **/
            BOOL IsMultiExand()
            {
                return IsHasAttribute("multi-expand");
            }

            // 启用多item同时展开模式
            void EnableMulitExpand(BOOL bEnable = TRUE);

            // 显示item
            void ShowItem(LPCWSTR szSelector,BOOL bShow = TRUE);

            inline ECtrl GetItem(LPCWSTR szSelector)
            {
                return find_first("li%S",szSelector);
            }

            inline ECtrl GetItemPanel(__in ECtrl& itm)
            {
                return itm.find_first(":root>.content");
            }

            ECtrl GetItemPanel(LPCWSTR szSelector);
            
            inline ECtrl GetCurrentExpandItem()
            {
                return find_first("li:expanded");
            }

            // 插入一个项目
            ECtrl InsertItem(__in LPCWSTR szItem, __in LPCWSTR szContTag = L"div",
                __in int idx = INT_MAX);

        };

        //////////////////////////////////////////////////////////////////////////
        // 表单 sheet
        class ESheet : public ETable
        {
        public:
            ESheet(HELEMENT h = NULL) : ETable(h) {}

        public:
            // 显示或隐藏cCell下的widget
            BOOL ShowCellWidget( ETableCell cCell );

            BOOL ShowCellWidget( ETableRow cRow, int iCol)
            {
                return ShowCellWidget(cRow.GetCell(iCol));
            }

            BOOL ShowCellWidget(int iRow, int iCol)
            {
                return ShowCellWidget(GetCell(iRow, iCol));
            }

            // 关闭当前显示的cell-widget
            // bUpdateContent       TRUE    更新cell-widget数据到cell上
            void CloseCellWidget( BOOL bUpdateContent = TRUE );

            // 是否正在显示着cell-widget
            inline BOOL IsCellWidgetShowing(ETableCell cell)
            {
                return GetCurEditCell() == cell;
            }

            // 当前正在显示着cell-widget的cell
            ETableCell GetCurEditCell();
        };

        //////////////////////////////////////////////////////////////////////////
        // 倒计时的behavior
        // <div style="behavior:clock-countdown;">00:30:29</div>
        class ETextCountdown : public ECtrl
        {
        public:
            ETextCountdown(HELEMENT h =NULL ) :ECtrl(h)
            {

            }

        public:
            // 设置倒计时
            // szTime 格式： HH:MM:SS
            void Settime(LPCWSTR szTime)
            {
                json::string ss(szTime);
                xcall("settime", ss);
            }

            // 在现在的基础上增加nSec秒
            void Increase(int nSec)
            {
                json::value vl(nSec);
                xcall("increase", vl);
            }

            // 在现在的基础上减少nSec秒
            // 如果减少为0，则停止倒计时
            void Decrease(int nSec)
            {
                json::value vl(nSec);
                xcall("decrease", vl);
            }

            void Stop()
            {
                xcall("stop");
            }

            void Start()
            {
                xcall("start");
            }

            int GetCountdown()
            {
                return get_attribute_int("countdown");
            }
        };

        class ETextImage : public ECtrl
        {
        public:
            ETextImage(HELEMENT h): ECtrl(h)
            {

            }

        public:
            void SetImage(__in LPCWSTR szImg);

            void SetText(LPCWSTR szText);
        };

        //////////////////////////////////////////////////////////////////////////
        class EWordInput : public ECtrl
        {
        protected:
            EPopup  popup_;
            EOptionsBox wordlst_;
            EEdit   inpt_;
            ECtrl   cap_;

        public:
            EWordInput(HELEMENT he = NULL);
            
        public:
            // 添加到Caption上
            ECtrl AddCaptionItem(__in LPCWSTR szItem);

            // 添加一个项目
            EOption AddWordItem(__in LPCWSTR szItem);

            // caption中 是否已经有了一个
            inline BOOL IsCaptionHasWord(__in LPCWSTR szWord)
            {
                return FindWord(szWord).is_valid();
            }

            // 在caption中查找
            ECtrl FindWord(__in LPCTSTR szWord);

            // 删除caption中的选中项目
            void DeleteCheckedCaptionItem();

            // 过滤select
            void FilterSelectByInput();

            // items
            CStdString GetCaptionItems(__in wchar_t chSep);

        protected:
            inline BOOL IsWordCaseSentive()
            {
                return IsHasAttribute("word-case-senstive");
            }
        };
    };
};

#endif // ECtrl_h__
