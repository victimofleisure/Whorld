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
		03		28apr05	relay param update to main frame
		04		29apr05	move view update to main frame
		05		04may05	remove get/set view params
		06		17may05	add form view
		07		19may05	add row tab handler
		08		21may05	add parm info object
		09		02jun05	make frame dynamic for proper cleanup
        10      15jul05	derive from row dialog
		11		22jul05	add row order
		12		06sep05	add SetOffsetMode
		13		02oct05	add edit selections to system menu
		14		17feb06	enable oscillator controls in offset mode
		15		21dec07	replace AfxGetMainWnd with GetThis 
		16		22jan08	add special caption for global rotation
		17		28jan08	support Unicode
		18		29jan08	in OnInitDialog, make for loop ANSI
		19		30jan08	use main keyboard accelerators

        parameters dialog
 
*/

// ParmsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ParmsDlg.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CParmsDlg dialog

IMPLEMENT_DYNAMIC(CParmsDlg, CRowDialog);

const CRowDialog::COLINFO CParmsDlg::m_ColInfo[COLS] = {
	{IDC_PM_TITLE,			0},
	{IDC_PM_PARM_SLIDER,	IDS_PM_SLIDER},
	{IDC_PM_PARM_EDIT,		IDS_PM_VALUE},
	{IDC_PM_MOD_WAVE,		IDS_PM_WAVE},
	{IDC_PM_MOD_AMP,		IDS_PM_AMP},
	{IDC_PM_MOD_FREQ,		IDS_PM_FREQ},
	{IDC_PM_MOD_PW,			IDS_PM_PW},
};

CParmsDlg::CParmsDlg(CWnd* pParent /*=NULL*/)
	: CRowDialog(CParmsDlg::IDD, IDR_MAINFRAME, _T("ParmsDlg"), pParent)
{
	//{{AFX_DATA_INIT(CParmsDlg)
	//}}AFX_DATA_INIT
	m_Frm = NULL;
	for (int i = 0; i < ROWS; i++)	// storage and display orders differ
		m_RowPos[CParmInfo::m_RowOrder[i]] = i;	// reverse lookup for list position
}

BOOL CParmsDlg::Create(UINT nIDTemplate, CMainFrame *pFrame)
{
	m_Frm = pFrame;
	return CRowDialog::Create(nIDTemplate, pFrame);
}

void CParmsDlg::DoDataExchange(CDataExchange* pDX)
{
	CRowDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CParmsDlg)
	//}}AFX_DATA_MAP
}

void CParmsDlg::GetInfo(CParmInfo& Info) const
{
	for (int i = 0; i < ROWS; i++)
		GetRow(i)->GetInfo(Info.m_Row[i]);
}

void CParmsDlg::SetInfo(const CParmInfo& Info)
{
	for (int i = 0; i < ROWS; i++)
		GetRow(i)->SetInfo(Info.m_Row[i]);
}

void CParmsDlg::SetDocTitle(LPCTSTR Title)
{
	SetWindowText(m_Caption + " - " + Title);
}

void CParmsDlg::Reset()
{
	CParmInfo	Info;
	Info.SetDefaults();
	SetInfo(Info);
}

CWnd *CParmsDlg::CreateRow(int Idx, int& Pos)
{
	const CParmInfo::ROWDATA	*rdp = &CParmInfo::m_RowData[Idx];
	CParmRow	*rp = new CParmRow;
	rp->Create(IDD_PARM_ROW);
	rp->SetSliderRange(rdp->MinVal, rdp->MaxVal, rdp->Steps);
	CString	s((LPCTSTR)rdp->TitleID);
	rp->SetCaption(s + ":");
	rp->SetScale(rdp->Scale);
	Pos = m_RowPos[Idx];	// find row's position in list
	return(rp);
}

void CParmsDlg::SetOffsetMode(bool Enable)
{
	for (int i = 0; i < ROWS; i++) {
		double	Lower, Upper;
		int	Steps;
		const CParmInfo::ROWDATA	*rdp = &CParmInfo::m_RowData[i];
		if (Enable) {
			Lower = -rdp->MaxVal;
			Upper = rdp->MaxVal;
			Steps = 64;
		} else {
			Lower = rdp->MinVal;
			Upper = rdp->MaxVal;
			Steps = rdp->Steps;
		}
		GetRow(i)->SetSliderRange(Lower, Upper, Steps);
	}
	bool	IsGlob = m_Frm->GetEditSel() == CMainFrame::SEL_GLOBALS;
	CString	RotateCap;
	RotateCap.LoadString(IsGlob ? IDS_VP_ROTATION : IDS_VP_ROTATE_SPEED);
	GetRow(CParmInfo::ROTATE_SPEED)->SetCaption(RotateCap + ":");
}

BEGIN_MESSAGE_MAP(CParmsDlg, CRowDialog)
	//{{AFX_MSG_MAP(CParmsDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_INITMENUPOPUP()
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_PARMROWEDIT, OnParmRowEdit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParmsDlg message handlers

BOOL CParmsDlg::OnInitDialog() 
{
	CRowDialog::OnInitDialog();

	GetWindowText(m_Caption);
	CreateCols(COLS, m_ColInfo);
	CreateRows(ROWS);

	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);
	CMenu	*pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL) {
		int	i;
		for (i = 0; i < CMainFrame::SELS; i++) {
			int	id = EDIT_SEL_SYSCMD + (i << 4);
			pSysMenu->InsertMenu(i, MF_BYPOSITION, id, m_Frm->GetSelName(i));
		}
		pSysMenu->InsertMenu(i, MF_BYPOSITION, MF_SEPARATOR);
	}

	return FALSE;	// CreateRows sets focus to a control
}

LRESULT CParmsDlg::OnParmRowEdit(WPARAM wParam, LPARAM lParam)
{
	CMainFrame::GetThis()->SendMessage(UWM_PARMROWEDIT, wParam, lParam);	// relay to main
	return(TRUE);
}

void CParmsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	int	idx = (nID - EDIT_SEL_SYSCMD) >> 4;
	if (idx >= 0 && idx < CMainFrame::SELS)
		m_Frm->SetEditSel(idx);
	else
		CRowDialog::OnSysCommand(nID, lParam);
}

void CParmsDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	int	EditSel = m_Frm->GetEditSel();
	for (int i = 0; i < CMainFrame::SELS; i++)
		pPopupMenu->CheckMenuItem(i, MF_BYPOSITION | (i == EditSel ? MF_CHECKED : 0));
	CRowDialog::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
}
