// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22feb25	initial version
		01		09mar25	add flags bitmask to drawing state
		02		10mar25	add target size to drawing state

*/

#pragma once

#include "WhorldBase.h"

// This class has a variable size due to the rings array.
// Allocate instances of this class via its Alloc method.

class CSnapshot : public CWhorldBase {
public:
// Constants
	enum {	// flags
		SF_V1	= 0x01,			// true if V1 legacy snapshot
	};
	static const UINT m_nFileID;
	static const USHORT m_nFileVersion;

// Public data
	DRAW_STATE	m_drawState;	// drawing state
	GLOB_RING	m_globRing;		// global ring offsets
	RING		m_aRing[1];		// variable-length array of rings

// Attributes
	int		GetSize() const;

// Operations
	static CSnapshot*	Alloc(int nRings);
	static void	Write(const CSnapshot* pSnapshot, LPCTSTR pszPath);
	static CSnapshot*	Read(LPCTSTR pszPath);
	static void	Read(CFile& file, void* lpBuf, UINT nCount);
	static UINT	GetSize(int nRings);
	template<class T> 
	static void FormatItem(LPCTSTR pszCaption, const T& val, CString& sLine)
	{
		sLine += CString(pszCaption) + _T(": ") + ValToString(val) + '\n';
	}
	static CString	FormatState(const DRAW_STATE& drawState);
	static CString	FormatRing(int iRing, const RING& ring);
	static CString	FormatGlobRing(const GLOB_RING& globRing);
	void	DumpToFile(LPCTSTR pszPath) const;

protected:
// Types
	struct HEADER {
		UINT	nFileID;		// file identifier
		USHORT	nVersion;		// version number
		USHORT	nDrawStateSize;	// size of DRAW_STATE in bytes
		USHORT	nGlobRingSize;	// size of GLOB_RING in bytes
		USHORT	nRingSize;		// size of RING in bytes
	};
};

inline int CSnapshot::GetSize() const
{
	return GetSize(m_drawState.nRings);
}
