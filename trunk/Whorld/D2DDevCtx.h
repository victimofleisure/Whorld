// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		15jan25	initial version
		01		12mar25	remove frame size accessor

*/

#pragma once

#include "d2d1_1.h"
#include "d3d11.h"
#include "dxgi1_2.h"

class CD2DDevCtx {
public:
// Construction
	CD2DDevCtx();
	virtual ~CD2DDevCtx();

// Attributes
	bool	IsCreated() const;
	bool	IsFullScreen();

// Operations
	bool	Create(HWND hWnd);
	void	Destroy();
	bool	Resize();
	bool	SetFullScreen(bool bEnable);
	bool	GetOutputDesc(DXGI_OUTPUT_DESC& desc);
	bool	GetAdapterDesc(DXGI_ADAPTER_DESC& desc);
	bool	GetDisplayFrequency(DWORD& dwFrequency);
	static bool	GetDisplayDeviceInfo(LPCTSTR pszDevName, DISPLAY_DEVICE& dd);
	static bool	GetDeviceMode(const DXGI_OUTPUT_DESC& desc, DEVMODE& mode);

// Overrideables
	virtual void	OnError(HRESULT hr, LPCSTR pszSrcFileName, int nLineNum, LPCSTR pszSrcFileDate);
	virtual	bool	CreateUserResources();
	virtual	void	DestroyUserResources();
	virtual void	OnResize();

protected:
// Data members
	CComPtr<ID3D11Device> m_pD3DDevice;	// Direct3D device
	CComPtr<ID2D1Factory1>	m_pD2DFactory;	// Direct2D factory
	CComPtr<ID2D1Device>	m_pD2DDevice;	// Direct2D device
	CComPtr<ID2D1DeviceContext>	m_pD2DDeviceContext;	// Direct2D device context
	CComPtr<IDXGISwapChain1>	m_pSwapChain;	// DXGI swap chain
	CComPtr<ID2D1Bitmap1>	m_pTargetBitmap;	// bitmap to which Direct2D renders
};

inline CD2DDevCtx::~CD2DDevCtx()
{
}

inline bool CD2DDevCtx::IsCreated() const
{
	return m_pTargetBitmap != NULL;
}
