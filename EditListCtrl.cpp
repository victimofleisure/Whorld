// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23feb06	initial version
		01		28jan08	support Unicode

        list control with clipboard support
 
*/

// EditListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "EditListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditListCtrl window

IMPLEMENT_DYNAMIC(CEditListCtrl, CDragListCtrl);

CEditListCtrl::CEditListCtrl() :
	m_Clipboard(NULL, NULL)
{
	m_InfoSize = 0;
}

CEditListCtrl::~CEditListCtrl()
{
}

void CEditListCtrl::SetFormat(LPCTSTR Format, int InfoSize)
{
	m_Clipboard.SetFormat(Format);
	m_InfoSize = InfoSize;
}

void CEditListCtrl::SelectAll()
{
	int	Items = GetItemCount();
	for (int i = 0; i < Items; i++)
		SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
}

void CEditListCtrl::Deselect()
{
	int	Items = GetItemCount();
	for (int i = 0; i < Items; i++)
		SetItemState(i, 0, LVIS_SELECTED);
}

void CEditListCtrl::SetCurSel(int Idx)
{
	Deselect();
	SetSelectionMark(Idx);
	SetItemState(Idx, LVIS_SELECTED, LVIS_SELECTED);
}

void CEditListCtrl::Cut()
{
	Copy();
	Delete();
}

void CEditListCtrl::Copy()
{
	int	sels = GetSelectedCount();
	if (sels <= 0)
		return;
	DWORD	len = m_InfoSize * sels;
	char	*cbd = new char[len];
	POSITION	pos = GetFirstSelectedItemPosition();
	char	*p = cbd;
	while (pos != NULL) {
		int	sel = GetNextSelectedItem(pos);
		GetInfo(sel, p);
		p += m_InfoSize;
	}
	m_Clipboard.Write(cbd, len);
	delete [] cbd;
}

void CEditListCtrl::Paste()
{
	if (!CanPaste())
		return;
	DWORD	len;
	char	*cbd = (char *)m_Clipboard.Read(len);
	int	cnt = len / m_InfoSize;
	int	pos = GetCurSel();
	if (pos == LB_ERR)	// if no selection
		pos = GetItemCount();	// append
	char	*p = cbd;
	for (int i = 0; i < cnt; i++) {
		InsertInfo(pos + i, p);
		p += m_InfoSize;
	}
	delete [] cbd;
}

void CEditListCtrl::Delete()
{
	int	sels = GetSelectedCount();
	if (sels <= 0)
		return;
	int	*sel = new int[sels];
	POSITION	pos = GetFirstSelectedItemPosition();
	int	i = 0;
	while (pos != NULL)
		sel[i++] = GetNextSelectedItem(pos);
	for (i = sels - 1; i >= 0; i--)
		DeleteInfo(sel[i]);
	delete [] sel;
	SetItemState(GetSelectionMark(), LVIS_SELECTED, LVIS_SELECTED);
}

BEGIN_MESSAGE_MAP(CEditListCtrl, CDragListCtrl)
	//{{AFX_MSG_MAP(CEditListCtrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditListCtrl message handlers

