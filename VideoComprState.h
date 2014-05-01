// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      16aug06	initial version
        01      05mar07	add Read and Write

        video compressor state

*/

#ifndef CVIDEOCOMPRSTATE_INCLUDED
#define CVIDEOCOMPRSTATE_INCLUDED

class CVideoComprState : public CObject {
public:
	DECLARE_SERIAL(CVideoComprState);

// Construction
	CVideoComprState();
	CVideoComprState(const CVideoComprState& st);
	CVideoComprState& operator=(const CVideoComprState& st);

// Types
	typedef struct tagPARMS {
		double	Quality;		// compression quality, from 0..1
		long	DataRate;		// desired data rate, in bits per second
		long	KeyFrameRate;	// generate a key frame every N frames
		long	PFramesPerKey;	// number of P frames between key frames
		long	WindowSize;		// number of frames to average data rate over
	} PARMS;

// Operations
	void	Serialize(CArchive& ar);
	void	Write(CFile& fp);
	void	Read(CFile& fp);
	bool	Write(LPCTSTR Path);
	bool	Read(LPCTSTR Path);

// Public data
	CString		m_Name;			// compressor name
	CByteArray	m_DlgState;		// Vfw Configure dialog state
	PARMS		m_Parms;		// compression parameters

protected:
// Helpers
	void	Copy(const CVideoComprState& st);
};

inline CVideoComprState::CVideoComprState(const CVideoComprState& st)
{
	Copy(st);
}

inline CVideoComprState& CVideoComprState::operator=(const CVideoComprState& st)
{
	Copy(st);
	return(*this);
}

#endif
