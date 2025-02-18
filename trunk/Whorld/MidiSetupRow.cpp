// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      13aug05	initial version
		01		04oct05	add event type
		02		23jan08	replace MIDI range scaler with start/end
		03		28jan08	support Unicode

		MIDI setup dialog row
 
*/

// MidiSetupRow.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MidiSetupRow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupRow dialog

IMPLEMENT_DYNAMIC(CMidiSetupRow, CRowDialogRow);

const COLORREF CMidiSetupRow::m_SelColor = RGB(0, 255, 0);
const CBrush CMidiSetupRow::m_SelBrush(m_SelColor);

CMidiSetupRow::CMidiSetupRow(CWnd* pParent /*=NULL*/)
	: CRowDialogRow(CMidiSetupRow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMidiSetupRow)
	//}}AFX_DATA_INIT
	m_Selected = FALSE;
}

void CMidiSetupRow::SetCaption(LPCTSTR Title)
{
	m_Title.SetWindowText(Title);
}

void CMidiSetupRow::GetInfo(INFO& Info) const
{
	GetRange(Info.Range);
	Info.Event = m_Event.GetCurSel();
	Info.Chan = m_Chan.GetIntVal() - 1;
	Info.Ctrl = m_Ctrl.GetIntVal();
}

void CMidiSetupRow::SetInfo(const INFO& Info)
{
	m_RangeStart.SetVal(Info.Range.Start);
	m_RangeEnd.SetVal(Info.Range.End);
	Assign(Info.Event, Info.Chan, Info.Ctrl);
}

void CMidiSetupRow::SetValue(int Val)
{
	m_Value.SetVal(Val);
}

void CMidiSetupRow::Assign(int Event, int Chan, int Ctrl)
{
	m_Event.SetCurSel(Event);
	m_Chan.SetVal(Chan + 1);
	m_Ctrl.SetNoteEntry(Event == ET_NOTE);
	m_Ctrl.SetVal(Ctrl);
}

void CMidiSetupRow::SetSelected(bool Enable)
{
	m_Selected = Enable;
	Invalidate();
	m_Title.Invalidate();	// necessary because we're clipping children
}

void CMidiSetupRow::DoDataExchange(CDataExchange* pDX)
{
	CRowDialogRow::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMidiSetupRow)
	DDX_Control(pDX, IDC_MS_RANGE_START_SPIN, m_RangeStartSpin);
	DDX_Control(pDX, IDC_MS_RANGE_START, m_RangeStart);
	DDX_Control(pDX, IDC_MS_RANGE_END_SPIN, m_RangeEndSpin);
	DDX_Control(pDX, IDC_MS_RANGE_END, m_RangeEnd);
	DDX_Control(pDX, IDC_MS_CTRL_SPIN, m_CtrlSpin);
	DDX_Control(pDX, IDC_MS_CHAN_SPIN, m_ChanSpin);
	DDX_Control(pDX, IDC_MS_EVENT, m_Event);
	DDX_Control(pDX, IDC_MS_VALUE, m_Value);
	DDX_Control(pDX, IDC_MS_CHAN, m_Chan);
	DDX_Control(pDX, IDC_MS_CTRL, m_Ctrl);
	DDX_Control(pDX, IDC_MS_TITLE, m_Title);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMidiSetupRow, CRowDialogRow)
	//{{AFX_MSG_MAP(CMidiSetupRow)
	ON_CBN_SELCHANGE(IDC_MS_EVENT, OnSelchangeEvent)
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_PARENTNOTIFY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupRow message handlers

BOOL CMidiSetupRow::OnInitDialog() 
{
	CRowDialogRow::OnInitDialog();

	m_RangeStartSpin.SetDelta(.01);
	m_RangeEndSpin.SetDelta(.01);
	m_Event.SetCurSel(0);
	m_Chan.SetRange(1, 16);
	m_Chan.SetFormat(CNumEdit::DF_INT);
	m_Ctrl.SetRange(0, 127);
	m_Ctrl.SetFormat(CNumEdit::DF_INT);
	
	CWnd	*wp = GetWindow(GW_CHILD);
	while (wp != NULL) {	// enable parent notify for all our controls
		wp->ModifyStyleEx(WS_EX_NOPARENTNOTIFY, 0);
		wp = wp->GetNextWindow();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CMidiSetupRow::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	if (m_NotifyWnd != NULL)
		m_NotifyWnd->SendMessage(UWM_MIDIROWEDIT, m_RowIdx, wParam);
	return CRowDialogRow::OnNotify(wParam, lParam, pResult);
}

void CMidiSetupRow::OnSelchangeEvent() 
{
	if (m_NotifyWnd != NULL)
		m_NotifyWnd->SendMessage(UWM_MIDIROWEDIT, m_RowIdx, IDC_MS_EVENT);
}

HBRUSH CMidiSetupRow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CRowDialogRow::OnCtlColor(pDC, pWnd, nCtlColor);
	if (m_Selected && (pWnd == this || pWnd == &m_Title)) {
		if (pWnd == &m_Title)
			pDC->SetBkColor(m_SelColor);	// set title's background color
		hbr	= m_SelBrush;	// set row dialog's background color
	}
	return hbr;
}

void CMidiSetupRow::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_NotifyWnd->SendMessage(UWM_MIDIROWSEL, m_RowIdx);
	CWnd	*wp = GetFocus();
	// if another row has focus, give focus to the same column in our row
	if (wp != NULL && !IsChild(wp)) {	// if focus window isn't one of our controls
		int	id = wp->GetDlgCtrlID();	// get focus window's control ID
		wp = GetDlgItem(id);			// find one of our controls with that ID
	}
	if (wp == NULL)		
		wp = &m_Event;	// default focus to Event column
	wp->SetFocus();
	CEdit	*ep = DYNAMIC_DOWNCAST(CEdit, wp);
	if (ep != NULL)		// if an edit control has focus
		ep->SetSel(0, -1);	// select entire text
	CRowDialogRow::OnLButtonDown(nFlags, point);
}

void CMidiSetupRow::OnParentNotify(UINT message, LPARAM lParam)
{
	if (message == WM_LBUTTONDOWN)
		m_NotifyWnd->SendMessage(UWM_MIDIROWSEL, m_RowIdx);
	CRowDialogRow::OnParentNotify(message, lParam);
}
