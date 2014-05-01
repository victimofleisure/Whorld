// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      03feb06	initial version
        01      16aug06	add save/restore compressor state
		02		05mar07	let dialog own compressor state
		03		23nov07	support Unicode
		04		29jan08	remove file write CLSID to fix warning

        DirectShow BMP to AVI converter
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "BmpToAvi.h"
#include "VideoComprDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = NULL; } }
#define ABORTIF(cond, err) { if (cond) { m_err = err; return FALSE; } }
#define ABORTCO(err) ABORTIF(FAILED(m_hr), err)

// {E2510970-F137-11CE-8B67-00AA00A3F1A6}
static const GUID CLSID_AviMux = {
	0xE2510970, 0xF137, 0x11CE, { 0x8B, 0x67, 0x00, 0xAA, 0x00, 0xA3, 0xF1, 0xA6 }
};

// generate table that maps our errors to resource strings
#undef BMPTOAVIERR
#define BMPTOAVIERR(name) IDS_BTAE_##name,
const int CBmpToAvi::m_ErrStrID[] = {
#include "BmpToAviErrs.h"
};

CBmpToAvi::CBmpToAvi()
{
	m_pGraph = NULL;
	m_pControl = NULL;
	m_pEvent = NULL;
	m_pSource = NULL;
	m_pAviMux = NULL;
	m_pFileWriter = NULL;
	m_pBmpToAvi = NULL;
	m_pSourceOut = NULL;
	m_pMuxInput1 = NULL;
	m_pMuxAviOut = NULL;
	m_pWriterIn = NULL;
	m_pComprIn = NULL;
	m_pComprOut = NULL;
	m_hr = 0;
	m_err = 0;
	m_IsCoInit = FALSE;
}

CBmpToAvi::~CBmpToAvi()
{
	Close();
	if (m_IsCoInit)
		CoUninitialize();
}

bool CBmpToAvi::Close()
{
	if (m_pControl != NULL) {
		m_hr = m_pControl->Stop();
		if (m_pEvent != NULL) {
			long	evCode;
			m_pEvent->WaitForCompletion(INFINITE, &evCode);
		}
	}
	SAFE_RELEASE(m_pSourceOut);
	SAFE_RELEASE(m_pMuxInput1);
	SAFE_RELEASE(m_pMuxAviOut);
	SAFE_RELEASE(m_pWriterIn);
	SAFE_RELEASE(m_pComprIn);
	SAFE_RELEASE(m_pComprOut);
	SAFE_RELEASE(m_pBmpToAvi);
	SAFE_RELEASE(m_pSource);
	SAFE_RELEASE(m_pAviMux);
	SAFE_RELEASE(m_pFileWriter);
	SAFE_RELEASE(m_pEvent);
	SAFE_RELEASE(m_pControl);
	SAFE_RELEASE(m_pGraph);
	return SUCCEEDED(m_hr);
}

bool CBmpToAvi::InitGraph()
{
	Close();	// in case our instance is reused
	// init COM library.
	if (!m_IsCoInit) {	// if we haven't already
		m_hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		ABORTCO(ERR_INIT_COM);
		m_IsCoInit = TRUE;
	}
	// create filter graph
	m_hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void **)&m_pGraph);
	ABORTCO(ERR_CREATE_GRAPH);
	// get IMediaControl interface
	m_hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **)&m_pControl);
	ABORTCO(ERR_GET_CONTROL);
	// get IMediaEvent interface
	m_hr = m_pGraph->QueryInterface(IID_IMediaEvent, (void **)&m_pEvent);
	ABORTCO(ERR_GET_EVENT);
	return TRUE;
}

bool CBmpToAvi::Open(const BMPTOAVI_PARMS& Parms, LPCTSTR Path, bool ShowComprDlg)
{
	if (!InitGraph())
		return FALSE;
	// create filters
	m_hr = CoCreateInstance(CLSID_BmpToAvi, NULL, CLSCTX_INPROC_SERVER, 
		IID_IBaseFilter, (void **)&m_pSource);
	ABORTCO(ERR_CREATE_SOURCE);
	m_hr = CoCreateInstance(CLSID_AviMux, NULL, CLSCTX_INPROC_SERVER, 
		IID_IBaseFilter, (void **)&m_pAviMux);
	ABORTCO(ERR_CREATE_MUX);
	m_hr = CoCreateInstance(CLSID_FileWriter, NULL, CLSCTX_INPROC_SERVER, 
		IID_IBaseFilter, (void **)&m_pFileWriter);
	ABORTCO(ERR_CREATE_WRITER);
	// use source's BmpToAvi interface to set parameters
	m_hr = m_pSource->QueryInterface(IID_IBmpToAvi, (void **)&m_pBmpToAvi);
	ABORTCO(ERR_GET_BMPTOAVI);
	m_hr = m_pBmpToAvi->SetParms(Parms);
	ABORTCO(ERR_SET_PARMS);
	// add filters to graph
	m_hr = m_pGraph->AddFilter(m_pSource, L"BmpToAvi");
	ABORTCO(ERR_ADD_SOURCE);
	m_hr = m_pGraph->AddFilter(m_pAviMux, L"AviMux");
	ABORTCO(ERR_ADD_MUX);
	m_hr = m_pGraph->AddFilter(m_pFileWriter, L"FileWriter");
	ABORTCO(ERR_ADD_WRITER);
	IBaseFilter	*pCompr = NULL;
	if (ShowComprDlg) {
		if (!m_ComprDlg.SetFilters(m_pSource, m_pAviMux))
			return FALSE;	// dialog displays error
		if (m_ComprDlg.DoModal() != IDOK)
			return FALSE;	// user canceled
	} else {
		if (m_ComprDlg.HaveState()) {	// if we have a saved compressor state
			if (!m_ComprDlg.CreateCompr())	// create compressor
				return FALSE;
		}
	}
	pCompr = m_ComprDlg.GetCompr();	// copy selected compressor
	// get pins
	m_pSourceOut = CDirShowU::GetPin(m_pSource, PINDIR_OUTPUT);
	ABORTIF(m_pSourceOut == NULL, ERR_GET_PIN);
	m_pMuxInput1 = CDirShowU::GetPin(m_pAviMux, PINDIR_INPUT);
	ABORTIF(m_pMuxInput1 == NULL, ERR_GET_PIN);
	m_pMuxAviOut = CDirShowU::GetPin(m_pAviMux, PINDIR_OUTPUT);
	ABORTIF(m_pMuxAviOut == NULL, ERR_GET_PIN);
	m_pWriterIn = CDirShowU::GetPin(m_pFileWriter, PINDIR_INPUT);
	ABORTIF(m_pWriterIn == NULL, ERR_GET_PIN);
	// connect filters
	if (pCompr != NULL) {	// if we have a compressor
		// add compressor to graph
		m_hr = m_pGraph->AddFilter(pCompr, L"Compr");
		ABORTCO(ERR_ADD_COMPR);
		// get compressor pins
		m_pComprIn = CDirShowU::GetPin(pCompr, PINDIR_INPUT);
		ABORTIF(m_pComprIn == NULL, ERR_GET_PIN);
		m_pComprOut = CDirShowU::GetPin(pCompr, PINDIR_OUTPUT);
		ABORTIF(m_pComprOut == NULL, ERR_GET_PIN);
		// source -> compressor -> mux -> writer
		m_hr = m_pGraph->ConnectDirect(m_pSourceOut, m_pComprIn, NULL);
		ABORTCO(ERR_CONNECT_COMPR);
		m_hr = m_pGraph->ConnectDirect(m_pComprOut, m_pMuxInput1, NULL);
		ABORTCO(ERR_CONNECT_MUX);
		m_hr = m_pGraph->ConnectDirect(m_pMuxAviOut, m_pWriterIn, NULL);
		ABORTCO(ERR_CONNECT_WRITER);
	} else {	// source -> mux -> writer
		m_hr = m_pGraph->ConnectDirect(m_pSourceOut, m_pMuxInput1, NULL);
		ABORTCO(ERR_CONNECT_MUX);
		m_hr = m_pGraph->ConnectDirect(m_pMuxAviOut, m_pWriterIn, NULL);
		ABORTCO(ERR_CONNECT_WRITER);
	}
	// set writer's file name
	IFileSinkFilter	*sink;
	m_hr = m_pFileWriter->QueryInterface(IID_IFileSinkFilter, (void **)&sink);
	ABORTCO(ERR_GET_FILE_SINK);
#ifdef UNICODE
	LPCWCH	WidePath = Path;	// no conversion needed
#else
	WCHAR	WidePath[MAX_PATH + 1];
	int	retc = MultiByteToWideChar(CP_ACP, 0, Path, strlen(Path), WidePath, MAX_PATH);
	WidePath[retc] = 0;	// must terminate string
#endif
	m_hr = sink->SetFileName(WidePath, 0);
	sink->Release();
	ABORTCO(ERR_SET_FILE_NAME);
	// run the graph
	DeleteFile(Path);	// otherwise file won't get smaller
	m_hr = m_pControl->Run();
	ABORTCO(ERR_RUN_GRAPH);
	return TRUE;
}

bool CBmpToAvi::AddFrame(HBITMAP hBitmap)
{
	ABORTIF(m_pBmpToAvi == NULL, ERR_ADD_FRAME);
	m_hr = m_pBmpToAvi->AddFrame(hBitmap);
	ABORTCO(ERR_ADD_FRAME);
	return TRUE;
}

int	CBmpToAvi::GetLastError(HRESULT *hr) const
{
	if (hr != NULL)
		*hr = m_hr;
	return(m_err);
}

void CBmpToAvi::GetLastErrorString(CString& Err, CString& DSErr) const
{
	Err.LoadString(m_ErrStrID[m_err]);
	CDirShowU::GetErrorString(m_hr, DSErr);
}
