// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      03feb06	initial version

        DirectShow utilities

*/

#ifndef CDIRSHOWU_INCLUDED
#define CDIRSHOWU_INCLUDED

#include "dshow.h"

class CDirShowU {
public:
	static	bool	EnumDevs(REFCLSID DevClass, CStringArray& Name, CPtrArray& Moniker);
	static	IPin	*GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir);
	static	void	FreeMediaType(AM_MEDIA_TYPE& mt);
	static	void	DeleteMediaType(AM_MEDIA_TYPE *pmt);
	static	HRESULT RWVideoInfo(IBaseFilter *pFilter, VIDEOINFOHEADER& vih, bool Write);
	static	HRESULT	GetMediaTypes(IPin *pPin, CPtrArray& MediaType);
	static	void	GetErrorString(HRESULT hr, CString& str);
};

#endif
