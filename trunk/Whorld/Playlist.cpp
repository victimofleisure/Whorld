// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27jul05	initial version
		01		06aug05	add current patch to playlist info
		02		04sep05	get MIDI row names from setup dialog
		03		20sep05	in Read, fix missing conditional in MIDI for loop
		04		04oct05	add MIDI row event type
		05		05oct05	add playlist MIDI
		06		06oct05	make MIDI line tab-delimited
		07		20oct05	add banks
		08		11nov05 add max rings
		09		17feb06	GetRowName isn't static anymore
		10		19feb06	make write functions const
		11		20feb06	add master parameters I/O
		12		03mar06	add convex
		13		04mar06	add trail
		14		23mar06	use BYTE instead of INT for booleans
		15		23mar06	add Hue, LoopHue, InvertColor, Pause
		16		28mar06	add ZoomCenter
        17      17apr06	rename link to CPatchLink
		18		18apr06	add PatchMode, HueLoopLength, CanvasScale
		19		22may06	add video list info
		20		21jun06	add tag to MasterDef macro
		21		21dec07	rename GetMainFrame to GetThis
		22		23jan08	replace MIDI range scaler with start/end
		23		28jan08	support Unicode
		24		29jan08	in OnNotify, make for loop ANSI
		25		31jan08	add freeframe conditionals

		playlist container
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "Playlist.h"
#include "FormatIO.h"

const LPCTSTR CPlaylist::FILE_ID = _T("WHPL%d");
const LPCTSTR CPlaylist::PATCH_TAG = _T("PLPatch");
const LPCTSTR CPlaylist::MIDI_SETUP_TAG = _T("MSRow");
const LPCTSTR CPlaylist::MASTER_PARM_TAG = _T("MAParm");
const LPCTSTR CPlaylist::GLOB_PARM_TAG = _T("GLParm");

#define PARM_ROW_R_FMT		_T("%s %lf %d %lf %lf %lf")
#define PARM_ROW_W_FMT		_T("%s\t%s\t%g\t%d\t%g\t%g\t%g\n")
#define PATCH_R_FMT			_T("%d %d \"%[^\"]\"")
#define PATCH_W_FMT			_T("%s\t%d\t%d\t\"%s\"\n")
#define MIDI_SETUP_R_FMT	_T("%s %lf %d %d %d %lf")
#define MIDI_SETUP_W_FMT	_T("%s\t%s\t%g\t%d\t%d\t%d\t%g\n")

#define MF_OFS(x) offsetof(CMainFrame::INFO, x)

const CPlaylist::LINE_INFO CPlaylist::m_LineInfo[] = {
	#undef MASTERDEF
	#define MASTERDEF(tag, name, type) {_T("MA")_T(#name), FIO_##type, MF_OFS(Patch.m_Master.name)},
	#include "MasterDef.h"	// build I/O list for master members
	#undef MAINDEF
	#define MAINDEF(name, type) {_T("MF")_T(#name), FIO_##type, MF_OFS(Patch.m_Main.name)},
	#include "MainDef.h"	// build I/O list for main members
	{_T("MFViewSel"),		FIO_INT,	MF_OFS(ViewSel)},
	{_T("MFEditSel"),		FIO_INT,	MF_OFS(EditSel)},
	{_T("MFPatchMode"),		FIO_INT,	MF_OFS(PatchMode)},
	{_T("XFPos"),			FIO_DOUBLE,	MF_OFS(XFader.Pos)},
	{_T("XFSeconds"),		FIO_DOUBLE,	MF_OFS(XFader.Seconds)},
	{_T("XFLoop"),			FIO_BYTE,	MF_OFS(XFader.Loop)},
	{_T("XFWaveform"),		FIO_INT,	MF_OFS(XFader.Waveform)},
	{_T("PLMScheme"),		FIO_INT,	MF_OFS(Playlist.Midi.Scheme)},
	{_T("PLMChan"),			FIO_INT,	MF_OFS(Playlist.Midi.Chan)},
	{_T("PLMFirst"),		FIO_INT,	MF_OFS(Playlist.Midi.First)},
	{_T("PLMLast"),			FIO_INT,	MF_OFS(Playlist.Midi.Last)},
	{_T("PLPatchSecs"),		FIO_DOUBLE,	MF_OFS(Playlist.PatchSecs)},
	{_T("PLXFadeSecs"),		FIO_DOUBLE,	MF_OFS(Playlist.XFadeSecs)},
	{_T("PLPlaying"),		FIO_BYTE,	MF_OFS(Playlist.Playing)},
	{_T("PLShuffle"),		FIO_BYTE,	MF_OFS(Playlist.Shuffle)},
	{_T("PLCurBank"),		FIO_INT,	MF_OFS(Playlist.CurBank)},
	{_T("PLCurPatch"),		FIO_INT,	MF_OFS(Playlist.CurPatch)},
	{_T("PLCurPatchBank"),	FIO_INT,	MF_OFS(Playlist.CurPatchBank)},
	{_T("VLROPIdx"),		FIO_INT,	MF_OFS(VideoList.ROPIdx)},
	{_T("VLAutoRewind"),	FIO_BYTE,	MF_OFS(VideoList.AutoRewind)},
	{_T("VLCycle"),			FIO_BYTE,	MF_OFS(VideoList.Cycle)},
	{_T("VLCycleLen"),		FIO_INT,	MF_OFS(VideoList.CycleLen)},
	{NULL}	// terminator, don't delete
};

CPlaylist::CPlaylist()
{
	SetDefaults();
}

CPlaylist::CPlaylist(const CPlaylist& List)
{
	Copy(List);
}

CPlaylist& CPlaylist::operator=(const CPlaylist& List)
{
	Copy(List);
	return(*this);
}

void CPlaylist::SetDefaults()
{
	m_Patch.RemoveAll();
#ifndef WHORLDFF
	CMainFrame	*Frm = CMainFrame::GetThis();
	ASSERT(Frm != NULL);
	Frm->GetDefaults(m_Main);
#endif
}

void CPlaylist::Copy(const CPlaylist& List)
{
	m_Patch.Copy(List.m_Patch);
	m_Main = List.m_Main;
}

void CPlaylist::ReadParmRow(LPCTSTR Line, CParmInfo& ParmInfo)
{
	TCHAR	RowName[32];
	CParmInfo::ROW	Row;
	int	retc = _stscanf(Line, PARM_ROW_R_FMT, RowName, 
		&Row.Val, &Row.Wave, &Row.Amp, &Row.Freq, &Row.PW);
	if (retc >= 6) {
		for (int i = 0; i < CParmInfo::ROWS; i++) {
			if (!_tcscmp(CParmInfo::m_RowData[i].Name, RowName)) {
				ParmInfo.m_Row[i] = Row;
				break;
			}
		}
	}
}

void CPlaylist::WriteParmRows(CStdioFile& fp, LPCTSTR Tag, const CParmInfo& Defaults, const CParmInfo& ParmInfo) const
{
	CString	s;
	for (int i = 0; i < CParmInfo::ROWS; i++) {
		const CParmInfo::ROW	*rp = &ParmInfo.m_Row[i];
		if (memcmp(rp, &Defaults.m_Row[i], sizeof(CParmInfo::ROW))) {	// skip rows with default values
			s.Format(PARM_ROW_W_FMT, Tag, CParmInfo::m_RowData[i].Name,
				rp->Val, rp->Wave, rp->Amp, rp->Freq, rp->PW);
			fp.WriteString(s);
		}
	}
}

bool CPlaylist::Read(CStdioFile& fp)
{
#ifndef WHORLDFF
	CMainFrame	*Frm = CMainFrame::GetThis();
	ASSERT(Frm != NULL);
	CMidiSetupDlg&	MSDlg = Frm->GetMidiSetup();
#endif
	CString	s;
	int	Version;
	if (!fp.ReadString(s) || _stscanf(s, FILE_ID, &Version) != 1)
		return(FALSE);
	SetDefaults();	// in case some lines are missing
	if (Version == 1)
		m_Main.PatchMode = CMainFrame::PM_GEOMETRY;	// for backwards compatibility
	while (fp.ReadString(s)) {
		s.TrimLeft();
		CString	Name = s.SpanExcluding(_T(" \t"));
		CString	Arg = s.Mid(Name.GetLength());
		if (Name == PATCH_TAG) {
			CPatchLink	Patch;
			Patch.m_Path.GetBuffer(MAX_PATH);
			int	retc = _stscanf(Arg, PATCH_R_FMT, 
				&Patch.m_Bank, &Patch.m_HotKey, Patch.m_Path);
			Patch.m_Path.ReleaseBuffer();
			if (retc >= 3)
				m_Patch.Add(Patch);
		} else if (Name == MIDI_SETUP_TAG) {
#ifndef WHORLDFF
			TCHAR	RowName[32];
			CMidiSetupRow::INFO	Info;
			int	retc = _stscanf(Arg, MIDI_SETUP_R_FMT, RowName, 
				&Info.Range.End, &Info.Event, &Info.Chan, &Info.Ctrl, 
				&Info.Range.Start);
			if (retc >= 5) {
				for (int i = 0; i < CMidiSetupDlg::ROWS; i++) {
					if (!_tcscmp(MSDlg.GetRowName(i), RowName)) {
						if (retc < 6)
							CMidiSetupDlg::FixMidiRange(i, Info);
						m_Main.MidiSetup.Row[i] = Info;
						break;
					}
				}
			}
#endif
		} else if (Name == MASTER_PARM_TAG) {
			ReadParmRow(Arg, m_Main.Patch);
		} else if (Name == GLOB_PARM_TAG) {
			ReadParmRow(Arg, m_Main.GlobParm);
		} else {
			for (int i = 0; m_LineInfo[i].Name != NULL; i++) {
				if (Name == m_LineInfo[i].Name) {
					CFormatIO::StrToVal(m_LineInfo[i].Type, Arg,
						((char *)&m_Main) + m_LineInfo[i].Offset);
					break;
				}
			}
		}
	}
	return(TRUE);
}

bool CPlaylist::Write(CStdioFile& fp) const
{
#ifndef WHORLDFF
	CMainFrame	*Frm = CMainFrame::GetThis();
	ASSERT(Frm != NULL);
	CMidiSetupDlg&	MSDlg = Frm->GetMidiSetup();
#endif
	CString	s;
	s.Format(FILE_ID, FILE_VERSION);
	fp.WriteString(s + "\n");
	int	i;
	for (i = 0; i < m_Patch.GetSize(); i++) {	// write patches
		s.Format(PATCH_W_FMT, PATCH_TAG,
			m_Patch[i].m_Bank, m_Patch[i].m_HotKey, m_Patch[i].m_Path);
		fp.WriteString(s);
	}
	for (i = 0; m_LineInfo[i].Name != NULL; i++) {	// write misc values
		CFormatIO::ValToStr(m_LineInfo[i].Type, 
			((char *)&m_Main) + m_LineInfo[i].Offset, s);
		fp.WriteString(CString(m_LineInfo[i].Name) + "\t" + s + "\n");
	}
#ifndef WHORLDFF
	for (i = 0; i < CMidiSetupDlg::ROWS; i++) {	// write MIDI setup rows
		if (!MSDlg.IsDefault(i)) {	// skip rows with default values
			const CMidiSetupRow::INFO	*ip = &m_Main.MidiSetup.Row[i];
			s.Format(MIDI_SETUP_W_FMT, MIDI_SETUP_TAG,
				MSDlg.GetRowName(i), ip->Range.End, ip->Event, ip->Chan, ip->Ctrl, 
				ip->Range.Start);
			fp.WriteString(s);
		}
	}
#endif
	static const CParmInfo	m_MastParmDef;
	WriteParmRows(fp, MASTER_PARM_TAG, m_MastParmDef, m_Main.Patch);
	WriteParmRows(fp, GLOB_PARM_TAG, m_MastParmDef, m_Main.GlobParm);
	return(TRUE);
}

bool CPlaylist::Read(LPCTSTR Path)
{
	CStdioFile	fp;
	CFileException	e;
	CString	Errs;
	if (!fp.Open(Path, CFile::modeRead | CFile::shareDenyWrite, &e)) {
		e.ReportError();
		return(FALSE);
	}
	TRY {
		if (!Read(fp)) {
			AfxMessageBox(IDS_PL_CANT_READ);
			return(FALSE);
		}
	}
	CATCH(CFileException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}

bool CPlaylist::Write(LPCTSTR Path) const
{
	CStdioFile	fp;
	CFileException	e;
	if (!fp.Open(Path, CFile::modeCreate | CFile::modeWrite, &e)) {
		e.ReportError();
		return(FALSE);
	}
	TRY {
		if (!Write(fp)) {
			AfxMessageBox(IDS_PL_CANT_WRITE);
			return(FALSE);
		}
	}
	CATCH(CFileException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}
