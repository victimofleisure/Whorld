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
#include "SnapshotV1.h"
#include "Patch.h"	// for string conversion operators

const UINT CSnapshot::m_nFileID = MakeFourCC('W', 'H', 'S', '2');
const USHORT CSnapshot::m_nFileVersion = 1;

UINT CSnapshot::GetSize(int nRings)
{
	return sizeof(DRAW_STATE) + sizeof(GLOB_RING) + nRings * sizeof(RING);
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
	hdr.nDrawStateSize = sizeof(DRAW_STATE);
	hdr.nGlobRingSize = sizeof(GLOB_RING);
	hdr.nRingSize = sizeof(RING);
	fOut.Write(&hdr, sizeof(HEADER));
	UINT	nSnapSize = GetSize(pSnapshot->m_drawState.nRings);
	fOut.Write(pSnapshot, nSnapSize);
}

CSnapshot* CSnapshot::Read(LPCTSTR pszPath)
{
	CFile	fIn(pszPath, CFile::modeRead);	// may throw
	HEADER	hdr;
	Read(fIn, &hdr, sizeof(HEADER));
	if (hdr.nFileID != m_nFileID) {	// if invalid file ID
		return CSnapshotV1::Read(fIn);
	}
	DRAW_STATE	drawState;
	ZeroMemory(&drawState, sizeof(DRAW_STATE));
	Read(fIn, &drawState, min(hdr.nDrawStateSize, sizeof(DRAW_STATE)));
	CAutoPtr<CSnapshot>	pSnapshot(Alloc(drawState.nRings));
	if (pSnapshot == NULL)	// if allocation failed
		return NULL;	// can't proceed
	pSnapshot->m_drawState = drawState;
	ZeroMemory(&pSnapshot->m_globRing, sizeof(GLOB_RING));
	Read(fIn, &pSnapshot->m_globRing, min(hdr.nGlobRingSize, sizeof(GLOB_RING)));
	if (hdr.nRingSize == sizeof(RING)) {	// if ring is current size
		Read(fIn, pSnapshot->m_aRing, sizeof(RING) * drawState.nRings);
	} else {	// legacy ring size; must read rings one at a time
		int	nRingSize = min(hdr.nRingSize, sizeof(RING));
		for (int iRing = 0; iRing < drawState.nRings; iRing++) {
			Read(fIn, &pSnapshot->m_aRing[iRing], nRingSize);
		}
	}
	return pSnapshot.Detach();
}

void CSnapshot::Read(CFile& file, void* lpBuf, UINT nCount)
{
	// CFile handles system errors, but not end of file
	UINT	nBytesRead = file.Read(lpBuf, nCount);
	if (nBytesRead != nCount) {	// if bytes read differs from bytes requested
		AfxThrowFileException(CFileException::endOfFile, -1, file.GetFilePath());
	}
}

CString CSnapshot::FormatState(const DRAW_STATE& drawState)
{
	CString sLine;
	#define STATEDEF(type, name) \
		FormatItem(_T(#name), drawState.name, sLine);
	#include "WhorldDef.h"
	return sLine;
}

CString CSnapshot::FormatRing(int iRing, const RING& ring)
{
	CString	sIdx;
	sIdx.Format(_T("%d"), iRing);
	CString	sLine('[' + sIdx + _T("]\n"));
	#define RINGDEF(type, name) \
		FormatItem(_T(#name), ring.name, sLine);
	#include "WhorldDef.h"
	return sLine;
}

CString CSnapshot::FormatGlobRing(const GLOB_RING& globRing)
{
	CString	sLine(_T("[GlobRing]\n"));
	#define GLOBRINGDEF(type, name) \
		FormatItem(_T(#name), globRing.name, sLine);
	#include "WhorldDef.h"
	return sLine;
}

void CSnapshot::DumpToFile(LPCTSTR pszPath) const
{
	CStdioFile	fOut(pszPath, CFile::modeWrite | CFile::modeCreate);
	fOut.WriteString(FormatState(m_drawState));
	for (int iRing = 0; iRing < m_drawState.nRings; iRing++) {	// for each ring
		fOut.WriteString(FormatRing(iRing, m_aRing[iRing]));
	}
	fOut.WriteString(FormatGlobRing(m_globRing));
}
