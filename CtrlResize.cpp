// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        visualcpp.net
 
		revision history:
		rev		date	comments
        00      07may03	replaced leading underscores with m_ convention
		01		02oct04	add optional origin shift to initial parent rect
		02		30jul05	make style compatible
		03		19feb06	in AddControlList, fix loop test
 
		resize a windows control

*/

#include "stdafx.h"
#include "CtrlResize.h"

CCtrlResize::CtrlInfo::CtrlInfo()
{
	m_CtrlID = 0;
	m_BindType = BIND_UNKNOWN;
	m_RectInit.SetRectEmpty();
	m_pCtrlWnd = NULL;
}

CCtrlResize::CtrlInfo::CtrlInfo(int CtrlID, int BindType, const CRect *RectInit, CWnd* pCtrlWnd)
{
	m_CtrlID = CtrlID;
	m_BindType = BindType;
	m_RectInit = RectInit;
	m_pCtrlWnd = pCtrlWnd;
};

void CCtrlResize::AddControl(int CtrlID, int BindType, const CRect *RectInit)
{
	CtrlInfo	info(CtrlID, BindType, RectInit);
	m_Info.Add(info);
}

void CCtrlResize::AddControlList(CWnd *pWnd, const CTRL_LIST *List)
{
	SetParentWnd(pWnd);
	for (int i = 0; List[i].CtrlID; i++)
		AddControl(List[i].CtrlID, List[i].BindType);
	FixControls();
}

bool CCtrlResize::FixControls()
{
	if (!m_pWnd)
		return FALSE;
	m_pWnd->GetClientRect(&m_RectInit);
	for (int i = 0; i < m_Info.GetSize(); i++) {
		m_Info[i].m_pCtrlWnd = m_pWnd->GetDlgItem(m_Info[i].m_CtrlID);
		if (m_Info[i].m_pCtrlWnd) {
			m_Info[i].m_pCtrlWnd->GetWindowRect(&m_Info[i].m_RectInit);
			m_pWnd->ScreenToClient(&m_Info[i].m_RectInit);
		}
	}
	return TRUE;
}

CCtrlResize::CCtrlResize() 
{
	m_pWnd = NULL;
	m_OrgShift = CSize(0, 0);
}

void CCtrlResize::SetParentWnd(CWnd *pWnd)
{
	m_pWnd = pWnd;
}

void CCtrlResize::OnSize()
{
	if (!m_pWnd || !m_pWnd->IsWindowVisible())
		return;
	CRect rr, rectWnd;
	m_pWnd->GetClientRect(&rectWnd);
	for (int i = 0; i < m_Info.GetSize(); i++) {
		rr = m_Info[i].m_RectInit;
		rr.OffsetRect(m_OrgShift);
		if (m_Info[i].m_BindType & BIND_RIGHT) 
			rr.right = rectWnd.right -(m_RectInit.Width() - m_Info[i].m_RectInit.right);
		if (m_Info[i].m_BindType & BIND_BOTTOM) 
			rr.bottom = rectWnd.bottom -(m_RectInit.Height() - m_Info[i].m_RectInit.bottom);
		if (m_Info[i].m_BindType & BIND_TOP)
			;
		else
			rr.top = rr.bottom - m_Info[i].m_RectInit.Height();
		if (m_Info[i].m_BindType & BIND_LEFT)
			;
		else
			rr.left = rr.right - m_Info[i].m_RectInit.Width();
		m_Info[i].m_pCtrlWnd->MoveWindow(&rr);
		m_Info[i].m_pCtrlWnd->Invalidate(FALSE);
	}
}

void CCtrlResize::SetOriginShift(SIZE Shift)
{
	m_OrgShift = Shift;
}
