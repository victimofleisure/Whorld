// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      03jun06	initial version
		01		10dec07	add global parameters
		02		21dec07	rename GetMainFrame to GetThis
		03		28jan08	support Unicode

        thumbnail manager
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "ThumbList.h"
#include "AviToBmp.h"
#include "Patch.h"
#include "MainFrm.h"
#include "WhorldView.h"
#include "ProgressDlg.h"
#include "shlwapi.h"

CThumbList::CThumbList()
{
	m_ThmSize = CSize(100, 75);
}

inline bool CThumbList::IsVideo(LPCTSTR Path)
{
	return(!_tcsicmp(PathFindExtension(Path), EXT_AVI));
}

bool CThumbList::Create(CSize ThmSize)
{
	m_SrcPath.RemoveAll();
	m_ThmSize = ThmSize;
	m_ThmList.DeleteImageList();
	return(m_ThmList.Create(ThmSize.cx, ThmSize.cy, ILC_COLOR24, 0, 0) != 0);
}

void CThumbList::RemoveAll()
{
	m_SrcPath.RemoveAll();
	if (IsCreated())
		m_ThmList.SetImageCount(0);
}

bool CThumbList::CreateThumb(CDC& ThumbDC, CBitmap& Thumb, CSize sz)
{
	BITMAPINFO	bmi;
	ZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(bmi);
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biWidth = sz.cx;
	bmi.bmiHeader.biHeight = sz.cy;
	void	*ThumbBits;
	HBITMAP	hThumb = CreateDIBSection(NULL, &bmi,	// CImageList expects DIBs
		DIB_RGB_COLORS, &ThumbBits, 0, 0);
	if (hThumb == NULL)
		return(FALSE);
	if (!Thumb.Attach(hThumb)) {	// attach bitmap handle to CBitmap object
		DeleteObject(hThumb);
		return(FALSE);
	}
	if (!ThumbDC.CreateCompatibleDC(NULL))
		return(FALSE);
	if (!ThumbDC.SetStretchBltMode(HALFTONE))	// slower but higher quality
		return(FALSE);
	return(TRUE);
}

bool CThumbList::DrawThumb(LPCTSTR Path, CDC& ThumbDC, CBitmap& Thumb, CSize sz)
{
	if (IsVideo(Path)) {
		CAviToBmp	avi;
		if (!avi.Open(Path))
			return(FALSE);
		LPBITMAPINFOHEADER lpbi = avi.GetFrame(0);	// returns packed DIB
		if (lpbi != NULL) {
			LPBYTE pData = (LPBYTE)lpbi + lpbi->biSize;
			HGDIOBJ	PrevObj = SelectObject(ThumbDC, Thumb);
			StretchDIBits(ThumbDC, 0, 0, sz.cx, sz.cy,
				0, 0, lpbi->biWidth, lpbi->biHeight, pData, 
				(LPBITMAPINFO)lpbi, DIB_RGB_COLORS, SRCCOPY);
			SelectObject(ThumbDC, PrevObj);	// required
		}
	} else {
		CPatch	Patch;
		if (!Patch.Read(Path))
			return(FALSE);
		CMainFrame	*Frm = CMainFrame::GetThis();
		CWhorldView	*View = Frm->GetView();
		View->ClearScreen();
		View->FlushHistory();
		View->GetVideo().Deselect();
		CParmInfo	*Info = &Patch;
		if (Frm->GetPatchMode() == CMainFrame::PM_FULL) {
			Patch.m_Main.Pause = TRUE;	// so refresh stays disabled
			Frm->SetPatch(Patch);	// restore master and main settings
		}
		View->SetNormOrigin(DPoint(.5, .5));	// center the drawing
		double	Speed = Frm->GetReverse() ? -1 : 1;	// default master speed
		CWhorldView::PARMS	GlobParm;
		ZeroMemory(&GlobParm, sizeof(GlobParm));
		for (int i = 0; i < VIEW_TICKS; i++) {
			View->StepRings(TRUE);		// step rings forward one tick
			View->TimerHook(*Info, GlobParm, Speed);	// update oscillators and add rings
			View->ValidateRect(NULL);	// keep view from painting
		}
		CSize	DibSize(VIEW_WIDTH, VIEW_HEIGHT);
		HBITMAP	dib = View->MakeDIB(&DibSize);
		CDC	dc;
		dc.CreateCompatibleDC(&ThumbDC);
		HGDIOBJ	PrevObj = SelectObject(ThumbDC, Thumb);
		HGDIOBJ	PrevObj2 = SelectObject(dc, dib);
		StretchBlt(ThumbDC, 0, 0, sz.cx, sz.cy,
			dc, 0, 0, DibSize.cx, DibSize.cy, SRCCOPY);
		SelectObject(ThumbDC, PrevObj);	// else image list add fails
		SelectObject(dc, PrevObj2);	// order matters
		DeleteObject(dib);
	}
	return(TRUE);
}

int CThumbList::Find(LPCTSTR Path) const
{
	int	Paths = GetCount();
	for (int i = 0; i < Paths; i++) {
		if (m_SrcPath[i] == Path)
			return(i);
	}
	return(-1);	// path not found
}

int CThumbList::Add(LPCTSTR Path)
{
	if (!IsCreated())
		return(-1);
	int	ThmIdx = Find(Path);
	if (ThmIdx < 0) {	// if path not found
		CDC	ThumbDC;
		CBitmap	Thumb;
		if (CreateThumb(ThumbDC, Thumb, m_ThmSize)) {
			CMainFrame::TimerState	ts(FALSE);	// disable refresh until out of scope
			CSnapshot::ViewBackup	vb;	// restore view state when out of scope
			CMainFrame::INFO	MainInfo;
			CMainFrame	*Frm = CMainFrame::GetThis();
			Frm->GetInfo(MainInfo);
			if (DrawThumb(Path, ThumbDC, Thumb, m_ThmSize)) {
				ThmIdx = m_ThmList.Add(&Thumb, RGB(0, 0, 0));	// add thumbnail
				if (ThmIdx >= 0)
					m_SrcPath.Add(Path);	// add thumbnail's path
			}
			Frm->SetInfo(MainInfo);	// restore main info saved above
		}
	}
	return(ThmIdx);
}

bool CThumbList::Add(const CStringArray& PathList, CDWordArray& ThmIdxList)
{
	if (!IsCreated())
		return(FALSE);
	CDC	ThumbDC;
	CBitmap	Thumb;
	if (!CreateThumb(ThumbDC, Thumb, m_ThmSize))
		return(FALSE);
	int	Paths = PathList.GetSize();
	CMainFrame::TimerState	ts(FALSE);	// disable refresh until out of scope
	CSnapshot::ViewBackup	vb;	// restore view state when out of scope
	CMainFrame::INFO	MainInfo;
	CMainFrame	*Frm = CMainFrame::GetThis();
	Frm->GetInfo(MainInfo);
	ThmIdxList.SetSize(Paths);
	int	i;
	int	NewThumbs = 0;
	for (i = 0; i < Paths; i++) {
		int	ThmIdx = Find(PathList[i]);
		if (ThmIdx < 0)	// if path not found
			NewThumbs++;	// path needs a thumbnail
		ThmIdxList[i] = ThmIdx;
	}
	if (NewThumbs) {	// if at least one path needs a thumbnail
		CProgressDlg	prog;
		prog.Create();
		prog.SetRange(0, NewThumbs);
		prog.SetWindowText(LDS(IDS_CREATING_THUMBS));
		int	Pos = 0;
		for (i = 0; i < Paths; i++) {
			if (int(ThmIdxList[i]) >= 0)	// if path already has a thumbnail
				continue;	// skip it
			prog.SetPos(Pos++);
			if (prog.Canceled())
				break;
			LPCTSTR	Path = PathList[i];
			if (DrawThumb(Path, ThumbDC, Thumb, m_ThmSize)) {
				int	ThmIdx = m_ThmList.Add(&Thumb, RGB(0, 0, 0));	// add thumbnail
				if (ThmIdx >= 0)
					m_SrcPath.Add(Path);	// add thumbnail's path
				ThmIdxList[i] = ThmIdx;
			}
		}
	}
	Frm->SetInfo(MainInfo);	// restore main info saved above
	return(i >= Paths);	// return false if user canceled
}
