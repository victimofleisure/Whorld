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

		playlist container
 
*/

#include "stdafx.h"
#include "Whorld.h"
#include "Playlist.h"
#include "IniFile.h"
#include "MainFrm.h"

const LPCTSTR CPlaylist::m_pszPlaylistExt = _T("whl");
const LPCTSTR CPlaylist::m_pszPlaylistFilter = _T("Playlist Files (*.whl)|*.whl|All Files (*.*)|*.*||");

// file versioning
#define FILE_ID				_T("WhorldPlay")
#define	FILE_VERSION		1

// file format keys
#define RK_FILE_ID			_T("FileID")
#define RK_FILE_VERSION		_T("FileVersion")

CPlaylist::CPlaylist() : CAuxiliaryDoc(IDR_PLAYLIST, 0, _T("Recent Playlist"), _T("Playlist%d"), theApp.m_options.m_General_nMRUItems)
{
	m_bModified = false;
}

BOOL CPlaylist::OnNewDocument()
{
	if (!CAuxiliaryDoc::OnNewDocument())
		return false;
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
	theApp.m_midiMgr.ReadMappings(fIn);
	CMappingBar&	wndMappingBar = theApp.GetMainFrame()->m_wndMappingBar;
	wndMappingBar.OnUpdate(NULL);
	return true;
}

BOOL CPlaylist::OnSaveDocument(LPCTSTR lpszPathName)
{
	if (!CAuxiliaryDoc::OnSaveDocument(lpszPathName))
		return false;
	CIniFile	fOut(lpszPathName, true);
	fOut.Put(RK_FILE_ID, CString(FILE_ID));
	fOut.Put(RK_FILE_VERSION, FILE_VERSION);
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
