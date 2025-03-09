// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22feb25	initial version
		01		09mar25	add flags bitmask to state

*/

#pragma once

#include "WhorldBase.h"

// This class has a variable size due to the rings array.
// Allocate instances of this class via its Alloc method.

class CSnapshot : public CWhorldBase {
public:
// Types
	struct STATE {
		D2D1_COLOR_F	clrBkgnd;	// background color
		double	fZoom;			// current zoom, as a scaling factor
		int		nRings;			// number of elements in ring array
		bool	bConvex;		// true if drawing in descending size order
		BYTE	bReserved;		// reserved, must be zero	
		USHORT	nFlags;			// see enum below
	};

// Constants
	enum {	// flags
		SF_V1	= 0x0001,		// true if V1 legacy snapshot
	};
	static const UINT m_nFileID;
	static const USHORT m_nFileVersion;

// Public data
	STATE		m_state;		// drawing state
	GLOBRING	m_globRing;		// global ring offsets
	RING		m_aRing[1];		// variable-length array of rings

// Attributes
	int		GetSize() const;

// Operations
	static CSnapshot*	Alloc(int nRings);
	static void	Write(const CSnapshot* pSnapshot, LPCTSTR pszPath);
	static CSnapshot*	Read(LPCTSTR pszPath);
	static void	Read(CFile& file, void* lpBuf, UINT nCount);
	static UINT	GetSize(int nRings);

protected:
// Types
	struct HEADER {
		UINT	nFileID;		// file identifier
		USHORT	nVersion;		// version number
		USHORT	nStateSize;		// size of STATE
		USHORT	nGlobRingSize;	// size of GLOBRING in bytes
		USHORT	nRingSize;		// size of RING in bytes
	};
};

inline int CSnapshot::GetSize() const
{
	return GetSize(m_state.nRings);
}
