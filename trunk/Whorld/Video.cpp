// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06may06	initial version
		01		07jun06	add seek
		02		15sep06	in TimerHook, get frame after updating m_FrameIdx
		03		11oct06	add DestroySurface
		04		28oct06	add GetProps
		05		28nov06	fix memory leak in Close
		06		23nov07	support Unicode

        video clip
 
*/

#include "stdafx.h"
#include "Video.h"

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = NULL; } }

DDSURFACEDESC CVideo::m_DefSurf = {
	sizeof(DDSURFACEDESC),	// dwSize
	DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_LPSURFACE | DDSD_PIXELFORMAT,	// dwFlags
	1,	// dwHeight
	1,	// dwWidth
	4,	// lPitch (Width * BitCount / 8)
	0, 0, 0, 0, // dwBackBufferCount, dwMipMapCount, dwAlphaBitDepth, dwReserved
	&m_DefSurfMem, // lpSurface
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, // color keys
	{
		sizeof(DDPIXELFORMAT),	// dwSize
		DDPF_RGB,	// dwFlags
		0,			// dwFourCC
		32, 		// dwRGBBitCount
		0xff0000,	// dwRBitMask
		0x00ff00,	// dwGBitMask
		0x0000ff	// dwBBitMask
	}
};
DWORD CVideo::m_DefSurfMem; // pointed to by m_DefSurf.lpSurface

CVideo::CVideo()
{
	m_Surface = NULL;
	m_FrameBmp = NULL;
	m_FrameBits = NULL;
	m_FrameIdx = 0;
	m_FirstFrame = FALSE;
	m_hr = NULL;
}

CVideo::~CVideo()
{
	SAFE_RELEASE(m_Surface);
}

bool CVideo::Open(LPCTSTR Path)
{
	if (m_Surface == NULL)	// in case DirectDraw is disabled
		return(FALSE);
	Close();
	if (!m_AviFile.Open(Path))
		return(FALSE);
	m_FrameBmp = m_AviFile.GetFrame(0);	// get initial frame
	if (!UpdateSurface()) {
		Close();
		return(FALSE);
	}
	m_Path = Path;
	m_FirstFrame = TRUE;
	return(TRUE);
}

void CVideo::Close()
{
	// if surface exists, we must attach it to a default 1 x 1 memory surface,
	// otherwise DirectDraw leaves a mysterious hidden frame buffer allocated
	if (m_Surface != NULL)
		m_Surface->SetSurfaceDesc(&m_DefSurf, 0);	// prevents a major leak
	m_FrameBmp = NULL;
	m_FrameBits = NULL;
	m_FrameIdx = 0;
	m_FirstFrame = FALSE;
	m_Path.Empty();
	m_AviFile.Close();
}

void CVideo::TimerHook()
{
	if (m_FirstFrame)
		m_FirstFrame = FALSE;
	else {
		if (m_AviFile.IsOpen()) {
			m_FrameIdx++;
			if (m_FrameIdx >= m_AviFile.GetFrameCount())
				m_FrameIdx = 0;
			m_FrameBmp = m_AviFile.GetFrame(m_FrameIdx);	// get next frame
		}
	}
}

HRESULT CVideo::CreateSurface(CBackBufDD& BackBuf)
{
	SAFE_RELEASE(m_Surface);
	DDSURFACEDESC	sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.dwSize = sizeof(sd);
	sd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY;
	sd.dwWidth = 1;
	sd.dwHeight	= 1;
	m_hr = BackBuf.CreateSurface(&sd, &m_Surface);
	if (SUCCEEDED(m_hr) && m_FrameBmp != NULL) {	// if created ok and video is open
		m_FrameBits = NULL;	// force UpdateSurface to actually update
		UpdateSurface();
	}
	return(m_hr);
}

void CVideo::DestroySurface()
{
	SAFE_RELEASE(m_Surface);
}

bool CVideo::UpdateSurface()
{
	ASSERT(m_FrameBmp != NULL);
	LPBITMAPINFOHEADER	lpbi = m_FrameBmp;
	LPBYTE	pData = (LPBYTE)lpbi + lpbi->biSize;
	if (pData != m_FrameBits) {	// if bitmap moved, update surface description
		DDSURFACEDESC	sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.dwSize = sizeof(sd);
		sd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH 
			| DDSD_LPSURFACE | DDSD_PIXELFORMAT;
		sd.dwWidth = lpbi->biWidth;
		sd.dwHeight = lpbi->biHeight;
		sd.lPitch = lpbi->biWidth * lpbi->biBitCount / 8;
		sd.lpSurface = pData;
		sd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		sd.ddpfPixelFormat.dwRGBBitCount = lpbi->biBitCount;
		switch (lpbi->biBitCount) {
		case 16:
			sd.ddpfPixelFormat.dwFlags = DDPF_RGB;
			if (lpbi->biCompression == BI_BITFIELDS) {
				sd.ddpfPixelFormat.dwRBitMask = 0xf800;	// 5-6-5
				sd.ddpfPixelFormat.dwGBitMask = 0x07e0;
				sd.ddpfPixelFormat.dwBBitMask = 0x001f;
			} else {
				sd.ddpfPixelFormat.dwRBitMask = 0x7c00;	// 5-5-5
				sd.ddpfPixelFormat.dwGBitMask = 0x03e0;
				sd.ddpfPixelFormat.dwBBitMask = 0x001f;
			}
			break;
		case 24:
		case 32:
			sd.ddpfPixelFormat.dwFlags = DDPF_RGB;
			sd.ddpfPixelFormat.dwRBitMask = 0xff0000;
			sd.ddpfPixelFormat.dwGBitMask = 0x00ff00;
			sd.ddpfPixelFormat.dwBBitMask = 0x0000ff;
			break;
		}
		if (FAILED(m_hr = m_Surface->SetSurfaceDesc(&sd, 0)))
			return(FALSE);
		m_FrameBits = pData;
	}
	return(TRUE);
}

HRESULT CVideo::GetLastError() const
{
	if (FAILED(m_hr))
		return(m_hr);
	return(m_AviFile.GetLastError());
}

LPCTSTR CVideo::GetLastErrorString() const
{
	if (FAILED(m_hr))
		return(CBackBufDD::GetErrorString(m_hr));
	return(m_AviFile.GetLastErrorString());
}

void CVideo::Seek(DWORD Frame)
{
	m_FrameIdx = Frame;
	m_FrameBmp = m_AviFile.GetFrame(Frame);
	m_FirstFrame = TRUE;
}

bool CVideo::GetProps(PROPS& Props) const
{
	AVISTREAMINFO si;
	if (!m_AviFile.GetStreamInfo(si))
		return(FALSE);
	Props.FrameSize = GetFrameSize();
	Props.FrameCount = GetFrameCount();
	Props.FrameRate = float(si.dwRate) / si.dwScale;	// avoid 2nd GetStreamInfo
	Props.BitCount = GetBitCount();
	Props.FCCHandler = si.fccHandler;
	return(TRUE);
}
