// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22feb25	initial version

*/

#pragma once

#include "WhorldBase.h"

class CSnapshot : public CWhorldBase {
public:
// Types
	struct SNAP_HEADER {
		UINT	nFileID;		// file identifier
		USHORT	nVersion;		// version number
		USHORT	nDataSize;		// size of data struct
	};
	struct DRAW_STATE {
		D2D1_COLOR_F	clrBkgnd;	// background color
		double	fZoom;			// current zoom, as a scaling factor
		bool	bConvex;		// true if drawing in descending size order
		BYTE	baReserved[7];	// reserved, must be zero	
	};
	struct SNAP_DATA {
		USHORT	nRingSize;		// size of RING in bytes
		USHORT	nGlobRingSize;	// size of GLOBRING in bytes
		int		nRings;			// number of elements in ring array
		DRAW_STATE	drawState;	// draw state; must be set by user
	};
	typedef CArrayEx<RING, RING&> CRingArray;

// Constants
	static const UINT m_nFileID;
	static const USHORT m_nFileVersion;

// Public data
	SNAP_HEADER	m_hdr;		// file header
	SNAP_DATA	m_data;		// various data
	CRingArray	m_aRing;	// array of rings
	GLOBRING	m_globRing;	// global ring offsets

// Operations
	void	Write(LPCTSTR pszPath);
	void	Read(LPCTSTR pszPath);

// Helpers
	static void	Read(CFile& file, void* lpBuf, UINT nCount);
};
