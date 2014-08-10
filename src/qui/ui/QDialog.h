#ifndef QDialog_h__
#define QDialog_h__

#pragma once

#include "QFrame.h"
#include "QMessageLoop.h"

/*
 *	对话框基础类
 */
class QDialog : public QFrame
{
	BEGIN_MSG_MAP_EX(QDialog)
		MSG_WM_DESTROY(OnDestroy)
		CHAIN_MSG_MAP(QFrame)
	END_MSG_MAP()
public:
	QDialog(LPCWSTR szResName=NULL) : QFrame(szResName) {}

    virtual ~QDialog() throw()
    {

    }

	/** 创建一个模态对话框
	 *	-return:		标准的模态对话框返回值
	 *	-params:	
	 *		-[in]	nDecoratedStyle		自定义的窗口美化风格
	 *									查看风格标志定义时的说明
     *              hWndParent          如果为NULL，则自动检测当前活动窗口作为父亲窗口
     *              rcWnd               定制窗口位置，如果为NULL，使用默认大小和位置
	 **/
	INT_PTR DoModal(HWND hWndParent = NULL, UINT nStyleEx=0,
        UINT nDecoratedStyle = WS_QEX_WNDSHADOW|WS_QEX_ROUNDCONNER,
        LPCRECT rcWnd = NULL);
		
	void EndDialog(INT_PTR nResult);

protected:
	void OnDestroy();

private:
    QMessageLoop    msg_loop_;
};

#endif // QDialog_h__
