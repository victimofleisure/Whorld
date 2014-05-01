// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      03feb06	initial version
        01      05may06	add IsOpen
		02		16may06	remove GetStreamInfo
		03		28oct06	add GetBitCount and GetStreamInfo
		04		23nov07	support Unicode

		AVI file frame-grabber

*/

#ifndef CAVITOBMP_INCLUDED
#define CAVITOBMP_INCLUDED

#include "vfw.h"

class CAviToBmp {
public:
// Construction
	CAviToBmp();
	~CAviToBmp();

// Operations
	bool	Open(LPCTSTR Path);
	void	Close();
	LPBITMAPINFOHEADER	GetFrame(DWORD Frame);

// Attributes
	bool	IsOpen() const;
	DWORD	GetFrameCount() const;
	float	GetFrameRate() const;
	HRESULT	GetLastError() const;
	LPCTSTR	GetLastErrorString() const;
	DWORD	GetBitCount() const;
	bool	GetStreamInfo(AVISTREAMINFO& Info) const;

private:
// Types
	typedef	struct tagERRTRAN {
		HRESULT	Code;
		LPCTSTR	Text;
	} ERRTRAN;

// Constants
	static	const	ERRTRAN	m_ErrTran[];	// map AVIFile error codes to text

// Member data
	PAVIFILE		m_pFile;		// pointer to input file
	PAVISTREAM		m_pStream;		// pointer to input video stream
	LPBITMAPINFO	m_pBmpInfo;		// pointer to input bitmap info
	PGETFRAME		m_pGetFrame;	// pointer to get frame object
	HRESULT	m_hr;			// most recent AVIFile result code
	DWORD	m_FrameCount;	// number of frames in video

// Helpers
	void	Init();
};

inline DWORD CAviToBmp::GetFrameCount() const
{
	return(m_FrameCount);
}

inline HRESULT CAviToBmp::GetLastError() const
{
	return(m_hr);
}

inline bool CAviToBmp::IsOpen() const
{
	return(m_pGetFrame != NULL);
}

inline DWORD CAviToBmp::GetBitCount() const
{
	return(m_pBmpInfo->bmiHeader.biBitCount);
}

#endif
