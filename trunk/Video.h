// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06may06	initial version
		01		07jun06	add seek
		02		26jul06	add GetFrameBits
		03		02aug06	add GetSurface
		04		11oct06	add DestroySurface
		05		28oct06	add GetProps
		06		28nov06	add default surface
		07		04dec06	add IsFirstFrame
		08		23nov07	support Unicode

        video clip
 
*/

#ifndef CVIDEO_INCLUDED
#define CVIDEO_INCLUDED

#include "AviToBmp.h"
#include "BackBufDD.h"

class CVideo : public WObject {
public:
// Construction
	CVideo();
	~CVideo();

// Types
	typedef struct tagPROPS {	// video properties
		SIZE	FrameSize;		// size of frame, in pixels
		DWORD	FrameCount;		// total number of frames
		float	FrameRate;		// data rate, in frames per second
		DWORD	BitCount;		// color depth, in bits per pixel
		DWORD	FCCHandler;		// compressor's FourCC code
	} PROPS;

// Attributes
	bool	IsOpen() const;
	SIZE	GetFrameSize() const;
	DWORD	GetFrameCount() const;
	float	GetFrameRate() const;
	DWORD	GetBitCount() const;
	DWORD	GetCurFrame() const;
	LPCTSTR	GetPath() const;
	HRESULT	GetLastError() const;
	LPCTSTR	GetLastErrorString() const;
	LPVOID	GetFrameBits();
	LPDIRECTDRAWSURFACE	GetSurface();
	bool	GetProps(PROPS& Props) const;
	bool	IsFirstFrame() const;

// Operations
	HRESULT	CreateSurface(CBackBufDD& BackBuf);
	void	DestroySurface();
	bool	UpdateSurface();
	bool	Open(LPCTSTR Path);
	void	Close();
	void	TimerHook();
	void	GetDC(HDC *phDC);
	void	ReleaseDC(HDC hDC);
	void	Rewind();
	void	Seek(DWORD Frame);

private:
// Member data
	CAviToBmp	m_AviFile;	// AVI file reader
	CString	m_Path;			// path to current video file
	LPDIRECTDRAWSURFACE	m_Surface;	// video frame's direct draw surface
	LPBITMAPINFOHEADER	m_FrameBmp;	// video frame's bitmap info
	LPBYTE	m_FrameBits;	// pointer to video frame bits
	DWORD	m_FrameIdx;		// index of current video frame
	bool	m_FirstFrame;	// true if initial frame is pending
	HRESULT	m_hr;			// most recent DirectDraw result code
	static	DDSURFACEDESC	m_DefSurf;	// default 1 x 1 32-bit surface
	static	DWORD	m_DefSurfMem;	// default surface's memory
};

inline bool CVideo::IsOpen() const
{
	return(m_FrameBmp != NULL);
}

inline void CVideo::GetDC(HDC *phDC)
{
	m_Surface->GetDC(phDC);
}

inline void CVideo::ReleaseDC(HDC hDC)
{
	m_Surface->ReleaseDC(hDC);
}

inline SIZE CVideo::GetFrameSize() const
{
	SIZE	sz;
	sz.cx = m_FrameBmp->biWidth;
	sz.cy = m_FrameBmp->biHeight;
	return(sz);
}

inline DWORD CVideo::GetCurFrame() const
{
	return(m_FrameIdx);
}

inline DWORD CVideo::GetFrameCount() const
{
	return(m_AviFile.GetFrameCount());
}

inline float CVideo::GetFrameRate() const
{
	return(m_AviFile.GetFrameRate());
}

inline DWORD CVideo::GetBitCount() const
{
	return(m_AviFile.GetBitCount());
}

inline LPCTSTR CVideo::GetPath() const
{
	return(m_Path);
}

inline void CVideo::Rewind()
{
	m_FrameIdx = 0;
}

inline LPVOID CVideo::GetFrameBits()
{
	return(m_FrameBits);
}

inline LPDIRECTDRAWSURFACE CVideo::GetSurface()
{
	return(m_Surface);
}

inline bool CVideo::IsFirstFrame() const
{
	return(m_FirstFrame);
}

#endif
