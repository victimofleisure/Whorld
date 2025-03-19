// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14mar25	initial version
		01		19mar25	add macro to disable asynchronous writing

*/

#include "stdafx.h"
#include "SnapMovie.h"
#include "Snapshot.h"

const UINT CSnapMovie::m_nFileID = CSnapshot::MakeFourCC('W', 'H', 'M', '2');
const USHORT CSnapMovie::m_nFileVersion = 1;

// Use this version to handle failure of a system function;
// it assumes the function has already called SetLastError.
#define ABORT { m_nLastErrorLine = __LINE__; return false; }

// Use this version to handle application-specific failures;
// it obliges you to specify a suitable Windows error code.
#define ABORT2(errcode) { m_nLastErrorLine = __LINE__; SetLastError(errcode); return false; }

#define ASYNCHRONOUS_WRITING 1

CSnapMovie::CSnapMovie()
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_nWriteFrames = 0;
	m_nWriteBytes = 0;
	ZeroMemory(&m_hdr, sizeof(m_hdr));
	m_iReadFrame = 0;
	m_nIOState = IO_CLOSED;
	m_nLastErrorLine = 0;
}

CSnapMovie::~CSnapMovie()
{
	Close();
}

void CSnapMovie::GetLastErrorState(ERROR_STATE& errLast) const
{
	errLast.nError = GetLastError();
	errLast.nLineNum = m_nLastErrorLine;
	errLast.pszSrcFileName = __FILE__;
	errLast.pszSrcFileDate = __DATE__;
}

bool CSnapMovie::Open(LPCTSTR pszPath, bool bWrite)
{
	Close();	// close file if it's open
	DWORD	dwAttributes;
	DWORD	dwDesiredAccess;
	DWORD	dwCreationDisposition;
	if (bWrite) {	// if opening for write
		// frame rate and target size must be specified before opening
		if (!PreparedForWrite()) {	// if caller hasn't prepared
			ABORT2(ERROR_INVALID_STATE);	// fail
		}
		dwDesiredAccess = GENERIC_WRITE;
		dwCreationDisposition = CREATE_ALWAYS;
#if ASYNCHRONOUS_WRITING
		dwAttributes = FILE_FLAG_OVERLAPPED;
#else
		dwAttributes = 0;
#endif
	} else {	// opening for read
		dwDesiredAccess = GENERIC_READ | FILE_SHARE_READ;
		dwCreationDisposition = OPEN_EXISTING;
		dwAttributes = 0;
	}
	// open file for read or write as specified above
	m_hFile = CreateFile(pszPath, dwDesiredAccess, 
		0, NULL, dwCreationDisposition, dwAttributes, 0);
	if (!IsOpen()) {	// if file not open
		ABORT;	// failed to open file
	}
	m_aWriteFrameIndex.RemoveAll();
	m_aReadFrameIndex.RemoveAll();
	if (bWrite) {	// if opening for write
		// write dummy header; real header will be written by Close
		if (!WriteHeader()) {	// if we can't write header
			return false;	// fail; error already handled
		}
		m_nWriteBytes = sizeof(m_hdr);	// set output position past header
	} else {	// opening for read
		if (!ReadHeader()) {	// if we can't read header
			return false;	// fail; error already handled
		}
		if (!ReadFrameIndex()) {	// if we can't read frame index
			return false;	// fail; error already handled
		}
		m_iReadFrame = 0;	// read starts with first frame
	}
	m_nIOState = bWrite;	// update I/O state
	return true;	// success
}

bool CSnapMovie::Close()
{
	if (!IsOpen()) {	// if file isn't open
		return true;	// nothing to do, but not an error
	}
	bool	bRetVal = true;
	if (IsWriting()) {	// if we're writing
		// write frame index and then rewrite header
		bRetVal = FinalizeWrite();
	}
	CloseHandle(m_hFile);	// close file
	m_hFile = INVALID_HANDLE_VALUE;	// mark it closed
	m_nIOState = IO_CLOSED;	// update I/O state
	return bRetVal;
}

bool CSnapMovie::ReadHeader()
{
	ZeroMemory(&m_hdr, sizeof(m_hdr));
	if (!ReadBuf(&m_hdr, sizeof(m_hdr))) {	// if we can't read header
		ABORT;	// failed to read header
	}
	if (m_hdr.nFileID != m_nFileID) {	// if file identifiers don't match
		ABORT2(ERROR_INVALID_DATA);	// unexpected file format
	}
	return true;	// success
}

bool CSnapMovie::WriteHeader()
{
	// frame rate and target size must be specified beforehand
	if (!PreparedForWrite()) {	// if caller didn't do so
		ABORT2(ERROR_INVALID_STATE);	// fail
	}
	m_hdr.nFileID = m_nFileID;
	m_hdr.nVersion = m_nFileVersion;
	m_hdr.nDrawStateSize = sizeof(DRAW_STATE);
	m_hdr.nGlobRingSize = sizeof(GLOB_RING);
	m_hdr.nRingSize = sizeof(RING);
	m_hdr.nFrameCount = m_nWriteFrames;
	m_hdr.nIndexOffset = m_nWriteBytes;
	if (!WriteWait(&m_hdr, sizeof(m_hdr), 0)) {	// if we can't write header
		ABORT;	// failed to write header
	}
	return true;	// success
}

bool CSnapMovie::ReadFrameIndex()
{
	ASSERT(m_hdr.nFileID == m_nFileID);	// sanity check
	// seek to frame index
	LARGE_INTEGER	nFilePos;
	nFilePos.QuadPart = m_hdr.nIndexOffset;
	if (!SetFilePointerEx(m_hFile, nFilePos, NULL, FILE_BEGIN)) {	// if we can't seek
		ABORT;	// failed to seek to start of file
	}
	// read frame index
	m_aReadFrameIndex.SetSize(m_hdr.nFrameCount);
	DWORD	nBufSize = static_cast<DWORD>(m_hdr.nFrameCount * sizeof(FRAME_INDEX_ENTRY));
	if (!ReadBuf(m_aReadFrameIndex.GetData(), nBufSize)) {	// if we can't read
		ABORT;	// failed to read frame index
	}
	// seek to first snapshot
	nFilePos.QuadPart = sizeof(m_hdr);
	if (!SetFilePointerEx(m_hFile, nFilePos, NULL, FILE_BEGIN)) {	// if we can't seek
		ABORT;	// failed to seek to first snapshot
	}
	return true;	// success
}

bool CSnapMovie::WriteFrameIndex()
{
	INT_PTR	nBlocks = m_aWriteFrameIndex.GetBlockCount();
	ULONGLONG	nFilePos = m_nWriteBytes;
	for (int iBlock = 0; iBlock < nBlocks; iBlock++) {	// for each block
		const FRAME_INDEX_ENTRY* pEntry = m_aWriteFrameIndex.GetBlock(iBlock);
		DWORD	nIdxEntries = static_cast<DWORD>(m_aWriteFrameIndex.GetBlockSize(iBlock));
		DWORD	nBufSize = nIdxEntries * sizeof(FRAME_INDEX_ENTRY);
		if (!WriteWait(pEntry, nBufSize, nFilePos)) {	// if we can't write block
			ABORT;	// failed to write frame index block
		}
		nFilePos += nBufSize;	// increment file position by amount we wrote
	}
#if !ASYNCHRONOUS_WRITING
	LARGE_INTEGER	liZero = {0};
	if (!SetFilePointerEx(m_hFile, liZero, NULL, FILE_BEGIN)) {	// if we can't seek
		ABORT;	// failed to rewind to start of file
	}
#endif
	return true;	// success
}

CSnapshot* CSnapMovie::Read()
{
	// same as CSnapshot::Read except file header is handled differently
	// and we use file handle instead of CFile; no exceptions are thrown
	ASSERT(m_hdr.nFileID == m_nFileID);	// sanity check
	DRAW_STATE	drawState;
	ZeroMemory(&drawState, sizeof(DRAW_STATE));
	if (!ReadBuf(&drawState, min(m_hdr.nDrawStateSize, sizeof(DRAW_STATE)))) {
		ABORT;	// failed to read draw state
	}
	// Allocate a snapshot having sufficient space for the specified number
	// of rings, and attach it to a smart pointer for automatic cleanup.
	CAutoPtr<CSnapshot>	pSnapshot(CSnapshot::Alloc(drawState.nRings));
	if (pSnapshot == NULL) {	// if allocation failed
		ABORT;	// failed to allocate snapshot
	}
	pSnapshot->m_drawState = drawState;
	ZeroMemory(&pSnapshot->m_globRing, sizeof(GLOB_RING));
	if (!ReadBuf(&pSnapshot->m_globRing, min(m_hdr.nGlobRingSize, sizeof(GLOB_RING)))) {
		ABORT;	// failed to read global ring
	}
	if (m_hdr.nRingSize == sizeof(RING)) {	// if ring is current size
		if (!ReadBuf(pSnapshot->m_aRing, sizeof(RING) * drawState.nRings)) {
			ABORT;	// failed to read ring array
		}
	} else {	// legacy ring size; must read rings one at a time
		int	nRingSize = min(m_hdr.nRingSize, sizeof(RING));
		for (int iRing = 0; iRing < drawState.nRings; iRing++) {
			if (!ReadBuf(&pSnapshot->m_aRing[iRing], nRingSize)) {
				ABORT;	// failed to read ring
			}
		}
	}
	m_iReadFrame++;	// increment read position in frames
	// detach snapshot from smart pointer and return it to caller
	return pSnapshot.Detach();	// success
}

bool CSnapMovie::SeekFrame(LONGLONG iFrame)
{
	if (!IsReading()) {	// if not reading
		ABORT2(ERROR_INVALID_OPERATION);	// attempted seek while not reading
	}
	if (iFrame < 0 || iFrame >= GetFrameCount()) {	// if range error
		ABORT2(ERROR_INVALID_INDEX);	// frame index is out of range
	}
	// seek to specified frame
	LARGE_INTEGER	nFilePos;
	nFilePos.QuadPart = m_aReadFrameIndex[iFrame];
	if (!SetFilePointerEx(m_hFile, nFilePos, NULL, FILE_BEGIN)) {	// if we can't seek
		ABORT;	// failed to seek to specified frame
	}
	m_iReadFrame = iFrame;	// set read position
	return true;
}

bool CSnapMovie::FinalizeWrite()
{
	if (!CompletePendingWrites()) {	// if we can't complete writes
		return false;	// fail; error already handled
	}
	if (!WriteFrameIndex()) {	// if we can't write the frame index
		return false;	// fail; error already handled
	}
	if (!WriteHeader()) {	// if we can't rewrite the header
		return false;	// fail; error already handled
	}
	return true;	// success
}

bool CSnapMovie::WriteWait(LPCVOID lpBuffer, DWORD nBytesToWrite, ULONGLONG nFilePos)
{
#if ASYNCHRONOUS_WRITING
	OVERLAPPED	ovl;
	ZeroMemory(&ovl, sizeof(ovl));
	LARGE_INTEGER	li;	// split 64-bit file position in half, for legacy reasons
	li.QuadPart = nFilePos;
	ovl.Offset = li.LowPart;
	ovl.OffsetHigh = li.HighPart;
	// start asynchronous write; if it fails but last error is I/O pending, it's OK
	BOOL	bResult = WriteFile(m_hFile, lpBuffer, nBytesToWrite, NULL, &ovl);
	if (!bResult && GetLastError() != ERROR_IO_PENDING) {	// if any error other than pending
		ABORT;	// failed to write data
	}
	// write was successfully started, so now wait for it to finish
	DWORD	nBytesWritten;
	bResult = GetOverlappedResult(m_hFile, &ovl, &nBytesWritten, true);
	if (!bResult || nBytesWritten != nBytesToWrite) {	// if any error other than pending
		ABORT;	// failed to obtain overlapped result
	}
#else	// !ASYNCHRONOUS_WRITING
	UNREFERENCED_PARAMETER(nFilePos);
	if (!WriteBuf(lpBuffer, nBytesToWrite)) {	// synchronous write
		ABORT;	// failed to write snapshot
	}
#endif	// ASYNCHRONOUS_WRITING
	return true;	// success
}

bool CSnapMovie::Write(const CSnapshot *pSnapshot)
{
	if (pSnapshot == NULL) {	// if null snapshot
		ABORT2(ERROR_INVALID_PARAMETER);	// fail
	}
#if ASYNCHRONOUS_WRITING
	// Look for a free buffer, but without waiting; in normal conditions
	// this first method will succeed and likely return the first buffer.
	int	iBuf = FindWriteBuffer(WS_COMPLETED);	// very efficient
	if (iBuf < 0) {	// if no free buffers were found
		// all buffers are in use; system is fully loaded
		iBuf = WaitForFreeWriteBuffer();	// waits for a completion signal
		if (iBuf < 0) {	// if still no free buffer found; something's wrong
			delete pSnapshot;	// avoid memory leak
			ABORT2(ERROR_NO_SYSTEM_RESOURCES);	// failed to obtain a write buffer
		}
	}
	// got a free buffer
	CWriteBuf&	buf = m_aWriteBuf[iBuf];	// for brevity
	buf.m_pSnapshot.Free();	// it's now safe to free previous snapshot
	buf.m_pSnapshot.Attach(pSnapshot);	// attach new snapshot to buffer
	ZeroMemory(&buf.m_ovl, sizeof(OVERLAPPED));	// clear overlapped struct, as required
	LARGE_INTEGER	li;	// split 64-bit file position in half, for legacy reasons
	li.QuadPart = m_nWriteBytes;
	buf.m_ovl.Offset = li.LowPart;
	buf.m_ovl.OffsetHigh = li.HighPart;
	DWORD	nSnapSize = pSnapshot->GetSize();	// get snapshot size
	// start asynchronous write; if it fails but last error is I/O pending, it's OK
	BOOL	bResult = WriteFile(m_hFile, buf.m_pSnapshot, nSnapSize, NULL, &buf.m_ovl);
	if (!bResult && GetLastError() != ERROR_IO_PENDING) {	// if any error other than pending
		ABORT;	// failed to write snapshot
	}
#else	// !ASYNCHRONOUS_WRITING
	CAutoPtr<const CSnapshot> pSnapshotHolder(pSnapshot);	// ensure cleanup
	DWORD	nSnapSize = pSnapshot->GetSize();	// get snapshot size
	if (!WriteBuf(pSnapshot, nSnapSize)) {	// synchronous write
		ABORT;	// failed to write snapshot
	}
#endif	// ASYNCHRONOUS_WRITING
	m_aWriteFrameIndex.Add(m_nWriteBytes);	// add file position to frame index
	m_nWriteBytes += nSnapSize;	// add snapshot size to total bytes written
	m_nWriteFrames++;	// bump count of frames written
	return true;	// success
}

int CSnapMovie::FindWriteBuffer(bool bDesiredState)
{
	for (int iBuf = 0; iBuf < WRITE_BUFFERS; iBuf++) {	// for each write buffer
		CWriteBuf&	buf = m_aWriteBuf[iBuf];	// for brevity
		// high performance test that polls for completion of an outstanding I/O
		bool	bIsCompleted = HasOverlappedIoCompleted(&buf.m_ovl) != 0;
		if (bIsCompleted == bDesiredState) {	// if buffer is in the desired state
			return iBuf;	// return index of buffer
		}
	}
	return -1;	// no buffers are in the desired state
}

int CSnapMovie::WaitForFreeWriteBuffer()
{
	int	iBuf = FindWriteBuffer(WS_PENDING);	// find a pending write
	if (iBuf < 0) {	// if no writes are pending (unlikely but possible)
		return 0;	// return index of first buffer
	}
	// found a pending write
	CWriteBuf&	buf = m_aWriteBuf[iBuf];	// for brevity
	DWORD	nBytesWritten;
	// wait for pending write to finish
	BOOL	bResult = GetOverlappedResult(m_hFile, &buf.m_ovl, &nBytesWritten, true);
	if (!bResult) {	// if we can't get overlapped result
		return -1;	// fail
	}
	// A write completed, but we don't know which one. This ambiguity could
	// be avoided by giving each write buffer its own independent event object,
	// but it's easier (though maybe less efficient) to just poll them again.
	return FindWriteBuffer(WS_COMPLETED);	// find a completed write
}

bool CSnapMovie::CompletePendingWrites()
{
#if ASYNCHRONOUS_WRITING
	for (int iBuf = 0; iBuf < WRITE_BUFFERS; iBuf++) {	// for each write buffer
		CWriteBuf&	buf = m_aWriteBuf[iBuf];	// for brevity
		DWORD	nBytesWritten;
		// wait for pending write to finish
		BOOL	bResult = GetOverlappedResult(m_hFile, &buf.m_ovl, &nBytesWritten, true);
		if (!bResult) {	// if we can't get overlapped result
			ABORT;	// failed to obtain overlapped result
		}
	}
	// We don't free ANY snapshots until ALL writes are completed, due to
	// ambiguity about which write signaled; see WaitForFreeWriteBuffer.
	for (int iBuf = 0; iBuf < WRITE_BUFFERS; iBuf++) {	// for each write buffer
		CWriteBuf&	buf = m_aWriteBuf[iBuf];	// for brevity
		buf.m_pSnapshot.Free();	// free snapshot
	}
#endif	// ASYNCHRONOUS_WRITING
	return true;	// success
}

bool CSnapMovie::Validate(LPCTSTR pszPath)
{
	m_hFile = CreateFile(pszPath, GENERIC_READ | FILE_SHARE_READ, 
		0, NULL, OPEN_EXISTING, 0, 0);
	if (!IsOpen()) {	// if file not open
		ABORT;	// failed to open file
	}
	if (!ReadHeader()) {	// if we can't read header
		return false;	// fail; error already handled
	}
	printf("%lld frames\n", m_hdr.nFrameCount);
	// read frames sequentially without trying to read index first
	ULONGLONG	nOffset = sizeof(m_hdr);
	for (LONGLONG iFrame = 0; iFrame < m_hdr.nFrameCount; iFrame++) {
		CAutoPtr<CSnapshot>	pSnapshot(Read());
		if (pSnapshot == NULL) {
			printf("read snapshot failed, frame %lld\n", iFrame);
			printf("bytes read = %lld \n", nOffset);
			ABORT;
		}
		nOffset += CSnapshot::GetSize(pSnapshot->m_drawState.nRings);
	}
	return true;	// success
}
