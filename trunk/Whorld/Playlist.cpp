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
		26		28feb25	refactor for V2
		27		27mar25	add patch array

		playlist container
 
*/

#include "stdafx.h"
#include "Whorld.h"
#include "Playlist.h"
#include "IniFile.h"
#include "MainFrm.h"
#include "MissingFilesDlg.h"

const LPCTSTR CPlaylist::m_pszPlaylistExt = _T("whl");
const LPCTSTR CPlaylist::m_pszPlaylistFilter = _T("Playlist Files (*.whl)|*.whl|All Files (*.*)|*.*||");

// file versioning
#define FILE_ID				_T("WhorldPlay")
#define	FILE_VERSION		1

// file format keys
#define RK_FILE_ID			_T("FileID")
#define RK_FILE_VERSION		_T("FileVersion")

// patch list keys
#define RK_PATCH_COUNT		_T("Count")
#define RK_PATCH_SECTION	_T("Patch")
#define RK_PATCH_PATH		_T("Path")

CPlaylist::CPlaylist() : CAuxiliaryDoc(IDR_PLAYLIST, 0, _T("Recent Playlist"), _T("Playlist%d"), theApp.m_options.m_General_nMRUItems)
{
	m_bModified = false;
}

void CPlaylist::CPatchLinkArray::GetPaths(CStringArrayEx& arrPath) const
{
	int	nPatches = GetSize();
	arrPath.SetSize(nPatches);
	for (int iPatch = 0; iPatch < nPatches; iPatch++) {	// for each patch
		arrPath[iPatch] = GetAt(iPatch).m_sPath;	// get patch's path
	}
}

void CPlaylist::CPatchLinkArray::SetPaths(const CStringArrayEx& arrPath)
{
	int	nPatches = arrPath.GetSize();
	SetSize(nPatches);
	for (int iPatch = 0; iPatch < nPatches; iPatch++) {	// for each patch
		GetAt(iPatch).m_sPath = arrPath[iPatch];	// set patch's path
	}
}

BOOL CPlaylist::OnNewDocument()
{
	if (!CAuxiliaryDoc::OnNewDocument())
		return false;
	theApp.m_pPlaylist->m_arrPatch.RemoveAll();
	theApp.GetMainFrame()->m_wndPlaylistBar.OnUpdate(NULL);
	theApp.m_midiMgr.RemoveAllMappings();
	CMappingBar&	wndMappingBar = theApp.GetMainFrame()->m_wndMappingBar;
	wndMappingBar.GetUndoManager()->DiscardAllEdits();
	wndMappingBar.OnUpdate(NULL);
	return TRUE;
}

BOOL CPlaylist::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CAuxiliaryDoc::OnOpenDocument(lpszPathName))
		return false;
	CIniFile	fIn(lpszPathName, false);
	fIn.Read();
	if (!ValidateFileType(fIn, lpszPathName))
		return false;
	ReadPatches(fIn);
	CheckPatchPaths();
	theApp.GetMainFrame()->m_wndPlaylistBar.OnUpdate(NULL);
	theApp.m_midiMgr.ReadMappings(fIn);
	theApp.GetMainFrame()->m_wndMappingBar.OnUpdate(NULL);
	return true;
}

BOOL CPlaylist::OnSaveDocument(LPCTSTR lpszPathName)
{
	if (!CAuxiliaryDoc::OnSaveDocument(lpszPathName))
		return false;
	CIniFile	fOut(lpszPathName, true);
	fOut.Put(RK_FILE_ID, CString(FILE_ID));
	fOut.Put(RK_FILE_VERSION, FILE_VERSION);
	WritePatches(fOut);
	theApp.m_midiMgr.WriteMappings(fOut);
	fOut.Write();
	return true;
}

bool CPlaylist::ValidateFileType(CIniFile& fIn, LPCTSTR lpszPathName)
{
	CString	sFileID;
	fIn.Get(RK_FILE_ID, sFileID);
	if (sFileID != FILE_ID) {	// if unexpected file ID
		CString	msg;
		AfxFormatString1(msg, IDS_DOC_BAD_FORMAT, lpszPathName);
		AfxMessageBox(msg, MB_OK, IDS_DOC_BAD_FORMAT);
		return false;
	}
	fIn.Get(RK_FILE_VERSION, m_nFileVersion);
	if (m_nFileVersion > FILE_VERSION) {	// if file is from a newer version
		CString	msg;
		AfxFormatString1(msg, IDS_DOC_NEWER_VERSION, lpszPathName);
		AfxMessageBox(msg, MB_OK, IDS_DOC_NEWER_VERSION);
	}
	return true;
}

void CPlaylist::ReadPatches(CIniFile& fIn)
{
	CString	sSectionIdx;
	int	nPatches = fIn.GetInt(RK_PATCH_SECTION, RK_PATCH_COUNT, 0);
	m_arrPatch.SetSize(nPatches);
	for (int iPatch = 0; iPatch < nPatches; iPatch++) {	// for each patch
		sSectionIdx.Format(_T("%d"), iPatch);
		fIn.GetUnicodeString(RK_PATCH_SECTION _T("\\") + sSectionIdx, RK_PATCH_PATH, m_arrPatch[iPatch].m_sPath);
	}
}

void CPlaylist::WritePatches(CIniFile& fOut)
{
	CString	sSectionIdx;
	int	nPatches = m_arrPatch.GetSize();
	fOut.WriteInt(RK_PATCH_SECTION, RK_PATCH_COUNT, nPatches);
	for (int iPatch = 0; iPatch < nPatches; iPatch++) {	// for each patch
		sSectionIdx.Format(_T("%d"), iPatch);
		fOut.WriteUnicodeString(RK_PATCH_SECTION _T("\\") + sSectionIdx, RK_PATCH_PATH, m_arrPatch[iPatch].m_sPath);
	}
}

void CPlaylist::CheckPatchPaths()
{
	BENCH_START
	CStringArrayEx	arrPath;
	m_arrPatch.GetPaths(arrPath);
	CStringArrayEx	arrOldPath(arrPath);	// for change detection
	CMissingFilesDlg	dlgMissing(arrPath, CString('.') + CMainFrame::m_pszPatchExt, 
		CMainFrame::m_pszPatchFilter, AfxGetMainWnd());
	dlgMissing.Check();
	if (arrPath != arrOldPath) {	// if any paths changed
		SetModifiedFlag();	// replacing missing files counts as modification
	}
	m_arrPatch.SetPaths(arrPath);
	BENCH_STOP
}
