// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      03sep05	initial version
		01		20sep05	add output frame rate, frame range, export bitmaps
		02		22sep05	if uncompressed, check output file size
		03		27sep05	pass input frame size to view
		04		27sep05	disable origin clamping
		05		04feb06	replace VFW BmpToAvi with DirectShow version
		06		10feb06	display all times in mm:ss:ff format
		07		29mar06	remove origin clamping
		08		03may06	add optional path and frame range
		09		04may06	add FrameRange arg to UpdateUI
		10		04may06	prevent divide by zero in FrameToTime
		11		02oct06	make output frame rate a float
		12		21dec07	rename GetMainFrame to GetThis
		13		27jan08	make codec settings persistent
		14		29jan08	in Export, remove unused local var

        movie export options dialog
 
*/

// MovieExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MovieExportDlg.h"
#include "SnapMovie.h"
#include "BmpToAvi.h"
#include "ProgressDlg.h"
#include "PathStr.h"
#include "MainFrm.h"
#include "MultiFileDlg.h"
#include "Whorld.h"

// MakeSureDirectoryPathExists doesn't support Unicode; SHCreateDirectoryEx
// is a reasonable substitute, but our version of the SDK doesn't define it
#ifdef UNICODE
#ifndef SHCreateDirectoryEx
int WINAPI SHCreateDirectoryExW(HWND hwnd, LPCWSTR pszPath, SECURITY_ATTRIBUTES *psa);
#define SHCreateDirectoryEx SHCreateDirectoryExW
#endif
#else
#include "imagehlp.h"	// for MakeSureDirectoryPathExists
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMovieExportDlg dialog

IMPLEMENT_DYNAMIC(CMovieExportDlg, CDialog);

#define REG_VALUE(name, defval) m_Reg##name(_T("MovieExp")_T(#name), m_st.name, defval)

const SIZE CMovieExportDlg::m_StandardSize[] = {
	{320, 240},
	{640, 480},
	{0, 0}	// list terminator, don't delete
};

static const LPCTSTR COMPR_STATE_FILE_NAME = _T("ComprState.dat");

CMovieExportDlg::CMovieExportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMovieExportDlg::IDD, pParent),
	REG_VALUE(Width, 640),
	REG_VALUE(Height, 480),
	REG_VALUE(ScaleToFit, FALSE)
{
	//{{AFX_DATA_INIT(CMovieExportDlg)
	m_ScaleToFit = 0;
	m_OutHeight = 0;
	m_OutWidth = 0;
	m_OutLength = _T("");
	m_RangeSubset = 0;
	m_ExportBitmaps = FALSE;
	m_DisplayOutput = TRUE;
	m_RangeFirstEdit = _T("");
	m_RangeLastEdit = _T("");
	m_OutFrameRate = 0.0f;
	//}}AFX_DATA_INIT
	m_InFrameSize = CSize(0, 0);
	m_InFrameCount = 0;
	m_InFrameRate = 0;
	m_RangeSubset = FALSE;
	m_RangeFirst = 0;
	m_RangeLast = 0;
}

void CMovieExportDlg::FrameToTime(int Frame, int FrameRate, CString& Time)
{
	if (FrameRate > 0) {
		int	Secs = Frame / FrameRate;
		Time.Format(_T("%d:%02d:%02d"), Secs / 60, Secs % 60, Frame % FrameRate);
	} else
		Time.Empty();
}

int CMovieExportDlg::TimeToFrame(LPCTSTR Time, int FrameRate)
{
	static const int PLACES = 3;	// minutes, seconds, frames
	int	ip[PLACES], op[PLACES];	// input and output place arrays
	ZeroMemory(op, sizeof(op));
	int	ps = _stscanf(Time, _T("%d%*[: ]%d%*[: ]%d"), &ip[0], &ip[1], &ip[2]);
	if (ps >= 0)
		CopyMemory(&op[PLACES - ps], ip, ps * sizeof(int));
	return(op[0] * FrameRate * 60 + op[1] * FrameRate + op[2]);
}

void CMovieExportDlg::SetInput(CSize FrameSize, int FrameCount, int FrameRate)
{
	m_InFrameSize	= FrameSize;
	m_InFrameCount	= FrameCount;
	m_InFrameRate	= FrameRate;
}

void CMovieExportDlg::GetUncomprSize(ULARGE_INTEGER& Size) const
{
	Size.QuadPart = m_st.Width * m_st.Height * 3;	// assume 24-bit color
	Size.QuadPart *= m_RangeLast - m_RangeFirst;
}

void CMovieExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMovieExportDlg)
	DDX_Control(pDX, IDC_MEX_OUT_SIZE, m_OutSize);
	DDX_Radio(pDX, IDC_MEX_SCALE_TO_FIT, m_ScaleToFit);
	DDX_Text(pDX, IDC_MEX_OUT_HEIGHT, m_OutHeight);
	DDX_Text(pDX, IDC_MEX_OUT_WIDTH, m_OutWidth);
	DDX_Text(pDX, IDC_MEX_OUT_LENGTH, m_OutLength);
	DDX_Radio(pDX, IDC_MEX_RANGE_SUBSET, m_RangeSubset);
	DDX_Check(pDX, IDC_MEX_EXPORT_BITMAPS, m_ExportBitmaps);
	DDX_Check(pDX, IDC_MEX_DISPLAY_OUTPUT, m_DisplayOutput);
	DDX_Text(pDX, IDC_MEX_RANGE_FIRST, m_RangeFirstEdit);
	DDX_Text(pDX, IDC_MEX_RANGE_LAST, m_RangeLastEdit);
	DDX_Text(pDX, IDC_MEX_OUT_FRAME_RATE, m_OutFrameRate);
	DDV_MinMaxFloat(pDX, m_OutFrameRate, 24.f, 60.f);
	//}}AFX_DATA_MAP
}

void CMovieExportDlg::UpdateUI(bool RangeChange)
{
	UpdateData(TRUE);	// retrieve data
	GetDlgItem(IDC_MEX_OUT_WIDTH)->EnableWindow(IsCustomSize());
	GetDlgItem(IDC_MEX_OUT_HEIGHT)->EnableWindow(IsCustomSize());
	if (!IsCustomSize()) {
		CSize	sz = m_StandardSize[m_OutSize.GetCurSel()];
		m_OutWidth = sz.cx;
		m_OutHeight = sz.cy;
	}
	if (RangeChange) {	// get range values from edit controls
		m_RangeFirst = TimeToFrame(m_RangeFirstEdit, m_InFrameRate);
		m_RangeFirst = CLAMP(m_RangeFirst, 0, m_InFrameCount - 1);
		m_RangeLast = TimeToFrame(m_RangeLastEdit, m_InFrameRate);
		m_RangeLast = CLAMP(m_RangeLast, 0, m_InFrameCount - 1);
	}
	if (!m_RangeSubset) {	// do before updating length
		m_RangeFirst = 0;
		m_RangeLast = m_InFrameCount - 1;
	}
	int	Frames = max(m_RangeLast - m_RangeFirst + 1, 0);
	FrameToTime(Frames, round(m_OutFrameRate), m_OutLength);
	GetDlgItem(IDC_MEX_RANGE_FIRST)->EnableWindow(m_RangeSubset);
	GetDlgItem(IDC_MEX_RANGE_LAST)->EnableWindow(m_RangeSubset);
	FrameToTime(m_RangeFirst, m_InFrameRate, m_RangeFirstEdit);
	FrameToTime(m_RangeLast, m_InFrameRate, m_RangeLastEdit);
	UpdateData(FALSE);	// init dialog
}

bool CMovieExportDlg::CheckDiskSpace(LPCTSTR Folder, ULARGE_INTEGER Size)
{
	ULARGE_INTEGER	BytesAvl, BytesTot;
	if (GetDiskFreeSpaceEx(Folder, &BytesAvl, &BytesTot, NULL)) {
		bool	IsComp = (GetFileAttributes(Folder) & FILE_ATTRIBUTE_COMPRESSED) != 0;
		if (Size.QuadPart > BytesAvl.QuadPart && !IsComp)
			return(FALSE);
	}
	return(TRUE);
}

int CMovieExportDlg::Export(LPCTSTR SrcPath, int FirstFrame, int LastFrame)
{
	CMainFrame	*Frm = CMainFrame::GetThis();
	ASSERT(Frm != NULL);
	CWhorldView	*View = Frm->GetView();
	CMainFrame::TimerState	ts(FALSE);	// stop timer, dtor restores it
	CSnapshot::ViewBackup	vb;	// save view state, dtor restores it
	CString	SrcPathStr = SrcPath;
	if (SrcPathStr.IsEmpty()) {
		CMultiFileDlg	src(TRUE, EXT_MOVIE, NULL, OFN_HIDEREADONLY,
			LDS(IDS_FILTER_MOVIE), LDS(IDS_MOVIE_OPEN));
		if (src.DoModal() != IDOK)	// get source file path
			return(IDCANCEL);
		SrcPathStr = src.GetPathName();
	}
	if (FirstFrame >= 0 && LastFrame >= 0) {
		m_RangeSubset = TRUE;
		m_RangeFirst = FirstFrame;
		m_RangeLast = LastFrame;
	} else {
		m_RangeSubset = FALSE;
		m_RangeFirst = 0;
		m_RangeLast	= m_InFrameCount - 1;
	}
	CSnapMovie	sm;
	sm.SetView(View);
	if (!sm.Open(SrcPathStr, TRUE))	// open source file
		return(IDCANCEL);
	CPathStr	AviPath = SrcPathStr;
	AviPath.RenameExtension(EXT_AVI);
	CMultiFileDlg	dst(FALSE, EXT_AVI, AviPath, OFN_OVERWRITEPROMPT, LDS(IDS_FILTER_AVI));
	if (dst.DoModal() != IDOK)	// get destination file path
		return(IDCANCEL);
	SetInput(sm.GetFrameSize(), sm.GetFrameCount(), sm.GetFrameRate());
	if (DoModal() != IDOK)	// get export options
		return(IDCANCEL);
	CSize	OutFrameSize = GetFrameSize();
	CProgressDlg	pd;
	sm.Seek(m_RangeFirst);
	CPathStr	DstFolder;
	DstFolder = dst.GetPathName();
	DstFolder.RemoveFileSpec();	// get destination folder
	ULARGE_INTEGER	UncomprSize;
	GetUncomprSize(UncomprSize);
	if (m_ExportBitmaps) {	// if we're exporting bitmaps
		if (!CheckDiskSpace(DstFolder, UncomprSize)) {
			AfxMessageBox(IDS_MEX_NOT_ENOUGH_DISK);
			return(IDABORT);
		}
		CString	s;
		pd.Create();
		pd.SetRange(m_RangeFirst, m_RangeLast);
		int	j = 0;
		CPathStr	path;
		for (int i = m_RangeFirst; i <= m_RangeLast; i++) {
			sm.Read();
			s.Format(_T("%06d.bmp"), j++);	// number bitmaps sequentially
			path = DstFolder;
			path.Append(s);
			CFile	fp;
			CFileException	e;
			if (fp.Open(path, CFile::modeCreate | CFile::modeWrite, &e))
				View->ExportBitmap(fp, &OutFrameSize, &m_InFrameSize, GetScaleToFit(), 72);
			else {
				e.ReportError();
				return(IDABORT);
			}
			if (!m_DisplayOutput)	// prevent view from painting
				View->ValidateRect(NULL);	// must revalidate before SetPos
			pd.SetPos(i);
			if (pd.Canceled())
				return(IDCANCEL);
		}
	} else {	// we're exporting an AVI file
		BMPTOAVI_PARMS	Parms;
		Parms.Width = m_st.Width;
		Parms.Height = m_st.Height;
		Parms.BitCount = 24;
		Parms.FrameRate = m_OutFrameRate;
		CBmpToAvi	bta;
		CVideoComprState	ComprState;
		CPathStr	StateFolder, StatePath;
		CWhorldApp::GetAppDataFolder(StateFolder);
		StateFolder.Append(AfxGetApp()->m_pszAppName);
		StatePath = StateFolder;
		StatePath.Append(COMPR_STATE_FILE_NAME);
		if (PathFileExists(StatePath) && ComprState.Read(StatePath))
			bta.SetComprState(ComprState);
		if (bta.Open(Parms, dst.GetPathName(), TRUE)) {
			bta.GetComprState(ComprState);
			if (!PathFileExists(StatePath))	// test using shlwapi, it's faster
#ifdef UNICODE
				SHCreateDirectoryEx(NULL, StateFolder, NULL);
#else
				MakeSureDirectoryPathExists(StatePath);	// very slow
#endif
			ComprState.Write(StatePath);	// write compressor state
			if (!bta.IsCompressed() && !CheckDiskSpace(DstFolder, UncomprSize)) {
				AfxMessageBox(IDS_MEX_NOT_ENOUGH_DISK);
				return(IDABORT);
			}
			pd.Create();
			pd.SetRange(m_RangeFirst, m_RangeLast);
			for (int i = m_RangeFirst; i <= m_RangeLast; i++) {
				sm.Read();
				HBITMAP	bm = View->MakeDIB(&OutFrameSize, &m_InFrameSize, GetScaleToFit());
				bta.AddFrame(bm);
				DeleteObject(bm);
				if (!m_DisplayOutput)	// prevent view from painting
					View->ValidateRect(NULL);	// must revalidate before SetPos
				pd.SetPos(i);
				if (pd.Canceled())
					return(IDCANCEL);
			}
		}
		if (bta.GetLastError()) {
			CString	Msg, Err, DSErr;
			bta.GetLastErrorString(Err, DSErr);
	 		Msg.Format(_T("%s\n%s"), Err, DSErr);
			AfxMessageBox(Msg);
		} 
	}
	return(IDOK);
}

BEGIN_MESSAGE_MAP(CMovieExportDlg, CDialog)
	//{{AFX_MSG_MAP(CMovieExportDlg)
	ON_CBN_SELCHANGE(IDC_MEX_OUT_SIZE, OnSelchangeSizeCombo)
	ON_EN_KILLFOCUS(IDC_MEX_OUT_FRAME_RATE, OnKillfocusOutFrameRate)
	ON_BN_CLICKED(IDC_MEX_RANGE_SUBSET, OnRangeType)
	ON_BN_CLICKED(IDC_MEX_RANGE_SUBSET2, OnRangeType)
	ON_EN_KILLFOCUS(IDC_MEX_RANGE_LAST, OnKillfocusRange)
	ON_EN_KILLFOCUS(IDC_MEX_RANGE_FIRST, OnKillfocusRange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMovieExportDlg message handlers

BOOL CMovieExportDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CString	s;
	s.Format(_T("%d x %d"), m_InFrameSize.cx, m_InFrameSize.cy);
	GetDlgItem(IDC_MEX_IN_SIZE)->SetWindowText(s);
	FrameToTime(m_InFrameCount, m_InFrameRate, s);
	GetDlgItem(IDC_MEX_IN_LENGTH)->SetWindowText(s);
	s.Format(_T("%d"), m_InFrameRate);
	GetDlgItem(IDC_MEX_IN_FRAME_RATE)->SetWindowText(s);
	m_OutFrameRate = float(m_InFrameRate);
	m_OutSize.ResetContent();	// populate output size combo box
	int	sel = -1;	// assume persistent output size is non-standard
	for (int i = 0; m_StandardSize[i].cx; i++) {
		CSize	sz = m_StandardSize[i];
		s.Format(_T("%d x %d"), sz.cx, sz.cy);
		m_OutSize.AddString(s);
		if (sz.cx == m_st.Width && sz.cy == m_st.Height)
			sel = i;	// it's a standard size: store selection index
	}
	m_OutSize.AddString(LDS(IDS_MEX_CUSTOM));
	if (sel < 0)	// if non-standard output size
		sel = m_OutSize.GetCount() - 1;	// select custom
	m_OutSize.SetCurSel(sel);
	m_OutHeight		= m_st.Height;
	m_OutWidth		= m_st.Width;
	m_ScaleToFit	= m_st.ScaleToFit;
	UpdateData(FALSE);	// init dialog
	UpdateUI();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMovieExportDlg::OnOK() 
{
	if (m_RangeFirst > m_RangeLast) {
		AfxMessageBox(IDS_MEX_BAD_FRAME_RANGE);
		return;
	}
	CDialog::OnOK();
	m_st.Width		= m_OutWidth;
	m_st.Height		= m_OutHeight;
	m_st.ScaleToFit	= m_ScaleToFit != 0;
}

void CMovieExportDlg::OnSelchangeSizeCombo() 
{
	UpdateUI();
}

void CMovieExportDlg::OnKillfocusOutFrameRate() 
{
	UpdateUI();
}

void CMovieExportDlg::OnRangeType() 
{
	UpdateUI();
}

void CMovieExportDlg::OnKillfocusRange() 
{
	UpdateUI(TRUE);	// range change
}
