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

#include "stdafx.h"
#include "Snapshot.h"

#ifndef MAKEFOURCC
	#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
		((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | \
		((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif //defined(MAKEFOURCC)

const UINT CSnapshot::m_nFileID = MAKEFOURCC('W', 'H', 'S', '2');
const USHORT CSnapshot::m_nFileVersion = 1;

UINT CSnapshot::GetSize(int nRings)
{
	return sizeof(STATE) + sizeof(GLOBRING) + nRings * sizeof(RING);
}

CSnapshot* CSnapshot::Alloc(int nRings)
{
	UINT	nSnapSize = GetSize(nRings);
	BYTE	*pData = new BYTE[nSnapSize];
	return reinterpret_cast<CSnapshot*>(pData);
}

void CSnapshot::Write(const CSnapshot* pSnapshot, LPCTSTR pszPath)
{
	// set all members of pSnapshot before calling this method
	CFile	fOut(pszPath, CFile::modeCreate | CFile::modeWrite);	// may throw
	HEADER	hdr;
	hdr.nFileID = m_nFileID;
	hdr.nVersion = m_nFileVersion;
	hdr.nStateSize = sizeof(STATE);
	hdr.nGlobRingSize = sizeof(GLOBRING);
	hdr.nRingSize = sizeof(RING);
	fOut.Write(&hdr, sizeof(HEADER));
	UINT	nSnapSize = GetSize(pSnapshot->m_state.nRings);
	fOut.Write(pSnapshot, nSnapSize);
}

CSnapshot* CSnapshot::Read(LPCTSTR pszPath)
{
	CFile	fIn(pszPath, CFile::modeRead | CFile::shareDenyWrite);	// may throw
	HEADER	hdr;
	Read(fIn, &hdr, sizeof(HEADER));
	if (hdr.nFileID != m_nFileID) {	// if invalid file ID
		// throw invalid format exception
		AfxThrowArchiveException(CArchiveException::badIndex, fIn.GetFilePath());
	}
	STATE	state;
	ZeroMemory(&state, sizeof(STATE));
	Read(fIn, &state, min(hdr.nStateSize, sizeof(STATE)));
	CSnapshot*	pSnapshot = Alloc(state.nRings);
	if (pSnapshot == NULL)	// if allocation failed
		return NULL;	// can't proceed
	pSnapshot->m_state = state;
	ZeroMemory(&pSnapshot->m_globRing, sizeof(GLOBRING));
	Read(fIn, &pSnapshot->m_globRing, min(hdr.nGlobRingSize, sizeof(GLOBRING)));
	if (hdr.nRingSize == sizeof(RING)) {	// if ring is current size
		Read(fIn, pSnapshot->m_aRing, sizeof(RING) * state.nRings);
	} else {	// legacy ring size; must read rings one at a time
		int	nRingSize = min(hdr.nRingSize, sizeof(RING));
		for (int iRing = 0; iRing < state.nRings; iRing++) {
			Read(fIn, &pSnapshot->m_aRing[iRing], nRingSize);
		}
	}
	return pSnapshot;
}

void CSnapshot::Read(CFile& file, void* lpBuf, UINT nCount)
{
	// CFile handles system errors, but not end of file
	UINT	nBytesRead = file.Read(lpBuf, nCount);
	if (nBytesRead != nCount) {	// if bytes read differs from bytes requested
		AfxThrowFileException(CFileException::endOfFile, -1, file.GetFilePath());
	}
}
