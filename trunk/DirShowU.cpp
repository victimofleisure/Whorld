// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      03feb06	initial version
		01		23nov07	support Unicode
		02		29jan08	in RWVideoInfo, init return value

        DirectShow utilities
 
*/

#include "stdafx.h"
#include "DirShowU.h"

bool CDirShowU::EnumDevs(REFCLSID DevClass, CStringArray& Name, CPtrArray& Moniker)
{
	Name.RemoveAll();
	Moniker.RemoveAll();
	// create system device enumerator.
	ICreateDevEnum	*pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
		return FALSE;
	// create enumerator for specified device category
	IEnumMoniker	*pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(DevClass, &pEnumCat, 0);
	if (hr == S_OK)  {
		// enumerate monikers
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) {
			IPropertyBag *pPropBag;
			HRESULT hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void **)&pPropBag);
			if (SUCCEEDED(hr)) {
				// retrieve filter's friendly name
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr)) {
					// add filter's name and moniker to lists
					Name.Add(varName.bstrVal);
					Moniker.Add(pMoniker);
				}
				VariantClear(&varName);
				pPropBag->Release();
			}
			// caller must release all monikers
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
	return TRUE;
}

IPin *CDirShowU::GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir)
{
	BOOL	   bFound = FALSE;
	IEnumPins  *pEnum = NULL;
	IPin	   *pPin = NULL;
	// enumerate all pins on a filter
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr))
		return NULL;
	// look for a pin that matches the given direction
	while (pEnum->Next(1, &pPin, 0) == S_OK) {
		PIN_DIRECTION PinDirThis;
		pPin->QueryDirection(&PinDirThis);
		if (bFound = (PinDir == PinDirThis))
			break;
		pPin->Release();
	}
	pEnum->Release();
	return (bFound ? pPin : NULL);	
}

void CDirShowU::FreeMediaType(AM_MEDIA_TYPE& mt)
{
	// implemented here to avoid dependence on DirectShow base classes
	if (mt.cbFormat != 0) {
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL) {
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}

void CDirShowU::DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
	// implemented here to avoid dependence on DirectShow base classes
	if (pmt != NULL) {
		FreeMediaType(*pmt);
		CoTaskMemFree(pmt);
	}
}

HRESULT CDirShowU::RWVideoInfo(IBaseFilter *pFilter, VIDEOINFOHEADER& vih, bool Write)
{
	HRESULT	hr = NULL;
	IPin	*pOut = GetPin(pFilter, PINDIR_OUTPUT);
	if (pOut != NULL) {	// if we got filter's output pin
		IAMStreamConfig	*pStreamCfg;
		hr = pOut->QueryInterface(	// get IAMStreamConfig
			IID_IAMStreamConfig, (void **)&pStreamCfg);
		if (SUCCEEDED(hr)) {
			AM_MEDIA_TYPE	*pmt;
			hr = pStreamCfg->GetFormat(&pmt);
			if (SUCCEEDED(hr)) {
				if (pmt->formattype == FORMAT_VideoInfo) {
					VIDEOINFOHEADER	*pvih = (VIDEOINFOHEADER *)pmt->pbFormat;
					if (Write) {
						*pvih = vih;
						hr = pStreamCfg->SetFormat(pmt);
					} else
						vih = *pvih;
				} else
					hr = E_NOTIMPL;	// pin doesn't have video info
				DeleteMediaType(pmt);
			}
			pStreamCfg->Release();
		}
		pOut->Release();
	}
	return hr;
}

HRESULT CDirShowU::GetMediaTypes(IPin *pPin, CPtrArray& MediaType)
{
	MediaType.RemoveAll();
	HRESULT	hr;
	if (pPin != NULL) {
		IEnumMediaTypes	*pEnum;
		hr = pPin->EnumMediaTypes(&pEnum);
		if (SUCCEEDED(hr)) {
			AM_MEDIA_TYPE	*pmt;
			ULONG	cFetched;
			while (SUCCEEDED(pEnum->Next(1, &pmt, &cFetched)) && cFetched == 1)
				MediaType.Add(pmt);	// caller must delete all media types
			pEnum->Release();
		}
	} else
		hr = E_POINTER;
	return hr;
}

void CDirShowU::GetErrorString(HRESULT hr, CString& str)
{
    LPTSTR	p = str.GetBuffer(MAX_ERROR_TEXT_LEN);
    DWORD	retc = AMGetErrorText(hr, p, MAX_ERROR_TEXT_LEN);
	str.ReleaseBuffer();
	if (!retc)
		str.Format(_T("Unknown error 0x8%x."), hr);
}
