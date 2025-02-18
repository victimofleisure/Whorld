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

        DirectShow BMP to AVI converter

*/

#ifndef CBMPTOAVI_INCLUDED
#define CBMPTOAVI_INCLUDED

#include "DirShowU.h"		// DirectShow utilities
#include "BmpToAviCo.h"		// source filter interface
#include "VideoComprDlg.h"

class CBmpToAvi {
public:
// Constants
	#define BMPTOAVIERR(name) ERR_##name,
	enum {	// enumerate our errors, prefixed with ERR_
		#include "BmpToAviErrs.h"
	};

// Construction
	CBmpToAvi();
	~CBmpToAvi();

// Operations
	bool	Open(const BMPTOAVI_PARMS& Parms, LPCTSTR Path, bool ShowComprDlg);
	bool	Close();
	bool	AddFrame(HBITMAP hBitmap);

// Attributes
	bool	IsCompressed() const;
	int		GetLastError(HRESULT *hr = NULL) const;
	void	GetLastErrorString(CString& Err, CString& DSErr) const;
	void	GetComprState(CVideoComprState& State) const;
	void	SetComprState(const CVideoComprState& State);

private:
// Constants
	static const int	m_ErrStrID[];	// map our errors to resource strings

// Member data
	CVideoComprDlg	m_ComprDlg;		// video compression dialog
	IGraphBuilder	*m_pGraph;		// graph builder interface
	IMediaControl	*m_pControl;	// media control interface
	IMediaEvent		*m_pEvent;		// media event interface
	IBaseFilter		*m_pSource;		// source filter's base interface
	IBaseFilter		*m_pAviMux;		// AVI mux filter's base interface
	IBaseFilter		*m_pFileWriter;	// file writer filter's base interface
	IBmpToAvi		*m_pBmpToAvi;	// source filter's specialized interface
	IPin	*m_pSourceOut;	// source output pin
	IPin	*m_pMuxInput1;	// AVI mux input pin
	IPin	*m_pMuxAviOut;	// AVI mux output pin
	IPin	*m_pWriterIn;	// file writer input pin
	IPin	*m_pComprIn;	// compressor input pin
	IPin	*m_pComprOut;	// compressor output pin
	HRESULT m_hr;			// most recent COM result
	int		m_err;			// most recent error
	bool	m_IsCoInit;		// true if we've initialized COM

// Helpers
	bool	InitGraph();
	bool	GetCompressor(IBaseFilter *&pFilter);
};

inline bool CBmpToAvi::IsCompressed() const
{
	return(m_ComprDlg.GetCompr() != NULL);
}

inline void	CBmpToAvi::GetComprState(CVideoComprState& State) const
{
	m_ComprDlg.GetState(State);
}

inline void	CBmpToAvi::SetComprState(const CVideoComprState& State)
{
	m_ComprDlg.SetState(State);
}

#endif
