// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      16aug06	initial version
		01		05mar07	replace broken IAMVfwCompressDialogs::SetState
		02		23nov07	support Unicode
		03		29jan08	add const GetInfo, return ref instead of ptr

        video compressor list

*/

#include "stdafx.h"
#include "Resource.h"
#include "VideoComprList.h"
#include "vfw.h"	// for ICM messages

// these should be members of the VfwCompressDialogs enum
#define VfwCompressDialog_QueryConfig	0x04
#define VfwCompressDialog_QueryAbout	0x08

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = NULL; } }

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVideoComprList dialog

CVideoComprList::CVideoComprList()
{
	m_pGraph = NULL;
	m_pSource = NULL;
	m_pDest = NULL;
	m_pSourceOut = NULL;
	m_pDestIn = NULL;
	m_pComprIn = NULL;
	m_pComprOut = NULL;
	m_hr = 0;
	m_CurSel = 0;
	m_IsCoInit = FALSE;
}

CVideoComprList::~CVideoComprList()
{
	DisconnectCompr();	// disconnect compressor and remove it from graph
	SAFE_RELEASE(m_pGraph);	// release graph reference if any
	// free all resources held by compressor info list
	int	count = GetCount();
	for (int i = 0; i < count; i++) {
		INFO&	ip = m_Info[i];
		SAFE_RELEASE(ip.pVidCom);
		SAFE_RELEASE(ip.pVfwDlg);
		SAFE_RELEASE(ip.pCompr);
		SAFE_RELEASE(ip.pMoniker);
		delete [] ip.pDlgState;
		ip.pDlgState = NULL;
	}
	if (m_IsCoInit)	{	// if COM was initialized
		CoUninitialize();	// uninitialize it
		m_IsCoInit = FALSE;
	}
}

HRESULT CVideoComprList::SetState(IAMVfwCompressDialogs *pVfwDlg, LPVOID pState, int cbState)
{
	// IAMVfwCompressDialogs::SetState is broken: it sets the codec state, but
	// fails to set the configuration dialog state, which means to initialize
	// the dialog to non-default values we must talk directly to the driver
	return(pVfwDlg->SendDriverMessage(ICM_SETSTATE, (LONG)pState, cbState));
}

bool CVideoComprList::SetFilters(IBaseFilter *pSource, IBaseFilter *pDest)
{
	if (pSource == NULL || pDest == NULL)	// validate arguments
		return FALSE;
	SAFE_RELEASE(m_pGraph);	// release previous graph reference if any
	FILTER_INFO	Info;
	// assume both filters are currently in the same graph
	m_hr = pSource->QueryFilterInfo(&Info);	// get source filter info
	if (FAILED(m_hr) || Info.pGraph == NULL)
		return FALSE;
	m_pGraph = Info.pGraph;	// QueryFilterInfo added a reference to graph
	m_pSource = pSource;
	m_pDest = pDest;
	return TRUE;
}

bool CVideoComprList::CreateList()
{
	if (m_IsCoInit)
		return(TRUE);	// already created
	HRESULT	m_hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (FAILED(m_hr))
		return(FALSE);
	m_IsCoInit = TRUE;	// COM is initialized
	// enumerate video compressors
	CPtrArray	Moniker;
	CDirShowU::EnumDevs(CLSID_VideoCompressorCategory, m_Name, Moniker);
	// add uncompressed video to end of compressor list
	CString uc((LPCTSTR)IDS_VCD_UNCOMPRESSED);
	m_Name.Add(uc);
	Moniker.Add(NULL);
	// populate compressor info array
	INFO	Info;
	ZeroMemory(&Info, sizeof(Info));
	int count = GetCount();
	for (int i = 0; i < count; i++) {	// for each compressor
		Info.pMoniker = (IMoniker *)Moniker[i];	// copy its moniker to info struct
		m_Info.Add(Info);						// add info struct to info array
	}
	m_CurSel = count - 1;	// default selection is uncompressed video
	return(TRUE);
}

bool CVideoComprList::ConnectCompr(int ComprIdx)
{
	INFO&	ip = m_Info[ComprIdx];
	if (m_pGraph == NULL)	// SetFilters wasn't called, or didn't succeed
		return FALSE;
	if (ip.pCompr == NULL)	// compressor hasn't been created
		return FALSE;
	m_hr = m_pGraph->AddFilter(ip.pCompr, L"Compressor");	// add compressor to graph
	if (SUCCEEDED(m_hr)) {
		// get pins and connect source -> compressor -> destination
		m_pSourceOut	= CDirShowU::GetPin(m_pSource, PINDIR_OUTPUT);
		m_pDestIn		= CDirShowU::GetPin(m_pDest, PINDIR_INPUT);
		m_pComprIn		= CDirShowU::GetPin(ip.pCompr, PINDIR_INPUT);
		m_pComprOut		= CDirShowU::GetPin(ip.pCompr, PINDIR_OUTPUT);
		m_hr = m_pGraph->ConnectDirect(m_pSourceOut, m_pComprIn, NULL);
		if (SUCCEEDED(m_hr))
			m_hr = m_pGraph->ConnectDirect(m_pComprOut, m_pDestIn, NULL);
	}
	return SUCCEEDED(m_hr);
}

void CVideoComprList::DisconnectCompr()
{
	if (m_pGraph != NULL) {
		m_pGraph->Disconnect(m_pSourceOut);	// disconnect all pins
		m_pGraph->Disconnect(m_pDestIn);
		m_pGraph->Disconnect(m_pComprIn);
		m_pGraph->Disconnect(m_pComprOut);
		m_pGraph->RemoveFilter(GetCompr());	// remove compressor from graph
	}
	SAFE_RELEASE(m_pSourceOut);	// release pins
	SAFE_RELEASE(m_pDestIn);
	SAFE_RELEASE(m_pComprIn);
	SAFE_RELEASE(m_pComprOut);
}

bool CVideoComprList::SelectCompr(int ComprIdx)
{
	DisconnectCompr();	// disconnect compressor and remove it from graph
	if (ComprIdx == m_Info.GetSize() - 1) {	// if user selected uncompressed
		m_CurSel = ComprIdx;	// nothing to do but set current selection
		return(TRUE);
	}
	INFO&	ip = m_Info[ComprIdx];
	if (ip.pCompr == NULL) {	// if compressor isn't already created
		IMoniker *pMoniker = ip.pMoniker;	// get compressor's moniker
		if (pMoniker == NULL)
			return FALSE;
		m_hr = pMoniker->BindToObject(NULL, NULL,	// create compressor
			IID_IBaseFilter, (void **)&ip.pCompr);
		if (FAILED(m_hr))
			return FALSE;
		m_hr = ip.pCompr->QueryInterface(	// get IAMVfwCompressDialogs
			IID_IAMVfwCompressDialogs, (void **)&ip.pVfwDlg);
		if (SUCCEEDED(m_hr)) {	// query dialog support
			ip.Caps.CanConfig = ip.pVfwDlg->ShowDialog(	// Configure dialog
				VfwCompressDialog_QueryConfig, 0) == S_OK;
			ip.Caps.CanAbout = ip.pVfwDlg->ShowDialog(	// About dialog
				VfwCompressDialog_QueryAbout, 0) == S_OK;
		}
		IPin	*pComprOut = CDirShowU::GetPin(ip.pCompr, PINDIR_OUTPUT);
		if (pComprOut != NULL) {	// if we got compressor's output pin
			m_hr = pComprOut->QueryInterface(	// get IAMVideoCompression
				IID_IAMVideoCompression, (void **)&ip.pVidCom);
			if (SUCCEEDED(m_hr)) {	// retrieve compressor's defaults and capabilities
				int cbVersion;
				int cbDescrip;
				m_hr = ip.pVidCom->GetInfo(NULL, &cbVersion, NULL, &cbDescrip, 
					&ip.KeyFrameRate, &ip.PFramesPerKey, &ip.Quality, &ip.CapsBits);
				if (SUCCEEDED(m_hr)) {
					ip.Caps.CanQuality		= (ip.CapsBits & CompressionCaps_CanQuality) != 0;
					ip.Caps.CanCrunch		= (ip.CapsBits & CompressionCaps_CanCrunch) != 0;
					ip.Caps.CanKeyFrame	= (ip.CapsBits & CompressionCaps_CanKeyFrame) != 0;
					ip.Caps.CanBFrame		= (ip.CapsBits & CompressionCaps_CanBFrame) != 0;
					ip.Caps.CanWindow		= (ip.CapsBits & CompressionCaps_CanWindow) != 0;
				}
			}
			pComprOut->Release();
		}
	}
	// compressor must be connected, else Vfw Configure dialog ignores SetState
	ip.Caps.CanConnect = ConnectCompr(ComprIdx);
	m_CurSel = ComprIdx;	// compressor is now officially selected
	return TRUE;
}

bool CVideoComprList::ShowConfigDlg(HWND hWnd)
{
	INFO&	ip = GetCurInfo();
	IAMVfwCompressDialogs	*pVfwDlg = ip.pVfwDlg;
	if (pVfwDlg == NULL)
		return(FALSE);
	if (ip.pDlgState != NULL) {	// if dialog state was saved
		m_hr = SetState(pVfwDlg, ip.pDlgState, ip.szDlgState);	// restore it
		if (FAILED(m_hr))
			return(FALSE);
	}
	m_hr = pVfwDlg->ShowDialog(VfwCompressDialog_Config, hWnd);	// show dialog
	delete [] ip.pDlgState;	// delete previous dialog state
	ip.pDlgState = NULL;	// mark it deleted
	if (FAILED(m_hr))
		return(FALSE);
	int dsz;
	m_hr = pVfwDlg->GetState(NULL, &dsz);	// get size of dialog state
	if (FAILED(m_hr))
		return(FALSE);
	ip.pDlgState = new char[dsz];	// allocate new state
	ip.szDlgState = dsz;
	m_hr = pVfwDlg->GetState(ip.pDlgState, &dsz);	// save dialog state
	if (FAILED(m_hr))
		return(FALSE);
	return(TRUE);
}

bool CVideoComprList::ShowAboutDlg(HWND hWnd)
{
	INFO&	ip = GetCurInfo();
	IAMVfwCompressDialogs	*pVfwDlg = ip.pVfwDlg;
	if (pVfwDlg == NULL)
		return(FALSE);
	m_hr = pVfwDlg->ShowDialog(VfwCompressDialog_About, hWnd);	// show dialog
	if (FAILED(m_hr))
		return(FALSE);
	return(TRUE);
}

bool CVideoComprList::SetQuality(double Quality)
{
	INFO&	ip = GetCurInfo();
	if (ip.pVidCom == NULL)
		return(FALSE);
	m_hr = ip.pVidCom->put_Quality(Quality);
	if (FAILED(m_hr))
		return(FALSE);
	ip.Quality = Quality;
	return(TRUE);
}

bool CVideoComprList::SetDataRate(long DataRate)
{
	INFO&	ip = GetCurInfo();
	if (ip.pVidCom == NULL)
		return(FALSE);
	VIDEOINFOHEADER	vih;
	m_hr = CDirShowU::RWVideoInfo(ip.pCompr, vih, FALSE);	// read video info
	if (FAILED(m_hr))
		return(FALSE);
	vih.dwBitRate = DataRate;
	m_hr = CDirShowU::RWVideoInfo(ip.pCompr, vih, TRUE);	// write video info
	if (FAILED(m_hr))
		return(FALSE);
	ip.DataRate = DataRate;
	return(TRUE);
}

bool CVideoComprList::SetKeyFrameRate(long KeyFrameRate)
{
	INFO&	ip = GetCurInfo();
	if (ip.pVidCom == NULL)
		return(FALSE);
	m_hr = ip.pVidCom->put_KeyFrameRate(KeyFrameRate);
	if (FAILED(m_hr))
		return(FALSE);
	ip.KeyFrameRate = KeyFrameRate;
	return(TRUE);
}

bool CVideoComprList::SetPFramesPerKey(long PFramesPerKey)
{
	INFO&	ip = GetCurInfo();
	if (ip.pVidCom == NULL)
		return(FALSE);
	m_hr = ip.pVidCom->put_PFramesPerKeyFrame(PFramesPerKey);
	if (FAILED(m_hr))
		return(FALSE);
	ip.PFramesPerKey = PFramesPerKey;
	return(TRUE);
}

bool CVideoComprList::SetWindowSize(long WindowSize)
{
	INFO&	ip = GetCurInfo();
	if (ip.pVidCom == NULL)
		return(FALSE);
	m_hr = ip.pVidCom->put_WindowSize(WindowSize);
	if (FAILED(m_hr))
		return(FALSE);
	ip.WindowSize = WindowSize;
	return(TRUE);
}

void CVideoComprList::SaveState(CVideoComprState& State) const
{
	const INFO&	ip = GetCurInfo();
	State.m_Name = m_Name[m_CurSel];
	int	dsz = ip.szDlgState;
	State.m_DlgState.SetSize(dsz);
	if (ip.pDlgState != NULL)
		memcpy(State.m_DlgState.GetData(), ip.pDlgState, dsz);
	State.m_Parms.Quality = ip.Quality;
	State.m_Parms.DataRate = ip.DataRate;
	State.m_Parms.KeyFrameRate = ip.KeyFrameRate;
	State.m_Parms.PFramesPerKey = ip.PFramesPerKey;
	State.m_Parms.WindowSize = ip.WindowSize;
}

bool CVideoComprList::RestoreState(const CVideoComprState& State)
{
	int	i, count = GetCount();
	for (i = 0; i < count; i++) {	// search for compressor name
		if (m_Name[i] == State.m_Name)
			break;
	}
	if (i >= count)
		return(FALSE);	// unknown compressor name
	INFO&	ip = m_Info[i];
	delete [] ip.pDlgState;	// delete previous dialog state
	ip.pDlgState = NULL;	// mark it deleted
	int	dsz = State.m_DlgState.GetSize();	// size of caller's dialog state
	ip.szDlgState = dsz;
	if (dsz) {	// if caller has a dialog state, copy it
		ip.pDlgState = new char[dsz];
		memcpy(ip.pDlgState, State.m_DlgState.GetData(), dsz);
	}
	if (!SelectCompr(i))	// connect compressor first or SetState won't work
		return(FALSE);
	// older codecs can lie about their capabilities, so ignore set parameter errors
	SetQuality(State.m_Parms.Quality);
	SetDataRate(State.m_Parms.DataRate);
	SetKeyFrameRate(State.m_Parms.KeyFrameRate);
	SetPFramesPerKey(State.m_Parms.PFramesPerKey);
	SetWindowSize(State.m_Parms.WindowSize);
	if (ip.pDlgState != NULL) {	// if we have a dialog state
		if (ip.pVfwDlg == NULL)	// dialog interface is required
			return(FALSE);
		m_hr = SetState(ip.pVfwDlg, ip.pDlgState, dsz);	// restore dialog state
		if (FAILED(m_hr))
			return(FALSE);
	}
	return(TRUE);
}
