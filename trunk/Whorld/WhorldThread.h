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
		06		09mar25	add snapshot flags bitmask
		07		10mar25	add get/set draw state

*/

#pragma once

#include "Patch.h"
#include "RenderThread.h"
#include "Oscillator.h"
#include "D2DHelper.h"
#include "Snapshot.h"

class CWhorldThread : protected CPatch, public CRenderThread {
public:
// Construction
	CWhorldThread();

// Attributes
	UINT_PTR	GetRingCount() const;
	UINT_PTR	GetFrameCount() const;
	DWORD	GetFrameRate() const;
	DPoint	GetOrigin() const;

// Commands
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

// Operations
	static bool	WriteCapturedBitmap(ID2D1Bitmap1* pBitmap, LPCTSTR pszImagePath);

protected:
// Constants
	enum {
		MAX_SIDES = 50,			// maximum number of sides a ring can have
		MAX_VERTICES = MAX_SIDES * 2,	// rings have 2 vertices per side (even/odd)
		MAX_POINTS = MAX_VERTICES * 3 + 2,	// enough for worst-case curved ring
		DEFAULT_FRAME_RATE = 60,	// in Hertz
	};
	static const double MIN_ASPECT_RATIO;
	static const double MIN_STAR_RATIO;

// Data members
	CComPtr<ID2D1SolidColorBrush>	m_pBkgndBrush;	// background brush
	CComPtr<ID2D1SolidColorBrush>	m_pDrawBrush;	// drawing brush
	PARAM_VALS	m_params;	// parameters
	PARAM_VALS	m_globs;	// global parameters
	GLOB_RING	m_globRing;	// global ring offsets
	CList<RING, RING&>	m_aRing;	// array of rings
	COscillator	m_aOsc[PARAM_COUNT];	// array of oscillators
	PARAM_TABLE	m_aPrevParam;	// previous parameter table
	D2D_POINT_2F	m_aPt[MAX_POINTS * 2];	// enough for two rings
	UINT_PTR	m_nFrameCount;	// frame count
	POSITION	m_posDel;	// if non-null, position of ring after last deletion
	CD2DSizeF	m_szTarget;	// size of render target in DIPs
	CKD2DRectF	m_rCanvas;	// ring dies when ALL its vertices are off canvas
	double	m_fNewGrowth;	// new ring growth, computed at start of TimerHook
	int		m_nMaxRings;	// maximum number of rings
	bool	m_bIsPaused;	// if true, we're paused
	bool	m_bSnapshotMode;	// if true, we're displaying a snapshot
	bool	m_bFlushHistory;	// if true, next TimerHook won't interpolate oscillators
	bool	m_bCopying;		// if true, add a rotating skew to new ring origins
	double	m_fRingOffset;	// size of gap since last ring, in pixels
	double	m_fHue;			// hue of new rings, in degrees
	D2D1_COLOR_F	m_clrRing;	// color of new rings
	D2D1_COLOR_F	m_clrBkgnd;	// background color
	double	m_fHueLoopPos;	// hue loop position, in degrees
	double	m_fHueLoopBase;	// hue loop's base hue, in degrees
	double	m_fCopyTheta;	// copy rotation angle, in radians
	double	m_fCopyDelta;	// copy rotation angle increment, in radians per frame
	CTriggerOscillator	m_oscOrigin;	// trigger oscillator for origin motion
	DPoint	m_ptOrigin;		// distance from origin to window center, in DIPs
	DPoint	m_ptOriginTarget;	// target point for damped origin motion
	double	m_fZoom;		// current zoom, as a scaling factor
	double	m_fZoomTarget;	// target zoom for damped zooming
	DWORD	m_nFrameRate;	// current frame rate in Hertz
	CAutoPtr<CSnapshot>	m_pPrevSnapshot;	// render state before snapshot mode
	LONGLONG	m_nLastPushErrorTime;	// when push command retries last failed
	BYTE	m_nSnapshotFlags;	// snapshot flags bitmask
	double	m_fSnapshotZoom;	// snapshot zoom, as a scaling factor

// Overrides
	virtual	void	OnError(HRESULT hr, LPCSTR pszSrcFileName, int nLineNum, LPCSTR pszSrcFileDate);
	virtual	bool	CreateUserResources();
	virtual	void	DestroyUserResources();
	virtual void	OnResize();
	virtual bool	OnThreadCreate();
	virtual bool	OnDraw();
	virtual void	OnRenderCommand(const CRenderCmd& cmd);
	bool	PushCommand(const CRenderCmd& cmd);

// Helpers
	static void		HandleError(HRESULT hr, LPCSTR pszSrcFileName, int nLineNum, LPCSTR pszSrcFileDate);
	static double	Wrap(double fVal, double fLimit);
	static double	Reflect(double fVal, double fLimit);
	void	UpdateHue(double fDeltaTick);
	void	OnHueSpanChange();
	void	ResizeCanvas();
	void	OnCopiesChange();
	void	RemoveAllRings();
	void	AddRing();
	static double	RandDouble();
	void	OnTempoChange();
	void	OnOriginMotionChange();
	void	SetOriginTarget(DPoint ptOrigin, bool bDamping);
	void	UpdateOrigin();
	void	UpdateZoom();
	void	OnGlobalsChange();
	void	UpdateGlobals();
	void	TimerHook();
	void	DrawRing(
		ID2D1GeometrySink* pSink, D2D1_FIGURE_BEGIN nBeginType, 
		int iFirstPoint, int nPoints, int nVertices, bool bCurved) const;
	void	DrawOutline(ID2D1PathGeometry* pPath, const RING& ring, float fLineWidth) const;
	bool	CaptureBitmap(UINT nFlags, CD2DSizeU szImage, ID2D1Bitmap1*& pBitmap);
	void	OnMasterPropChange(int iProp);
	void	OnMainPropChange(int iProp);
	void	OnMasterPropChange();
	void	OnMainPropChange();
	DPoint	GetTargetSize() const;
	CSnapshot*	GetSnapshot() const;
	void	SetSnapshot(const CSnapshot* pSnapshot);
	void	ExitSnapshotMode();
	void	GetDrawState(int nRings, DRAW_STATE& drawState) const;
	int		SetDrawState(const DRAW_STATE& drawState);
	void	DumpSnapshot() const;
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
	bool	OnCaptureSnapshot() const;
	bool	OnDisplaySnapshot(const CSnapshot* pSnapshot);
	void	OnSetDampedGlobal(int iParam, double fGlobal);
	void	OnSetDrawMode(UINT nMask, UINT nVal);
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

inline DPoint CWhorldThread::GetTargetSize() const
{
	return DPoint(m_szTarget.width, m_szTarget.height);
}
