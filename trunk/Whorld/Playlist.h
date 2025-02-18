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

		playlist container
 
*/

#ifndef CPLAYLIST_INCLUDED
#define CPLAYLIST_INCLUDED

#include "PatchLink.h"
#include "MainFrm.h"

class CPlaylist : public WObject {
public:
// Construction
	CPlaylist();
	CPlaylist(const CPlaylist& List);
	CPlaylist& operator=(const CPlaylist& List);

// Types
	typedef CArray<CPatchLink, CPatchLink&> PATCH_LIST;
	typedef CPatch::LINE_INFO LINE_INFO;

// Public data
	PATCH_LIST	m_Patch;
	CMainFrame::INFO	m_Main;

// Operations
	void	SetDefaults();
	bool	Read(CStdioFile& fp);
	bool	Read(LPCTSTR Path);
	bool	Write(CStdioFile& fp) const;
	bool	Write(LPCTSTR Path) const;

private:
// Constants
	enum {
		FILE_VERSION = 2
	};
	static const LPCTSTR FILE_ID;
	static const LPCTSTR PATCH_TAG;
	static const LPCTSTR MIDI_SETUP_TAG;
	static const LPCTSTR MASTER_PARM_TAG;
	static const LPCTSTR GLOB_PARM_TAG;
	static const LINE_INFO	m_LineInfo[];

// Helpers
	void	Copy(const CPlaylist& List);
	void	ReadParmRow(LPCTSTR Line, CParmInfo& ParmInfo);
	void	WriteParmRows(CStdioFile& fp, LPCTSTR Tag, const CParmInfo& Defaults, const CParmInfo& ParmInfo) const;
};

#endif
