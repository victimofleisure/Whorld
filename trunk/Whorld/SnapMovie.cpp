// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14mar25	initial version

*/

#include "stdafx.h"
#include "SnapMovie.h"
#include "Snapshot.h"

const UINT CSnapMovie::m_nFileID = CSnapshot::MakeFourCC('W', 'H', 'M', '2');
const USHORT CSnapMovie::m_nFileVersion = 1;

CSnapMovie::CSnapMovie()
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_nWriteFrames = 0;
	m_nWriteBytes = 0;
	ZeroMemory(&m_hdr, sizeof(m_hdr));
	m_nState = MIO_CLOSED;
}

CSnapMovie::~CSnapMovie()
{
	Close();
}

bool CSnapMovie::Open(LPCTSTR pszPath, bool bWrite)
{
	Close();
	DWORD	dwAttributes = 0;
	DWORD	dwDesiredAccess;
	DWORD	dwCreationDisposition;
	if (bWrite) {
		// frame rate and target size must be specified before opening
		if (!(m_hdr.fFrameRate > 0 && m_hdr.szTarget.width > 0 && m_hdr.szTarget.height > 0)) {
			return false;
		}
		dwDesiredAccess = GENERIC_WRITE;
		dwCreationDisposition = CREATE_ALWAYS;
	} else {
		dwDesiredAccess = GENERIC_READ | FILE_SHARE_READ;
		dwCreationDisposition = OPEN_EXISTING;
	}
	m_hFile = CreateFile(pszPath, dwDesiredAccess, 
		0, NULL, dwCreationDisposition, dwAttributes, 0);
	if (!IsOpen()) {
		return false;
	}
	m_aWriteFrameIndex.RemoveAll();
	m_aReadFrameIndex.RemoveAll();
	if (bWrite) {
		if (!WriteHeader()) {
			return false;
		}
	} else {
		if (!ReadHeader()) {
			return false;
		}
		if (!ReadFrameIndex()) {
			return false;
		}
	}
	m_nState = bWrite;
	return true;
}

bool CSnapMovie::Close()
{
	if (!IsOpen()) {	// if file isn't open
		return false;	// nothing to do
	}
	bool	bRetVal = true;
	if (IsWriting()) {
		bRetVal = FinalizeWrite();
	}
	CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
	m_nState = MIO_CLOSED;
	return bRetVal;
}

bool CSnapMovie::WriteHeader()
{
	HEADER	hdr;
	ZeroMemory(&hdr, sizeof(hdr));
	hdr.nFileID = m_nFileID;
	hdr.nVersion = m_nFileVersion;
	hdr.nDrawStateSize = sizeof(DRAW_STATE);
	hdr.nGlobRingSize = sizeof(GLOB_RING);
	hdr.nRingSize = sizeof(RING);
//@@@	hdr.fFrameRate
//@@@	hdr.szTarget
	hdr.nFrameCount = m_nWriteFrames;
	hdr.nIndexOffset = m_nWriteBytes + sizeof(HEADER);
	return WriteBuf(&hdr, sizeof(hdr));
}

bool CSnapMovie::ReadHeader()
{
	ZeroMemory(&m_hdr, sizeof(m_hdr));
	if (!ReadBuf(&m_hdr, sizeof(m_hdr))) {
		return false;
	}
	if (m_hdr.nFileID != m_nFileID) {
		return false;
	}
	return true;
}

bool CSnapMovie::WriteFrameIndex()
{
	ASSERT(m_aWriteFrameIndex.GetSize() * sizeof(FRAME_INDEX_ENTRY) < UINT_MAX);
	INT_PTR	nBlocks = m_aWriteFrameIndex.GetBlockCount();
	for (int iBlock = 0; iBlock < nBlocks; iBlock++) {
		const FRAME_INDEX_ENTRY* pEntry = m_aWriteFrameIndex.GetBlock(iBlock);
		DWORD	nSize = static_cast<DWORD>(m_aWriteFrameIndex.GetBlockSize(iBlock));
		if (!WriteBuf(pEntry, nSize * sizeof(FRAME_INDEX_ENTRY))) {
			return false;
		}
	}
	return true;
}

bool CSnapMovie::ReadFrameIndex()
{
	ASSERT(m_hdr.nFileID == m_nFileID);	// sanity check
	LARGE_INTEGER	nFilePos;
	nFilePos.QuadPart = m_hdr.nIndexOffset;
	if (!SetFilePointerEx(m_hFile, nFilePos, NULL, FILE_BEGIN)) {
		return false;
	}
	m_aReadFrameIndex.SetSize(m_hdr.nFrameCount);
	DWORD	nSize = static_cast<DWORD>(m_aWriteFrameIndex.GetSize() * sizeof(FRAME_INDEX_ENTRY));
	if (!ReadBuf(m_aReadFrameIndex.GetData(), nSize)) {
		return false;
	}
	nFilePos.QuadPart = sizeof(m_hdr);
	if (!SetFilePointerEx(m_hFile, nFilePos, NULL, FILE_BEGIN)) {
		return false;
	}
	return true;
}

bool CSnapMovie::FinalizeWrite()
{
	if (!WriteFrameIndex()) {
		return false;
	}
	LARGE_INTEGER	nFilePos = {0};
	if (!SetFilePointerEx(m_hFile, nFilePos, NULL, FILE_BEGIN)) {
		return false;
	}
	if (!WriteHeader()) {
		return false;
	}
	return true;
}

bool CSnapMovie::Write(const CSnapshot *pSnapshot)
{
	DWORD	nSize = pSnapshot->GetSize();
	DWORD	nBytesWritten;
	BOOL	bResult = WriteFile(m_hFile, pSnapshot, nSize, &nBytesWritten, NULL);
	if (!bResult || nBytesWritten != nSize)
		return false;
	m_aWriteFrameIndex.Add(m_nWriteBytes);
	m_nWriteBytes += nSize;
	m_nWriteFrames++;
	return true;
}

CSnapshot* CSnapMovie::Read()
{
	// same as CSnapshot::Read except file header is handled differently
	// and we use file handle instead of CFile; no exceptions are thrown
	ASSERT(m_hdr.nFileID == m_nFileID);	// sanity check
	DRAW_STATE	drawState;
	ZeroMemory(&drawState, sizeof(DRAW_STATE));
	if (!ReadBuf(&drawState, min(m_hdr.nDrawStateSize, sizeof(DRAW_STATE)))) {
		return NULL;	// failed to read draw state
	}
	CAutoPtr<CSnapshot>	pSnapshot(CSnapshot::Alloc(drawState.nRings));
	if (pSnapshot == NULL) {	// if allocation failed
		return NULL;	// can't proceed
	}
	pSnapshot->m_drawState = drawState;
	ZeroMemory(&pSnapshot->m_globRing, sizeof(GLOB_RING));
	if (!ReadBuf(&pSnapshot->m_globRing, min(m_hdr.nGlobRingSize, sizeof(GLOB_RING)))) {
		return NULL;	// failed to global ring
	}
	if (m_hdr.nRingSize == sizeof(RING)) {	// if ring is current size
		if (!ReadBuf(pSnapshot->m_aRing, sizeof(RING) * drawState.nRings)) {
			return NULL;	// failed to read rings
		}
	} else {	// legacy ring size; must read rings one at a time
		int	nRingSize = min(m_hdr.nRingSize, sizeof(RING));
		for (int iRing = 0; iRing < drawState.nRings; iRing++) {
			if (!ReadBuf(&pSnapshot->m_aRing[iRing], nRingSize)) {
				return NULL;	// failed to read ring
			}
		}
	}
	return pSnapshot.Detach();
}
