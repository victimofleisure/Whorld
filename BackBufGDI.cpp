// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      11may05	initial version
        01      05jun05	add mirror blit
        02      02sep05	mirror to intermediate buffer
		03		29jan08	in Create, remove unused local var

        GDI back buffer for off-screen drawing
 
*/

#include "stdafx.h"
#include "BackBufGDI.h"

CBackBufGDI::CBackBufGDI()
{
	m_DefaultBitmap = NULL;
	m_Wnd = NULL;
	m_Width = 0;
	m_Height = 0;
	m_IsMirrored = FALSE;
}

CBackBufGDI::~CBackBufGDI()
{
	Destroy();
}

bool CBackBufGDI::Create(CWnd *Wnd)
{
	if (IsCreated())
		return(FALSE);
	m_Wnd = Wnd;
	CClientDC	dc(Wnd);
	if (!m_BackDC.CreateCompatibleDC(&dc))
		return(FALSE);
	if (!m_MirrorDC.CreateCompatibleDC(&dc))
		return(FALSE);
	m_DefaultBitmap = GetCurrentObject(m_BackDC, OBJ_BITMAP);
	if (m_DefaultBitmap == NULL)
		return(FALSE);
	return(TRUE);
}

void CBackBufGDI::Destroy()
{
	DeleteSurface();
}

bool CBackBufGDI::CreateSurface(int Width, int Height)
{
	if (!IsCreated())
		return(FALSE);
	DeleteSurface();
	CClientDC	dc(m_Wnd);
	if (m_IsMirrored) {
		if (!m_BackBitmap.CreateCompatibleBitmap(&dc, 
		(Width + 1) >> 1, (Height + 1) >> 1))
			return(FALSE);
		if (m_BackDC.SelectObject(&m_BackBitmap) == NULL)
			return(FALSE);
		if (!m_MirrorBitmap.CreateCompatibleBitmap(&dc, Width, Height))
			return(FALSE);
		if (m_MirrorDC.SelectObject(&m_MirrorBitmap) == NULL)
			return(FALSE);
	} else {
		if (!m_BackBitmap.CreateCompatibleBitmap(&dc, Width, Height))
			return(FALSE);
		if (m_BackDC.SelectObject(&m_BackBitmap) == NULL)
			return(FALSE);
	}
	m_Width = Width;
	m_Height = Height;
	return(TRUE);
}

void CBackBufGDI::DeleteSurface()
{
	if (!IsCreated())
		return;
	m_BackDC.SelectObject(m_DefaultBitmap);	// deselect back buffer first
	m_BackBitmap.DeleteObject();			// then delete its GDI object
	if (m_MirrorDC.m_hDC != NULL) {
		m_MirrorDC.SelectObject(m_DefaultBitmap);	// deselect mirror buffer first
		m_MirrorBitmap.DeleteObject();				// then delete its GDI object
	}
}

bool CBackBufGDI::Blt(CDC *pFrontDC)
{
	if (m_IsMirrored) {
		int	w = (m_Width + 1) >> 1;
		int	h = (m_Height + 1) >> 1;
		return(
			StretchBlt(m_MirrorDC, 0, 0, w, h, m_BackDC, 0, 0, w, h, SRCCOPY) &&
			StretchBlt(m_MirrorDC, w, 0, w, h, m_BackDC, w - 1, 0, -w, h, SRCCOPY) &&
			StretchBlt(m_MirrorDC, 0, h, w, h, m_BackDC, 0, h - 1, w, -h, SRCCOPY) &&
			StretchBlt(m_MirrorDC, w, h, w, h, m_BackDC, w - 1, h - 1, -w, -h, SRCCOPY) &&
			BitBlt(*pFrontDC, 0, 0, m_Width, m_Height, m_MirrorDC, 0, 0, SRCCOPY));
	} else
		return(BitBlt(*pFrontDC, 0, 0, m_Width, m_Height, m_BackDC, 0, 0, SRCCOPY) != 0);
}

void CBackBufGDI::SetMirror(bool Enable)
{
	m_IsMirrored = Enable;
}
