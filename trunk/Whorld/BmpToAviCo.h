// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      03feb06	initial version

        BMP to AVI source filter interface

*/

#ifndef BMPTOAVICO_INCLUDED
#define BMPTOAVICO_INCLUDED

// {CEAB8F27-1F47-4e61-BB0A-3F3AD386C2B4}
const GUID CLSID_BmpToAvi = {
	0xceab8f27, 0x1f47, 0x4e61, {0xbb, 0x0a, 0x3f, 0x3a, 0xd3, 0x86, 0xc2, 0xb4}
};

// {85858221-3A64-4d0a-9ADF-2F895D624DA3}
const GUID IID_IBmpToAvi = {
	0x85858221, 0x3a64, 0x4d0a, {0x9a, 0xdf, 0x2f, 0x89, 0x5d, 0x62, 0x4d, 0xa3}
};

typedef struct tagBMPTOAVI_PARMS {
	int		Width;		// frame width, in pixels
	int		Height;		// frame height, in pixels
	int		BitCount;	// number of bits per pixel
	float	FrameRate;	// frame rate, in frames per second
} BMPTOAVI_PARMS;

MIDL_INTERFACE("85858221-3A64-4d0a-9ADF-2F895D624DA3")
IBmpToAvi : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE SetParms(const BMPTOAVI_PARMS& Parms) = 0;
	virtual HRESULT STDMETHODCALLTYPE AddFrame(HBITMAP hBitmap) = 0;
};

#endif
