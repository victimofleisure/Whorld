// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23jun05	initial version
		01		08jul05	add zoom
		02		13jul05	add tempo and damping
		03		19jul05	add normalized accessors
		04		23jul05	convert to use edit slider
		05		27jul05	add get/set info
		06		05aug05	double damping log base for faster onset
		07		11nov05	add max rings
		08		10feb06	make all edit controls allow editing
        09      04mar06	add trail
        10      17apr06	move INFO struct into CPatch
		11		18apr06	add HueLoopLength and CanvasScale
		12		21jun06	make dialog resizable
		13		24jun06	add Copies and Spread
		14		14aug06	CNumEdit fix allows send instead of post
		15		21dec07	replace AfxGetMainWnd with GetThis 
		16		28jan08	support Unicode
		17		29jan08	in OnNotify, make for loop ANSI
		18		30jan08	use main keyboard accelerators

        master dialog
 
*/

// MasterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MasterDlg.h"
#include <math.h>
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMasterDlg dialog

IMPLEMENT_DYNAMIC(CMasterDlg, CToolDlg);

const CEditSliderCtrl::INFO CMasterDlg::m_SliderInfo[SLIDERS] = {
//	Range	Range	Log		Slider	Default	Tic		Edit	Edit
//	Min		Max		Base	Scale	Pos		Count	Scale	Precision
	{-100,	100,	20,		100,	0,		0,		.01f,	0},	// Speed
	{-100,	100,	10,		100,	0,		0,		.01f,	0},	// Zoom
	{0,		100,	20,		101,	50,		0,		100,	0},	// Damping
	{0,		100,	10,		100,	0,		0,		.01f,	1},	// Trail
	{0,		100,	30,		100,	100,	0,		1,		0},	// Rings
	{0,		250,	0,		1,		100,	0,		1,		2},	// Tempo
	{0,		360,	0,		1,		30,		0,		1,		0},	// Hue Loop
	{100,	300,	0,		100,	140,	0,		.01f,	0},	// Canvas
	{1,		20,		0,		1,		0,		0,		1,		0},	// Copies
	{0,		600,	0,		1,		0,		0,		1,		0},	// Spread
};

const float CMasterDlg::CDampingSlider::BASE	= m_SliderInfo[DAMPING].LogBase;
const float CMasterDlg::CDampingSlider::SCALE	= m_SliderInfo[DAMPING].SliderScale;
const float CMasterDlg::CDampingEdit::SCALE		= m_SliderInfo[DAMPING].EditScale;

const float CMasterDlg::CRingsSlider::BASE		= m_SliderInfo[RINGS].LogBase;
const float CMasterDlg::CRingsSlider::SCALE		= m_SliderInfo[RINGS].SliderScale;

const float CMasterDlg::CTrailSlider::BASE		= m_SliderInfo[TRAIL].LogBase;
const float CMasterDlg::CTrailSlider::SCALE		= m_SliderInfo[TRAIL].SliderScale;

CMasterDlg::CMasterDlg(CWnd* pParent /*=NULL*/)
	: CToolDlg(CMasterDlg::IDD, IDR_MAINFRAME, _T("MasterDlg"), pParent)
{
	//{{AFX_DATA_INIT(CMasterDlg)
	//}}AFX_DATA_INIT
	m_Damping = 1;
	#undef MASTERDEF
	#define MASTERDEF(tag, name, type) m_Slider[tag] = &m_##name##Slider;
	#include "MasterDef.h"	// emit m_Slider initialization for each slider
	m_InitRect.SetRectEmpty();
}

void CMasterDlg::DoDataExchange(CDataExchange* pDX)
{
	CToolDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMasterDlg)
	DDX_Control(pDX, IDC_MAST_SPREAD_SLIDER, m_SpreadSlider);
	DDX_Control(pDX, IDC_MAST_SPREAD_EDIT, m_SpreadEdit);
	DDX_Control(pDX, IDC_MAST_COPIES_SLIDER, m_CopiesSlider);
	DDX_Control(pDX, IDC_MAST_COPIES_EDIT, m_CopiesEdit);
	DDX_Control(pDX, IDC_MAST_CANVAS_SLIDER, m_CanvasScaleSlider);
	DDX_Control(pDX, IDC_MAST_CANVAS_EDIT, m_CanvasScaleEdit);
	DDX_Control(pDX, IDC_MAST_HUE_LOOP_SLIDER, m_HueLoopLengthSlider);
	DDX_Control(pDX, IDC_MAST_HUE_LOOP_EDIT, m_HueLoopLengthEdit);
	DDX_Control(pDX, IDC_MAST_TRAIL_SLIDER, m_TrailSlider);
	DDX_Control(pDX, IDC_MAST_TRAIL_EDIT, m_TrailEdit);
	DDX_Control(pDX, IDC_MAST_RINGS_SLIDER, m_RingsSlider);
	DDX_Control(pDX, IDC_MAST_RINGS_EDIT, m_RingsEdit);
	DDX_Control(pDX, IDC_MAST_TEMPO_EDIT, m_TempoEdit);
	DDX_Control(pDX, IDC_MAST_DAMPING_EDIT, m_DampingEdit);
	DDX_Control(pDX, IDC_MAST_ZOOM_EDIT, m_ZoomEdit);
	DDX_Control(pDX, IDC_MAST_SPEED_EDIT, m_SpeedEdit);
	DDX_Control(pDX, IDC_MAST_TEMPO_SLIDER, m_TempoSlider);
	DDX_Control(pDX, IDC_MAST_DAMPING_SLIDER, m_DampingSlider);
	DDX_Control(pDX, IDC_MAST_ZOOM_SLIDER, m_ZoomSlider);
	DDX_Control(pDX, IDC_MAST_SPEED_SLIDER, m_SpeedSlider);
	//}}AFX_DATA_MAP
}

inline double CMasterDlg::Log(double Base, double x)
{
	return(log(x) / log(Base));	// log with arbitrary base
}

inline double CMasterDlg::LogNorm(double x, double Base, double Scale)
{
	// x is exponential from 0..1, result is linear from 0..Scale
	return(Log(Base, x * (Base - 1) + 1) * Scale);
}

inline double CMasterDlg::ExpNorm(double x, double Base, double Scale)
{
	// x is linear from 0..Scale, result is exponential from 0..1
	return((pow(Base, x / Scale) - 1) / (Base - 1));
}

double CMasterDlg::CDampingSlider::Norm(double x) const
{
	return(SCALE - CMasterDlg::LogNorm(x, BASE, SCALE));
}

double CMasterDlg::CDampingSlider::Denorm(double x) const
{
	return(CMasterDlg::ExpNorm(SCALE - x, BASE, SCALE));
}

void CMasterDlg::CDampingEdit::ValToStr(CString& Str)
{
	Str.Format(_T("%.1f"), SCALE - m_Val * SCALE);
}

void CMasterDlg::CDampingEdit::StrToVal(LPCTSTR Str)
{
	m_Val = (SCALE - _tstof(Str)) / SCALE;
}

double CMasterDlg::CRingsSlider::Norm(double x) const
{
	return(CMasterDlg::LogNorm(x / CPatch::MAX_RINGS, BASE, SCALE));
}

double CMasterDlg::CRingsSlider::Denorm(double x) const
{
	return(CMasterDlg::ExpNorm(x, BASE, SCALE) * CPatch::MAX_RINGS);
}

double CMasterDlg::CTrailSlider::Norm(double x) const
{
	return(CMasterDlg::LogNorm(x, BASE, SCALE));
}

double CMasterDlg::CTrailSlider::Denorm(double x) const
{
	return(CMasterDlg::ExpNorm(x, BASE, SCALE));
}

void CMasterDlg::CRingsEdit::ValToStr(CString& Str)
{
	if (m_Val >= CPatch::MAX_RINGS)
		Str.LoadString(IDS_MAST_MAX_RINGS);
	else
		CNumEdit::ValToStr(Str);
}

void CMasterDlg::GetDefaults(INFO& Info) const
{
	Info = CPatch::m_MasterDefaults;
}

void CMasterDlg::GetInfo(INFO& Info) const
{
	#undef MASTERDEF
	#define MASTERDEF(tag, name, type) Info.name = m_##name##Slider.GetVal();
	#include "MasterDef.h"
}

void CMasterDlg::SetInfo(const INFO& Info)
{
	#undef MASTERDEF
	#define MASTERDEF(tag, name, type) m_##name##Slider.SetVal(Info.name);
	#include "MasterDef.h"
}

void CMasterDlg::RestoreDefaults()
{
	INFO	Info;
	GetDefaults(Info);
	SetInfo(Info);
}

BEGIN_MESSAGE_MAP(CMasterDlg, CToolDlg)
	//{{AFX_MSG_MAP(CMasterDlg)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMasterDlg message handlers

BOOL CMasterDlg::OnInitDialog() 
{
	CToolDlg::OnInitDialog();
	
	GetWindowRect(m_InitRect);
	m_Resize.SetParentWnd(this);
	for (int i = 0; i < SLIDERS; i++) {
		CEditSliderCtrl	*sp = m_Slider[i];
		CNumEdit	*ep = (CNumEdit *)sp->GetNextWindow();	// assume edit is next
		sp->SetInfo(m_SliderInfo[i], ep);
		m_Resize.AddControl(m_Slider[i]->GetDlgCtrlID(), BIND_LEFT | BIND_RIGHT);
		m_Resize.AddControl(ep->GetDlgCtrlID(), BIND_RIGHT);
	}
	m_Resize.FixControls();
	m_SpeedEdit.SetRange(0, 1e2);		// prevent math errors, CPU overload
	m_ZoomEdit.SetRange(1e-3, 1e2);		// prevent math errors, CPU overload
	m_DampingEdit.SetRange(1e-3, 1);	// prevent stuck origin, math errors
	m_TrailEdit.SetRange(0, 1);
	m_RingsEdit.SetRange(0, CPatch::MAX_RINGS);
	m_TempoEdit.SetRange(0, INT_MAX);
	m_TempoSlider.SetPageSize(10);
	m_HueLoopLengthEdit.SetRange(m_SliderInfo[HUE_LOOP].RangeMin,
		m_SliderInfo[HUE_LOOP].RangeMax);
	m_CanvasScaleEdit.SetRange(m_SliderInfo[CANVAS].RangeMin / 100.0,
		m_SliderInfo[CANVAS].RangeMax / 100.0);

	return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL CMasterDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	NMHDR	*nmh = (NMHDR *)lParam;
	int	i;
	for (i = 0; i < SLIDERS; i++) {
		if (nmh->hwndFrom == m_Slider[i]->GetEditCtrl()->m_hWnd)
			break;
	}
	if (i < SLIDERS)
		CMainFrame::GetThis()->SendMessage(UWM_MASTEREDIT, i);
	return CToolDlg::OnNotify(wParam, lParam, pResult);
}

void CMasterDlg::OnSize(UINT nType, int cx, int cy) 
{
	CToolDlg::OnSize(nType, cx, cy);
	m_Resize.OnSize();
}

void CMasterDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (!m_InitRect.IsRectEmpty()) {
		lpMMI->ptMinTrackSize = CPoint(m_InitRect.Width(), m_InitRect.Height());
		lpMMI->ptMaxTrackSize = CPoint(INT_MAX, m_InitRect.Height());
	}
}

void CMasterDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CToolDlg::OnShowWindow(bShow, nStatus);
	if (bShow)
		m_Resize.OnSize();
}
