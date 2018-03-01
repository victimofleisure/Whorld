// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		23apr05	don't let LoadWnd resize dialog
		02		27apr05	auto-size dialog to fit data
		03		21may05	add parm info object
		04		02jul05	add hue
		05		12jul05	add origin
		06		22jul05	add row order
		07		28jan08	support Unicode
		08		30jan08	use main keyboard accelerators

        display view parameters as numbers
 
*/

// NumbersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "NumbersDlg.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNumbersDlg dialog

IMPLEMENT_DYNAMIC(CNumbersDlg, CToolDlg);

CNumbersDlg::CNumbersDlg(CWnd* pParent /*=NULL*/)
	: CToolDlg(CNumbersDlg::IDD, IDR_MAINFRAME, _T("NumbersDlg"), pParent)
{
	//{{AFX_DATA_INIT(CNumbersDlg)
	//}}AFX_DATA_INIT
	m_Frm = NULL;
	m_View = NULL;
}

BOOL CNumbersDlg::Create(UINT nIDTemplate, CMainFrame *pFrame)
{
	m_Frm = pFrame;
	return CToolDlg::Create(nIDTemplate, pFrame);
}

void CNumbersDlg::DoDataExchange(CDataExchange* pDX)
{
	CToolDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNumbersDlg)
	DDX_Control(pDX, IDC_RES_CAPS, m_Caps);
	DDX_Control(pDX, IDC_RES_VALS, m_Vals);
	//}}AFX_DATA_MAP
}

void CNumbersDlg::TimerHook()
{
	CWhorldView::PARMS	vp;
	m_View->GetParms(vp);
	CString	Text, s;
	for (int i = 0; i < CParmInfo::ROWS; i++) {
		int	Idx = CParmInfo::m_RowOrder[i];	// storage and display orders differ
		s.Format(_T("%.3f"), ((double *)&vp)[Idx] / CParmInfo::m_RowData[Idx].Scale);
		Text += s.Left(6) + "\n";
		if (Idx == CParmInfo::COLOR_SPEED) {	// insert hue after color speed
			s.Format(_T("%.1f\n"), m_View->GetHue());
			Text += s.Left(6);
		}
	}
	DPOINT	org;
	m_View->GetNormOrigin(org);
	s.Format(_T("%.3f\n%.3f\n"), org.x, org.y);
	Text += s;
	s.Format(_T("%d"), m_View->GetRingCount());
	Text += s;
	m_Vals.SetWindowText(Text);
}

BEGIN_MESSAGE_MAP(CNumbersDlg, CToolDlg)
	//{{AFX_MSG_MAP(CNumbersDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNumbersDlg message handlers

BOOL CNumbersDlg::OnInitDialog() 
{
	CToolDlg::OnInitDialog();

	m_View = m_Frm->GetView();
	// append row titles to caption, one per line
	CString	Text, s;
	for (int i = 0; i < CParmInfo::ROWS; i++) {
		int	Idx = CParmInfo::m_RowOrder[i];	// storage and display orders differ
		s.LoadString(CParmInfo::m_RowData[Idx].TitleID);
		Text +=  s + ":\n";
		if (Idx == CParmInfo::COLOR_SPEED) {
			s.LoadString(IDS_VP_HUE);
			Text +=  s + ":\n";
		}
	}
	s.LoadString(IDS_VP_ORIGIN_X);
	Text += s + ":\n";
	s.LoadString(IDS_VP_ORIGIN_Y);
	Text += s + ":\n";
	s.LoadString(IDS_VP_RING_COUNT);
	Text += s + ":";
	m_Caps.SetWindowText(Text);
	// set our height to fit around text
	CRect	TextRect;
	TextRect.SetRectEmpty();
	CClientDC	dc(this);
	dc.SelectObject(GetFont());
	dc.DrawText(Text, TextRect, DT_CALCRECT);	// measure text
	CRect	wr, cr;
	m_Caps.GetWindowRect(cr);
	ScreenToClient(cr);
	TextRect.bottom += cr.top * 2 + 1;	// add top and bottom margins
	GetWindowRect(wr);
	GetClientRect(cr);
	ClientToScreen(cr);	// assume we're a top-level window
	int	NCHeight = wr.Height() - cr.Height();
	ClientToScreen(TextRect);
	wr.bottom = wr.top + NCHeight + TextRect.Height();
	MoveWindow(wr);

	return TRUE;  // return TRUE unless you set the focus to a control
}
