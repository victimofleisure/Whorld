// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      10may05	initial version
        01      11may05	add error strings
        02      12may05	adapt for DX7
        03      20may05	make DX7 optional
        04      05jun05	add mirror blit
        05      22jun05	add SetOptions
        06      29jun05	add mirror precision option
        07      10jul05	add auto memory option
        08      14oct05	add exclusive mode
        09      14oct05	kill DirectDraw's leaky timer
		10		19oct05	in SetExclusive, save/restore window placement
		11		26oct05	don't set position before restoring placement
		12		05may06	add GetErrorString
		13		02aug06	overload Blt for external surfaces
		14		23nov07	support Unicode
		15		30jan08	fix unmirrored exclusive in system memory case

        DirectDraw back buffer for off-screen drawing
 
*/

#include "stdafx.h"
#include "BackBufDD.h"

#define DIRDRAWERR(x) {x, _T(#x)},
const CBackBufDD::ERRTRAN CBackBufDD::m_ErrTran[] = {
#include "DirDrawErrs.h"
{0, NULL}};

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = NULL; } }

CBackBufDD::CBackBufDD()
{
	m_dd = NULL;
	m_Front = NULL;
	m_Back = NULL;
	m_SysBack = NULL;
	m_Unmirror = NULL;
	m_Mirror = NULL;
	m_DrawBuf = NULL;
	m_Clipper = NULL;
	m_Main = NULL;
	m_View = NULL;
	m_hr = 0;
	m_Width = 0;
	m_Height = 0;
	m_IsMirrored = FALSE;
	m_IsExclusive = FALSE;
	m_Options = OPT_AUTO_MEMORY | OPT_MIRROR_PRECISE;
	ZeroMemory(&m_PreExcl, sizeof(m_PreExcl));
}

CBackBufDD::~CBackBufDD()
{
	Destroy();
}

BOOL CBackBufDD::Create(HWND Main, HWND View, GUID *Driver, bool Exclusive)
{
	Destroy();
#if USE_DIRDRAW7
	if (FAILED(m_hr = DirectDrawCreateEx(Driver, (VOID **)&m_dd, IID_IDirectDraw7, NULL)))
		return(FALSE);
#else
	if (FAILED(m_hr = DirectDrawCreate(Driver, &m_dd, NULL)))
		return(FALSE);
#endif
	m_IsExclusive = Exclusive;	// order matters
	int	mode = Exclusive ? (DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN) : DDSCL_NORMAL;
	if (FAILED(m_hr = m_dd->SetCooperativeLevel(Exclusive ? Main : NULL, mode)))
		return(FALSE);
	// In exclusive mode, DirectDraw creates a 1.5 second periodic timer for
	// unknown reasons.  If the app is switched to exclusive mode and back in
	// less than 1.5 seconds, DirectDraw fails to clean up the timer, and the
	// app's main window receives spurious timer messages for timer ID 16962.
	KillTimer(Main, 16962);	// nuke it, seems to work fine
	m_Main = Main;
	m_View = View;
	return(TRUE);
}

void CBackBufDD::Destroy()
{
	if (!IsCreated())
		return;
	DeleteSurface();
	m_dd->Release();
	m_dd = NULL;
}

bool CBackBufDD::CreateSurface(int Width, int Height)
{
	if (!IsCreated())
		return(FALSE);
	Width = max(Width, 1);	// CreateSurface won't accept zero
	Height = max(Height, 1);
	DeleteSurface();
	DDSURFACEDESC	sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.dwSize = sizeof(sd);
	if (m_IsExclusive) {
		sd.dwFlags 	= DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		sd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		sd.dwBackBufferCount = 2;	// prevents tearing
		if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Front, NULL)))
			return(FALSE);
		DDSCAPS		caps;
		ZeroMemory(&caps, sizeof(caps));
		caps.dwCaps = DDSCAPS_BACKBUFFER;
		if (FAILED(m_hr = m_Front->GetAttachedSurface(&caps, &m_Back)))
			return(FALSE);
		// if desired back buffer location is auto or video memory
		if ((m_Options & (OPT_AUTO_MEMORY | OPT_USE_VIDEO_MEM)) != 0) {
			m_Unmirror = m_Back;	// unmirrored back buffer in video memory
		} else {	// desired back buffer location is system memory
			sd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
			sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE
				| DDSCAPS_SYSTEMMEMORY;
			sd.dwWidth	= Width;
			sd.dwHeight	= Height;
			if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_SysBack, NULL)))
				return(FALSE);
			m_Unmirror = m_SysBack;	// unmirrored back buffer in system memory
		}
		sd.dwFlags 	= DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE
			| DDSCAPS_SYSTEMMEMORY;	// draw to system memory in mirror mode
		sd.dwWidth	= Width >> 1;	// only need upper-left quadrant
		sd.dwHeight	= Height >> 1;
		if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Mirror, NULL)))
			return(FALSE);
		m_DrawBuf = m_IsMirrored ? m_Mirror : m_Unmirror;
	} else {	// not exclusive
		sd.dwFlags = DDSD_CAPS;
		sd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Front, NULL)))
			return(FALSE);
		sd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		// NOTE: must specify DDSCAPS_3DDEVICE to use Direct3D hardware
		sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
		// NOTE: mirroring may not work if DDSCAPS_VIDEOMEMORY is specified
		bool	UseVideoMem;
		if (m_Options & OPT_AUTO_MEMORY)
			UseVideoMem = !m_IsMirrored;
		else
			UseVideoMem = (m_Options & OPT_USE_VIDEO_MEM) != 0;
		if (UseVideoMem)
			sd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		else
			sd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		if (m_IsMirrored) {
			sd.dwWidth		= (Width + 1) >> 1;	// only need upper-left quadrant
			sd.dwHeight		= (Height + 1) >> 1;
			if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Back, NULL)))
				return(FALSE);
			if (m_Options & OPT_MIRROR_PRECISE) {	// mirror to intermediate surface
				sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE
					| DDSCAPS_VIDEOMEMORY;
				sd.dwWidth	= Width;
				sd.dwHeight	= Height;
				if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Mirror, NULL)))
					return(FALSE);
			}
		} else {	// unmirrored
			sd.dwWidth = Width;
			sd.dwHeight	= Height;
			if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Back, NULL)))
				return(FALSE);
		}
		if (FAILED(m_hr = m_dd->CreateClipper(0, &m_Clipper, NULL)))
			return(FALSE);
		if (FAILED(m_hr = m_Clipper->SetHWnd(0, m_View)))
			return(FALSE);
		if (FAILED(m_hr = m_Front->SetClipper(m_Clipper)))
			return(FALSE);
		m_DrawBuf = m_Back;
	}
	m_Width = Width;
	m_Height = Height;
	return(TRUE);
}

void CBackBufDD::DeleteSurface()
{
	if (!IsCreated())
		return;
	SAFE_RELEASE(m_Back);
	SAFE_RELEASE(m_SysBack);
	SAFE_RELEASE(m_Mirror);
	SAFE_RELEASE(m_Clipper);
	SAFE_RELEASE(m_Front);
	m_Unmirror = NULL;
	m_DrawBuf = NULL;
}

bool CBackBufDD::Blt()
{
	enum {
		WAIT	= DDBLT_WAIT,
		WAITFX	= DDBLT_WAIT | DDBLT_DDFX
	};
	while (1) {
		HRESULT hRet;
		if (m_IsExclusive) {
			if (m_IsMirrored) {	// mirrored blit
				int	w = m_Width >> 1;
				int	h = m_Height >> 1;
				CRect	r(0, 0, w, h);
				DDBLTFX	fx;
				ZeroMemory(&fx, sizeof(fx));
				fx.dwSize = sizeof(fx);
				hRet = m_Back->Blt(r, m_Mirror, NULL, WAIT, &fx);	// upper left
				if (hRet == DD_OK) {
					fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT;
					r += CSize(w, 0);
					hRet = m_Back->Blt(r, m_Mirror, NULL, WAITFX, &fx);	// upper right
					if (hRet == DD_OK) {
						fx.dwDDFX = DDBLTFX_MIRRORUPDOWN | DDBLTFX_MIRRORLEFTRIGHT;
						r += CSize(0, h);
						hRet = m_Back->Blt(r, m_Mirror, NULL, WAITFX, &fx);	// lower right
						if (hRet == DD_OK) {
							fx.dwDDFX = DDBLTFX_MIRRORUPDOWN;
							r += CSize(-w, 0);
							hRet = m_Back->Blt(r, m_Mirror, NULL, WAITFX, &fx);	// lower left
							if (hRet == DD_OK)
								hRet = m_Front->Flip(NULL, DDFLIP_WAIT);	// update display
						}
					}
				}
			} else {	// unmirrored
				if (m_SysBack != NULL)	// if drawing to system memory
					// blit system memory back buffer to flipping chain back buffer
					hRet = m_Back->Blt(NULL, m_SysBack, NULL, WAIT, NULL);
				hRet = m_Front->Flip(NULL, DDFLIP_WAIT);	// update display
			}
		} else {	// not exclusive
			CPoint	org(0, 0);
			ClientToScreen(m_View, &org);
			if (m_IsMirrored) {	// mirrored blit
				CRect	r(0, 0, (m_Width + 1) >> 1, (m_Height + 1) >> 1);
				int	w = (m_Width >> 1);
				int	h = (m_Height >> 1);
				DDBLTFX	fx;
				ZeroMemory(&fx, sizeof(fx));
				fx.dwSize = sizeof(fx);
				if (m_Mirror != NULL) {	// mirror to intermediate surface
					hRet = m_Mirror->Blt(r, m_Back, NULL, WAIT, NULL);	// upper left
					if (hRet == DD_OK) {
						fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT;
						r += CSize(w, 0);
						hRet = m_Mirror->Blt(r, m_Back, NULL, WAITFX, &fx);	// upper right
						if (hRet == DD_OK) {
							fx.dwDDFX = DDBLTFX_MIRRORUPDOWN | DDBLTFX_MIRRORLEFTRIGHT;
							r += CSize(0, h);
							hRet = m_Mirror->Blt(r, m_Back, NULL, WAITFX, &fx);	// lower right
							if (hRet == DD_OK) {
								fx.dwDDFX = DDBLTFX_MIRRORUPDOWN;
								r += CSize(-w, 0);
								hRet = m_Mirror->Blt(r, m_Back, NULL, WAITFX, &fx);	// lower left
								if (hRet == DD_OK) {	// blit mirrored image to display
									CRect	wr(0, 0, m_Width, m_Height);
									wr += org;
									hRet = m_Front->Blt(wr, m_Mirror, NULL, WAIT, NULL);
								}
							}
						}
					}
				} else {	// mirror directly to display; quick and dirty
					r += org;
					hRet = m_Front->Blt(r, m_Back, NULL, WAIT, NULL);	// upper left
					if (hRet == DD_OK) {
						fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT;
						r += CSize(w, 0);
						hRet = m_Front->Blt(r, m_Back, NULL, WAITFX, &fx);	// upper right
						if (hRet == DD_OK) {
							fx.dwDDFX = DDBLTFX_MIRRORUPDOWN | DDBLTFX_MIRRORLEFTRIGHT;
							r += CSize(0, h);
							hRet = m_Front->Blt(r, m_Back, NULL, WAITFX, &fx);	// lower right
							if (hRet == DD_OK) {
								fx.dwDDFX = DDBLTFX_MIRRORUPDOWN;
								r += CSize(-w, 0);
								hRet = m_Front->Blt(r, m_Back, NULL, WAITFX, &fx);	// lower left
							}
						}
					}
				}
			} else {	// ordinary blit
				CRect	r(0, 0, m_Width, m_Height);
				r += org;
				hRet = m_Front->Blt(r, m_Back, NULL, WAIT, NULL);
			}
		}
		if (hRet == DD_OK)
			break;
		else if (hRet == DDERR_SURFACELOST) {
			if (FAILED(m_Front->Restore()) || FAILED(m_Back->Restore()))
				return(FALSE);
		} else if (hRet != DDERR_WASSTILLDRAWING)
			return(FALSE);
	}
	return(TRUE);
}

LPCTSTR CBackBufDD::GetErrorString(HRESULT hr)
{
	for (int i = 0; m_ErrTran[i].Text != NULL; i++) {
		if (m_ErrTran[i].Code == hr)
			return(m_ErrTran[i].Text);
	}
	return(_T("unknown error"));
}

void CBackBufDD::SetMirror(bool Enable)
{
	if (m_IsExclusive)
		m_DrawBuf = Enable ? m_Mirror : m_Unmirror;
	m_IsMirrored = Enable;
}

void CBackBufDD::SetOptions(int Options)
{
	m_Options = Options;
}

bool CBackBufDD::SetExclusive(HWND Main, HWND View, bool Enable)
{
	GUID	MonGuid, *Device = NULL;
	CRect	rc;
	if (Enable) {	// if exclusive mode
		GetWindowPlacement(Main, &m_PreExcl);	// save window placement
		// get rect and handle of monitor that this window is mostly on
		HMONITOR	hMon = GetFullScreenRect(View, rc);
		if (GetMonitorGUID(hMon, MonGuid))	// if monitor's GUID is available
			Device = &MonGuid;	// pass GUID to Create for hardware acceleration
		m_Width = rc.Width();
		m_Height = rc.Height();
	}
	if (!Create(Main, View, Device, Enable))
		return(FALSE);
	if (!CreateSurface(m_Width, m_Height))
		return(FALSE);
	if (Enable) {
		SetWindowPos(Main, HWND_TOPMOST,	// set topmost attribute
			rc.left, rc.top, rc.Width(), rc.Height(), 0);	// and go full-screen
	} else {
		SetWindowPos(Main, HWND_NOTOPMOST,	// clear topmost attribute
			0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);	// but don't set position
		SetWindowPlacement(Main, &m_PreExcl);	// restore previous placement
	}
	return(TRUE);
}

HMONITOR CBackBufDD::GetFullScreenRect(HWND hWnd, CRect& rc)
{
	CRect	wr;
	::GetWindowRect(hWnd, wr);
	// try to get screen size from monitor API in case we're dual-monitor
	MONITORINFO	mi;
	mi.cbSize = sizeof(mi);
	HMONITOR	hMon = MonitorFromRect(wr, MONITOR_DEFAULTTONEAREST);
	if (hMon != NULL && GetMonitorInfo(hMon, &mi)) {
		rc = mi.rcMonitor;
	} else {	// fall back to older API
		rc = CRect(0, 0, GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN));
	}
	return(hMon);
}

BOOL WINAPI CBackBufDD::DDEnumCallbackEx(GUID FAR *lpGUID, LPSTR lpDriverDescription, 
	LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm)
{
	MONGUID	*mg = (MONGUID *)lpContext;
	if (hm == mg->Mon) {	// if it's the monitor we're looking for
		mg->Guid = *lpGUID;	// pass its GUID to caller
		mg->Valid = TRUE;	// tell caller we found it
		return(DDENUMRET_CANCEL);	// stop enumerating
	}
	return(DDENUMRET_OK);	// continue enumerating
}
 
bool CBackBufDD::GetMonitorGUID(HMONITOR hMon, GUID& MonGuid)
{
	// DirectDrawEnumerateEx has to be manually imported from the DirectDraw DLL
	HINSTANCE h = LoadLibrary(_T("ddraw.dll"));
	if (!h)
		return(FALSE);
	LPDIRECTDRAWENUMERATEEXA	lpDDEnumEx;	// specify ANSI version of struct
	lpDDEnumEx = (LPDIRECTDRAWENUMERATEEXA)GetProcAddress(h, "DirectDrawEnumerateExA");
	bool	retc = FALSE;
	if (lpDDEnumEx) {	// if the function was imported
		MONGUID	mg;
		ZeroMemory(&mg, sizeof(mg));
		mg.Mon = hMon;	// tell callback which monitor to look for
		lpDDEnumEx(DDEnumCallbackEx, &mg, DDENUM_ATTACHEDSECONDARYDEVICES);
		if (mg.Valid) {	// if callback found the monitor
			MonGuid = mg.Guid;	// pass its GUID to caller
			retc = TRUE;
		}
	}
    FreeLibrary(h);
	return(retc);
}

bool CBackBufDD::Blt(LPRECT DestRect, LPDIRECTDRAWSURFACE SrcSurface,
					 LPRECT SrcRect, DWORD Flags, LPDDBLTFX BltFx)
{
	while (1) {
		HRESULT hRet;
		if (m_IsExclusive) {
			hRet = m_Back->Blt(DestRect, SrcSurface, SrcRect, Flags, BltFx);
			if (hRet == DD_OK)
				hRet = m_Front->Flip(NULL, DDFLIP_WAIT);
		} else
			hRet = m_Front->Blt(DestRect, SrcSurface, SrcRect, Flags, BltFx);
		if (hRet == DD_OK)
			break;
		else if (hRet == DDERR_SURFACELOST) {
			if (FAILED(m_Front->Restore()) || FAILED(m_Back->Restore()))
				return(FALSE);
		} else if (hRet != DDERR_WASSTILLDRAWING)
			return(FALSE);
	}
	return(TRUE);
}
