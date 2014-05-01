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

#ifndef CVIDEOCOMPRLIST_INCLUDED
#define CVIDEOCOMPRLIST_INCLUDED

#include <afxtempl.h>
#include "DirShowU.h"
#include "VideoComprState.h"

class CVideoComprList : public WObject
{
// Construction
public:
	CVideoComprList();
	~CVideoComprList();

// Types
	typedef struct tagCAPS {	// compressor capabilities
		bool	CanConfig;		// if true, supports Vfw Configure dialog
		bool	CanAbout;		// if true, supports Vfw About dialog
		bool	CanQuality;		// if true, supports quality methods
		bool	CanCrunch;		// if true, supports compressing to a data rate
		bool	CanKeyFrame;	// if true, supports key frame rate methods
		bool	CanBFrame;		// if true, supports P frames per key methods
		bool	CanWindow;		// if true, supports window size methods
		bool	CanConnect;		// if true, compressor can be connected
	} CAPS;

// Attributes
	bool	SetFilters(IBaseFilter *pSource, IBaseFilter *pDest);
	IBaseFilter *GetCompr() const;
	int		GetCount() const;
	LPCTSTR	GetName(int ComprIdx) const;
	int		GetCurSel() const;
	void	GetCaps(CAPS& Caps) const;
	double	GetQuality() const;
	bool	SetQuality(double Quality);
	long	GetDataRate() const;
	bool	SetDataRate(long DataRate);
	long	GetKeyFrameRate() const;
	bool	SetKeyFrameRate(long KeyFrameRate);
	long	GetPFramesPerKey() const;
	bool	SetPFramesPerKey(long PFramesPerKey);
	long	GetWindowSize() const;
	bool	SetWindowSize(long WindowSize);
	HRESULT	GetLastError() const;

// Operations
	bool	CreateList();
	bool	ConnectCompr(int ComprIdx);
	void	DisconnectCompr();
	bool	SelectCompr(int ComprIdx);
	bool	ShowConfigDlg(HWND hWnd);
	bool	ShowAboutDlg(HWND hWnd);
	void	SaveState(CVideoComprState& State) const;
	bool	RestoreState(const CVideoComprState& State);

// Implementation
protected:
// Types
	typedef struct tagINFO {	// information about each compressor
		IMoniker	*pMoniker;	// unique identifier from enumeration
		IBaseFilter	*pCompr;	// compressor's base filter interface
		IAMVfwCompressDialogs	*pVfwDlg;	// Vfw compression dialogs interface
		IAMVideoCompression		*pVidCom;	// compression control interface
		void	*pDlgState;		// Vfw Configure dialog state, or NULL if none
		int		szDlgState;		// size of Vfw Configure dialog state
		double	Quality;		// compression quality, from 0..1
		long	DataRate;		// desired data rate, in bits per second
		long	KeyFrameRate;	// generate a key frame every N frames
		long	PFramesPerKey;	// number of P frames between key frames
		long	WindowSize;		// number of frames to average data rate over
		long	CapsBits;		// compressor's capability bits
		CAPS	Caps;			// compressor's capabilities
	} INFO;

// Member data
	IFilterGraph	*m_pGraph;	// reference to user's filter graph
	IBaseFilter		*m_pSource;	// user's source filter; don't release
	IBaseFilter		*m_pDest;	// user's destination filter; don't release
	IPin	*m_pSourceOut;		// source output pin
	IPin	*m_pComprIn;		// compressor input pin
	IPin	*m_pComprOut;		// compressor output pin
	IPin	*m_pDestIn;			// destination input pin
	CStringArray	m_Name;		// array of user-friendly compressor names
	CArray<INFO, INFO&> m_Info;	// array of information about each compressor
	HRESULT	m_hr;				// most recent COM result
	int		m_CurSel;			// index of currently selected compressor
	bool	m_IsCoInit;			// true if we've initialized COM

// Helpers
	INFO&	GetCurInfo();
	const INFO&	GetCurInfo() const;
	static	HRESULT	SetState(IAMVfwCompressDialogs *pVfwDlg, LPVOID pState, int cbState);
};

inline CVideoComprList::INFO& CVideoComprList::GetCurInfo()
{
	return(m_Info[m_CurSel]);
}

inline const CVideoComprList::INFO& CVideoComprList::GetCurInfo() const
{
	ASSERT(m_CurSel >= 0 && m_CurSel < m_Info.GetSize());
	return(m_Info.GetData()[m_CurSel]);
}

inline IBaseFilter *CVideoComprList::GetCompr() const
{
	return(GetCurInfo().pCompr);
}

inline int CVideoComprList::GetCount() const
{
	return(m_Name.GetSize());
}

inline LPCTSTR CVideoComprList::GetName(int ComprIdx) const
{
	return(m_Name[ComprIdx]);
}

inline int CVideoComprList::GetCurSel() const
{
	return(m_CurSel);
}

inline void CVideoComprList::GetCaps(CAPS& Caps) const
{
	Caps = GetCurInfo().Caps;
}

inline double CVideoComprList::GetQuality() const
{
	return(GetCurInfo().Quality);
}

inline long CVideoComprList::GetDataRate() const
{
	return(GetCurInfo().DataRate);
}

inline long CVideoComprList::GetKeyFrameRate() const
{
	return(GetCurInfo().KeyFrameRate);
}

inline long CVideoComprList::GetPFramesPerKey() const
{
	return(GetCurInfo().PFramesPerKey);
}

inline long CVideoComprList::GetWindowSize() const
{
	return(GetCurInfo().WindowSize);
}

inline HRESULT	CVideoComprList::GetLastError() const
{
	return(m_hr);
}

#endif
