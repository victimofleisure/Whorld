// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		15jan25	initial version

*/

#include "stdafx.h"
#include "D2DDevCtx.h"

#define CHECK(x) { HRESULT hr = x; if (FAILED(hr)) { OnError(hr, __FILE__, __LINE__, __DATE__); return false; }}

CD2DDevCtx::CD2DDevCtx()
{
}

void CD2DDevCtx::OnError(HRESULT hr, LPCSTR pszSrcFileName, int nLineNum, LPCSTR pszSrcFileDate)
{
	UNREFERENCED_PARAMETER(hr);
	UNREFERENCED_PARAMETER(pszSrcFileName);
	UNREFERENCED_PARAMETER(nLineNum);
	UNREFERENCED_PARAMETER(pszSrcFileDate);
}

bool CD2DDevCtx::CreateUserResources()
{
	return true;
}

void CD2DDevCtx::DestroyUserResources()
{
}

void CD2DDevCtx::OnResize()
{
}

bool CD2DDevCtx::Create(HWND hWnd)
{
	CHECK(D3D11CreateDevice(
		NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		NULL, 0, D3D11_SDK_VERSION, &m_pD3DDevice, NULL, NULL));
	CHECK(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory));
	CComPtr<IDXGIDevice1> pDXGIDevice;
	CHECK(m_pD3DDevice->QueryInterface(IID_PPV_ARGS(&pDXGIDevice)));
	CHECK(m_pD2DFactory->CreateDevice(pDXGIDevice, &m_pD2DDevice));
	D2D1_DEVICE_CONTEXT_OPTIONS	optsDevCtx = 
//		D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
		// geometry rendering MAY be performed on many threads in parallel 
		D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS;
	CHECK(m_pD2DDevice->CreateDeviceContext(optsDevCtx, &m_pD2DDeviceContext));
	CComPtr<IDXGIAdapter> pDXGIAdapter;
	CHECK(pDXGIDevice->GetAdapter(&pDXGIAdapter));
	CComPtr<IDXGIFactory2> pDXGIFactory;
	CHECK(pDXGIAdapter->GetParent(IID_PPV_ARGS(&pDXGIFactory)));
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;	// flip presentation model avoids blits
	CHECK(pDXGIFactory->CreateSwapChainForHwnd(m_pD3DDevice, hWnd, &swapChainDesc, NULL, NULL, &m_pSwapChain));
	CComPtr<IDXGISurface> pBackBuffer;
	CHECK(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)));
	CHECK(m_pD2DDeviceContext->CreateBitmapFromDxgiSurface(pBackBuffer, NULL, &m_pTargetBitmap));
	m_pD2DDeviceContext->SetTarget(m_pTargetBitmap);
	return CreateUserResources();
}

void CD2DDevCtx::Destroy()
{
	DestroyUserResources();
	if (m_pD2DDeviceContext != NULL)
		m_pD2DDeviceContext->SetTarget(NULL);  // unbind target before release
	if (m_pD3DDevice != NULL) {
		CComPtr<ID3D11DeviceContext> pD3DContext;
		m_pD3DDevice->GetImmediateContext(&pD3DContext);
		if (pD3DContext != NULL) {
			pD3DContext->ClearState();  // Unbind all views
			pD3DContext->Flush();       // Force immediate destruction
		}
	}
	m_pSwapChain.Release();
	m_pTargetBitmap.Release();
	m_pD2DDeviceContext.Release();
	m_pD2DDevice.Release();
	m_pD2DFactory.Release();
	m_pD3DDevice.Release();	// this implicitly releases the DXGI device
}

bool CD2DDevCtx::Resize()
{
	ASSERT(m_pSwapChain != NULL);
	m_pD2DDeviceContext->SetTarget(NULL); // release the target
	m_pTargetBitmap.Release();
	CHECK(m_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0));
	CComPtr<IDXGISurface> pBackBuffer;
	CHECK(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)));
	CHECK(m_pD2DDeviceContext->CreateBitmapFromDxgiSurface(pBackBuffer, NULL, &m_pTargetBitmap));
	m_pD2DDeviceContext->SetTarget(m_pTargetBitmap);	// reset the target
	OnResize();
	return true;
}

bool CD2DDevCtx::IsFullScreen()
{
	ASSERT(m_pSwapChain != NULL);
	BOOL bIsFullscreen = FALSE;
	CHECK(m_pSwapChain->GetFullscreenState(&bIsFullscreen, NULL));
	return bIsFullscreen != 0;
}

bool CD2DDevCtx::SetFullScreen(bool bEnable)
{
	if (bEnable == IsFullScreen())
		return false;
	CHECK(m_pSwapChain->SetFullscreenState(bEnable, NULL));
	Resize();
	return true;
}

bool CD2DDevCtx::GetOutputDesc(DXGI_OUTPUT_DESC& desc)
{
	ASSERT(m_pSwapChain != NULL);
	CComPtr<IDXGIOutput> pOutput;
	CHECK(m_pSwapChain->GetContainingOutput(&pOutput));
	CHECK(pOutput->GetDesc(&desc));
	return true;
}

bool CD2DDevCtx::GetAdapterDesc(DXGI_ADAPTER_DESC& desc)
{
	ASSERT(m_pD3DDevice != NULL);
	CComPtr<IDXGIDevice> pDXGIDevice;
	CHECK(m_pD3DDevice->QueryInterface(IID_PPV_ARGS(&pDXGIDevice)));
	CComPtr<IDXGIAdapter> pDXGIAdapter;
    CHECK(pDXGIDevice->GetAdapter(&pDXGIAdapter));
    CHECK(pDXGIAdapter->GetDesc(&desc));
	return true;
}

bool CD2DDevCtx::GetDisplayDeviceInfo(LPCTSTR pszDevName, DISPLAY_DEVICE& dd)
{
	ZeroMemory(&dd, sizeof(dd));
	dd.cb = sizeof(dd);
	return EnumDisplayDevices(pszDevName, 0, &dd, 0) != 0;
}

bool CD2DDevCtx::GetDeviceMode(const DXGI_OUTPUT_DESC& desc, DEVMODE& mode)
{
	ZeroMemory(&mode, sizeof(mode));
	mode.dmSize = sizeof(mode);
	if (!EnumDisplaySettings(desc.DeviceName, ENUM_CURRENT_SETTINGS, &mode))
		return false;
	return true;
}

bool CD2DDevCtx::GetDisplayFrequency(DWORD& dwFrequency)
{
	DXGI_OUTPUT_DESC	desc;
	if (!GetOutputDesc(desc))
		return false;
	DEVMODE	mode;
	if (!GetDeviceMode(desc, mode))
		return false;
	dwFrequency = mode.dmDisplayFrequency;
	return true;
}
