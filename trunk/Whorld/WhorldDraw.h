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
		04		02mar25	implement global parameters
		05		09mar25	add snapshot flags bitmask
		06		11mar25	add get/set snapshot draw state
		07		12mar25	add target size accessor
		08		14mar25	add movie recording and playback
		09		15mar25	move queue-related methods to separate class

*/

#pragma once

#include "Patch.h"
#include "RenderThread.h"
#include "Oscillator.h"
#include "D2DHelper.h"
#include "Snapshot.h"
#include "SnapMovie.h"

class CWhorldDraw : protected CPatch, public CRenderThread {
public:
// Construction
	CWhorldDraw();

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
	DRAW_STATE	m_dsSnapshot;	// snapshot draw state
	CSnapMovie	m_movie;	// snapshot movie for recording and playback
	bool	m_bIsMoviePaused;	// true if movie playback is paused
	bool	m_bMovieSingleStep;	// true to single-step movie while it's paused

// Overrides
	virtual	void	OnError(HRESULT hr, LPCSTR pszSrcFileName, int nLineNum, LPCSTR pszSrcFileDate);
	virtual	bool	CreateUserResources();
	virtual	void	DestroyUserResources();
	virtual void	OnResize();
	virtual bool	OnThreadCreate();
	virtual bool	OnDraw();

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
	CSnapshot*	GetSnapshot() const;
	void	SetSnapshot(const CSnapshot* pSnapshot);
	void	EnterSnapshotMode();
	void	ExitSnapshotMode();
	void	GetSnapshotDrawState(int nRings, DRAW_STATE& drawState) const;
	int		SetSnapshotDrawState(const DRAW_STATE& drawState);
	void	DumpSnapshot() const;
	void	DrawSnapshotLetterbox();
	bool	SetFrameRate(DWORD nFrameRate);
};
