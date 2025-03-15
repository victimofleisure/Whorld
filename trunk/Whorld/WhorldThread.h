// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version
        01      20feb25	add bitmap capture and write
        02      22feb25	add snapshot capture and load
		03		27feb25	add snapshot mode accessor
		04		01mar25	add commands to set origin coords individually
		05		02mar25	implement global parameters
		06		11mar25	add get/set snapshot draw state
		07		12mar25	add target size accessor
		08		14mar25	add movie recording and playback
		09		15mar25	move queue-related methods here

*/

#pragma once

#include "WhorldDraw.h"

class CWhorldThread : public CWhorldDraw {
public:
// Construction
	CWhorldThread();

// Attributes
	UINT_PTR	GetRingCount() const;
	UINT_PTR	GetFrameCount() const;
	DWORD	GetFrameRate() const;
	DPoint	GetOrigin() const;
	D2D1_SIZE_F	GetTargetSize() const;
	LONGLONG	GetMovieFrameCount() const;
	LONGLONG	GetReadFrameIdx() const;
	D2D1_SIZE_F	GetMovieFrameSize() const;
	void	CancelTask(LONG nTaskID);

// Operations
	bool	SetParam(int iParam, int iProp, VARIANT_PROP& prop);
	bool	SetMasterProp(int iProp, double fVal);
	bool	SetMainProp(int iProp, VARIANT_PROP& prop);
	bool	SetPatch(const CPatch& patch);
	bool	SetFrameRate(DWORD nFrameRate);
	bool	SetPause(bool bEnable);
	bool	SingleStep();
	bool	SetEmpty();
	bool	RandomPhase();
	bool	SetZoom(double fZoom, bool bDamping);
	bool	SetOrigin(DPoint ptOrigin, bool bDamping);
	bool	SetOriginX(double fOriginX, bool bDamping);
	bool	SetOriginY(double fOriginY, bool bDamping);
	bool	CaptureBitmap(UINT nFlags, SIZE szImage);
	bool	CaptureSnapshot();
	bool	DisplaySnapshot(const CSnapshot* pSnapshot);
	bool	SetDampedGlobal(int iParam, double fGlobal);
	bool	SetDrawMode(UINT nMask, UINT nVal);
	bool	SetSnapshotSize(SIZE szSnapshot);
	bool	MovieRecord(LPCTSTR pszMoviePath);
	bool	MoviePlay(LPCTSTR pszMoviePath, bool bPaused = false);
	bool	MoviePause(bool bEnable);
	bool	MovieSeek(LONGLONG iFrame);
	bool	MovieExport(const CMovieExportParams& params, LONG& nTaskID);

protected:
// Data members
	LONGLONG	m_nLastPushErrorTime;	// when push command retries last failed
	LONG	m_nNextTaskID;			// next available task ID
	LONG	m_nCancelTaskID;		// ID of task to be canceled

// Overrides
	virtual void	OnRenderCommand(const CRenderCmd& cmd);
	bool	PushCommand(const CRenderCmd& cmd);

// Helpers
	void	OnMasterPropChange(int iProp);
	void	OnMainPropChange(int iProp);
	void	OnMasterPropChange();
	void	OnMainPropChange();
	LONG	GetNextTaskID();
	static CString	RenderCommandToString(const CRenderCmd& cmd);

// Command handlers
	void	OnSetParam(int iParam, double fVal);
	void	OnSetWaveform(int iParam, int iWave);
	void	OnSetAmplitude(int iParam, double fAmp);
	void	OnSetFrequency(int iParam, double fFreq);
	void	OnSetPulseWidth(int iParam, double fPW);
	void	OnSetGlobalParam(int iParam, double fGlobal);
	void	OnSetMasterProp(int iProp, double fVal);
	void	OnSetMainProp(int iProp, const VARIANT_PROP& prop);
	void	OnSetPatch(const CPatch *pPatch);
	bool	OnSetFrameRate(DWORD nFrameRate);
	void	OnSetPause(bool bIsPaused);
	void	OnSingleStep();
	void	OnSetEmpty();
	void	OnRandomPhase();
	void	OnSetZoom(double fZoom, bool bDamping);
	void	OnSetOrigin(DPoint ptOrigin, bool bDamping);
	void	OnSetOriginX(double fOriginX, bool bDamping);
	void	OnSetOriginY(double rOriginY, bool bDamping);
	void	OnCaptureBitmap(UINT nFlags, SIZE szImage);
	void	OnCaptureSnapshot() const;
	bool	OnDisplaySnapshot(const CSnapshot* pSnapshot);
	void	OnSetDampedGlobal(int iParam, double fGlobal);
	void	OnSetDrawMode(UINT nMask, UINT nVal);
	void	OnSetSnapshotSize(SIZE szSnapshot);
	void	OnMovieRecord(LPCTSTR pszMoviePath);
	void	OnMoviePlay(LPCTSTR pszMoviePath, bool bPaused);
	void	OnMoviePause(bool bEnable);
	void	OnMovieSeek(LONGLONG iFrame);
	void	OnMovieExport(const CMovieExportParams* pParams, LONG nTaskID);
};

inline UINT_PTR CWhorldThread::GetRingCount() const
{
	return m_aRing.GetCount();
}

inline UINT_PTR CWhorldThread::GetFrameCount() const
{
	return m_nFrameCount;
}

inline DWORD CWhorldThread::GetFrameRate() const
{
	return m_nFrameRate;
}

inline D2D1_SIZE_F CWhorldThread::GetTargetSize() const
{
	return m_szTarget;	// atomic in x64, but not in x86!
}

inline LONGLONG CWhorldThread::GetMovieFrameCount() const
{
	return m_movie.GetFrameCount();
}

inline LONGLONG CWhorldThread::GetReadFrameIdx() const
{
	return m_movie.GetReadFrameIdx();
}

inline D2D1_SIZE_F CWhorldThread::GetMovieFrameSize() const
{
	return m_movie.GetTargetSize();
}

inline void CWhorldThread::CancelTask(LONG nTaskID)
{
	m_nCancelTaskID = nTaskID;
}
