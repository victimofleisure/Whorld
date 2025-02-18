// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      03feb06	initial version
		01		16may06	remove GetStreamInfo
		02		28oct06	add GetBitCount and GetStreamInfo
		03		23nov07	support Unicode

		AVI file frame-grabber
 
*/

#include "stdafx.h"
#include "AviToBmp.h"

#define AVIFILEERR(x) {x, _T(#x)},
const CAviToBmp::ERRTRAN CAviToBmp::m_ErrTran[] = {
#include "AviFileErrs.h"
{0, NULL}};

CAviToBmp::CAviToBmp()
{
	Init();
	AVIFileInit();
}

CAviToBmp::~CAviToBmp()
{
	Close();
	AVIFileExit();
}

void CAviToBmp::Init()
{
	m_hr = NULL;
	m_pFile = NULL;
	m_pStream = NULL;
	m_pBmpInfo = NULL;
	m_pGetFrame = NULL;
	m_FrameCount = 0;
}

void CAviToBmp::Close()
{
	if (m_pGetFrame != NULL)
		AVIStreamGetFrameClose(m_pGetFrame);
	if (m_pStream != NULL)
		AVIStreamRelease(m_pStream);
	if (m_pFile != NULL)
		AVIFileRelease(m_pFile);
	if (m_pBmpInfo != NULL)
		delete m_pBmpInfo;
	Init();
}

bool CAviToBmp::Open(LPCTSTR Path)
{
	Close();
	if (FAILED(m_hr = AVIFileOpen(&m_pFile, Path, OF_READ, NULL)))
		return(FALSE);
	if (FAILED(m_hr = AVIFileGetStream(m_pFile, &m_pStream, streamtypeVIDEO, 0)))
		return(FALSE);
	m_FrameCount = AVIStreamLength(m_pStream);
	long	Start = AVIStreamStart(m_pStream);
	if (Start < 0)
		return(FALSE);
	long	FmtSize;
 	if (FAILED(m_hr = AVIStreamReadFormat(m_pStream, Start, NULL, &FmtSize)))
		return(FALSE);
	m_pBmpInfo = (LPBITMAPINFO)new BYTE[FmtSize];
	if (FAILED(m_hr = AVIStreamReadFormat(m_pStream, Start, m_pBmpInfo, &FmtSize)))
		return(FALSE);
	m_pGetFrame = AVIStreamGetFrameOpen(m_pStream, (LPBITMAPINFOHEADER)AVIGETFRAMEF_BESTDISPLAYFMT);
	if (m_pGetFrame == NULL)
		return(FALSE);
	return(TRUE);
}

LPBITMAPINFOHEADER CAviToBmp::GetFrame(DWORD Frame)
{
	return((LPBITMAPINFOHEADER)AVIStreamGetFrame(m_pGetFrame, Frame));
}

LPCTSTR CAviToBmp::GetLastErrorString() const
{
	HRESULT	Code = GetLastError();
	for (int i = 0; m_ErrTran[i].Text != NULL; i++) {
		if (m_ErrTran[i].Code == Code)
			return(m_ErrTran[i].Text);
	}
	return(_T("unknown error"));
}

bool CAviToBmp::GetStreamInfo(AVISTREAMINFO& Info) const
{
	return(!AVIStreamInfo(m_pStream, &Info, sizeof(Info)));
}

float CAviToBmp::GetFrameRate() const
{
	AVISTREAMINFO si;
	return(GetStreamInfo(si) ? float(si.dwRate) / si.dwScale : 0);
}
