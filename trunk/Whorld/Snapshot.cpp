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
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif //defined(MAKEFOURCC)

const UINT CSnapshot::m_nFileID = MAKEFOURCC('W', 'H', 'S', '2');
const USHORT CSnapshot::m_nFileVersion = 1;

void CSnapshot::Write(LPCTSTR pszPath)
{
	// set m_aRing, m_globRing, and draw state before calling this method
	CFile	fOut(pszPath, CFile::modeCreate | CFile::modeWrite);	// may throw
	m_hdr.nFileID = m_nFileID;
	m_hdr.nVersion = m_nFileVersion;
	m_hdr.nDataSize = sizeof(SNAP_DATA);
	fOut.Write(&m_hdr, sizeof(SNAP_HEADER));
	m_data.nRingSize = sizeof(RING);
	m_data.nGlobRingSize = sizeof(GLOBRING);
	m_data.nRings = m_aRing.GetSize();
	ZeroMemory(m_data.drawState.baReserved, sizeof(m_data.drawState.baReserved));
	fOut.Write(&m_data, sizeof(SNAP_DATA));
	fOut.Write(m_aRing.GetData(), m_data.nRings * sizeof(RING));
	fOut.Write(&m_globRing, sizeof(GLOBRING));
}

void CSnapshot::Read(LPCTSTR pszPath)
{
	CFile	fIn(pszPath, CFile::modeRead | CFile::shareDenyWrite);	// may throw
	Read(fIn, &m_hdr, sizeof(SNAP_HEADER));
	if (m_hdr.nFileID != m_nFileID) {
		AfxThrowArchiveException(CArchiveException::badIndex, fIn.GetFilePath());
	}
	ZeroMemory(&m_data, sizeof(SNAP_DATA));
	Read(fIn, &m_data, min(m_hdr.nDataSize, sizeof(SNAP_DATA)));
	m_aRing.SetSize(m_data.nRings);
	if (m_data.nRingSize == sizeof(RING)) {	// if ring is current size
		Read(fIn, m_aRing.GetData(), m_data.nRings * m_data.nRingSize);
	} else {	// legacy ring size; must read rings one at a time
		int	nRingSize = min(m_data.nRingSize, sizeof(RING));
		for (int iRing = 0; iRing < m_data.nRings; iRing++) {
			Read(fIn, &m_aRing[iRing], nRingSize);
		}
	}
	ZeroMemory(&m_globRing, sizeof(GLOBRING));
	Read(fIn, &m_globRing, min(m_data.nGlobRingSize, sizeof(GLOBRING)));
}

void CSnapshot::Read(CFile& file, void* lpBuf, UINT nCount)
{
	// CFile handles system error, but not end of file
	UINT	nBytesRead = file.Read(lpBuf, nCount);
	if (nBytesRead != nCount) {	// if bytes read differs from bytes requested
		AfxThrowFileException(CFileException::endOfFile, -1, file.GetFilePath());
	}
}
