// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27jul05	initial version
		01		17feb06	GetRowName isn't static anymore
		02		19feb06	make write functions const
        03      17apr06	rename link to CPatchLink
        04      18apr06	bump file version to 2
		05		21jun06	add tag to MasterDef macro
		06		23jan08	replace MIDI range scaler with start/end
		07		28jan08	support Unicode
		08		28feb25	refactor for V2
		09		27mar25	add patch array

		playlist container
 
*/

#pragma once

#include "AuxiliaryDoc.h"

class CIniFile;

class CPlaylist : public CAuxiliaryDoc {
public:
// Construction
	CPlaylist();

// Constants
	static const LPCTSTR m_pszPlaylistExt;
	static const LPCTSTR m_pszPlaylistFilter;

// Types
	class CPatchLink {
	public:
		CString	m_sPath;
	};
	class CPatchLinkArray : public CArrayEx<CPatchLink, CPatchLink&> {
	public:
		void	GetPaths(CStringArrayEx& arrPath) const;
		void	SetPaths(const CStringArrayEx& arrPath);
	};

// Public data
	CPatchLinkArray	m_arrPatch;	// array of patch links

// Attributes

// Operations

// Overrides
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);

protected:
// Data members
	int		m_nFileVersion;		// file version number obtained from document

// Helpers
	bool	ValidateFileType(CIniFile& fIn, LPCTSTR lpszPathName);
	void	ReadPatches(CIniFile& fIn);
	void	WritePatches(CIniFile& fOut);
	void	CheckPatchPaths();
};
