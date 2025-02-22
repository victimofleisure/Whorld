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

*/

#pragma once

#include "Patch.h"
#include "RenderThread.h"
#include "Oscillator.h"
#include "D2DHelper.h"

class CWhorldThread : protected CPatch, public CRenderThread {
public:
// Construction
	CWhorldThread();

// Attributes
	UINT_PTR	GetRingCount() const;
	UINT_PTR	GetFrameCount() const;
	bool	IsPaused() const;
	DWORD	GetFrameRate() const;
	DPoint	GetOrigin() const;

// Operations
	static bool	WriteCapturedBitmap(ID2D1Bitmap1* pBitmap, LPCTSTR pszImagePath);

protected:
// Types
	struct RING {
		DPOINT	ptShiftDelta;	// additional shear per tick
		double	fRotDelta;		// additional rotation per tick
		double	fHue;			// current hue
		double	fLightness;		// current lightness
		double	fSaturation;	// current saturation
		// mini-ring begins here
		double	fRot;			// rotation for all vertices, in radians
		double	fRadius;		// radius of even vertices, in pixels
		DPOINT	ptScale;		// anisotropic scaling
		DPOINT	ptShift;		// shear, in pixels
		double	fStarRatio;		// ratio of odd radii to even radii
		short	nSides;			// polygon's number of sides
		short	nDrawMode;		// see draw mode enum
		bool	bDelete;		// true if ring should be deleted
		bool	bSkipFill;		// true if ring should be skipped in fill mode
		D2D1_COLOR_F	clrCur;	// current color
		double	fPinwheel;		// additional rotation for odd vertices, in radians
		double	fLineWidth;		// line width, in pixels
		DPOINT	ptOrigin;		// origin in client coords relative to window center
		double	fEvenCurve;		// even vertex curvature, as multiple of radius
		double	fOddCurve;		// odd vertex curvature, as multiple of radius
		double	fEvenShear;		// even vertex curve point asymmetry ratio
		double	fOddShear;		// odd vertex curve point asymmetry ratio
	};
	struct GLOBRING {
		double	fRot;			// rotation for all vertices, in radians
		double	fStarRatio;		// ratio of odd radii to even radii
		double	fPinwheel;		// additional rotation for odd vertices
		DPOINT	ptScale;		// anisotropic scaling
		DPOINT	ptShift;		// shear, in pixels
		double	fEvenCurve;		// even vertex curvature, as multiple of radius
		double	fOddCurve;		// odd vertex curvature, as multiple of radius
		double	fEvenShear;		// even vertex curve point asymmetry ratio
		double	fOddShear;		// odd vertex curve point asymmetry ratio
		double	fLineWidth;		// line width, in pixels
		int		nPolySides;		// number of sides
	};
	struct STATE {
		double	fRingOffset;	// size of gap since last ring, in pixels
		double	fHue;			// hue of new rings, in degrees
		D2D1_COLOR_F	clrRing;	// color of new rings
		D2D1_COLOR_F	clrBkgnd;	// background color
		DPOINT	ptOrigin;		// distance from origin to window center, in DIPs
		double	fHueLoopPos;	// hue loop position, in degrees
		double	fHueLoopLength;	// length of hue loop, in degrees
		double	fHueLoopBase;	// hue loop's base hue, in degrees
	};

// Constants
	enum {
		MAX_SIDES = 50,			// maximum number of sides a ring can have
		MAX_VERTICES = MAX_SIDES * 2,	// rings have 2 vertices per side (even/odd)
		MAX_POINTS = MAX_VERTICES * 3 + 2,	// enough for worst-case curved ring
		DEFAULT_FRAME_RATE = 60,	// in Hertz
	};
	static const double MIN_ASPECT_RATIO;
	static const double MIN_STAR_RATIO;
	static const STATE	m_stateDefault;	// default state
	static const GLOBRING	m_globalRingDefault;	// default global ring

// Data members
	CComPtr<ID2D1SolidColorBrush>	m_pBkgndBrush;	// background brush
	CComPtr<ID2D1SolidColorBrush>	m_pDrawBrush;	// drawing brush
	PARAM_VALS	m_params;	// parameters
	PARAM_VALS	m_globs;	// global parameters
	GLOBRING	m_globRing;	// global ring offsets
	STATE	m_st;			// current state
	CList<RING, RING&>	m_aRing;	// array of rings
	COscillator	m_aOsc[PARAM_COUNT];	// array of oscillators
	PARAM_TABLE	m_aPrevParam;	// previous parameter table
	D2D_POINT_2F	m_aPt[MAX_POINTS * 2];	// enough for two rings
	UINT_PTR	m_nFrameCount;	// frame count
	POSITION	m_posDel;	// if non-null, position of ring after last deletion
	CD2DSizeF	m_szClient;	// size of our window in DIPs
	CKD2DRectF	m_rCanvas;	// ring dies when ALL its vertices are off canvas
	double	m_fNewGrowth;	// new ring growth, computed at start of TimerHook
	int		m_nMaxRings;	// maximum number of rings
	bool	m_bIsPaused;	// if true, we're paused
	bool	m_bCapturing;	// if true, we're capturing an image
	bool	m_bFlushHistory;	// if true, next TimerHook won't interpolate
	bool	m_bCopying;		// if true, add a rotating skew to new ring origins
	int		m_nCopyCount;	// number of copies; must be > 0
	int		m_nCopySpread;	// copy rotation radius, in pixels
	double	m_fCopyTheta;	// copy rotation angle, in radians
	double	m_fCopyDelta;	// copy rotation angle increment, in radians per frame
	DWORD	m_nFrameRate;	// current frame rate in Hertz
	DPoint	m_ptOriginTarget;	// target point for damped origin motion
	CTriggerOscillator	m_oscOrigin;	// trigger oscillator for origin motion
	double	m_fZoomTarget;	// target zoom for damped zooming

// Overrides
	virtual	void	OnError(HRESULT hr, LPCSTR pszSrcFileName, int nLineNum, LPCSTR pszSrcFileDate);
	virtual	bool	CreateUserResources();
	virtual	void	DestroyUserResources();
	virtual bool	OnThreadCreate();
	virtual bool	OnDraw();
	virtual void	OnRenderCommand(const CRenderCmd& cmd);
	virtual void	Log(CString sMsg);

// Command handlers
	void	SetParam(int iParam, double fVal);
	void	SetWaveform(int iParam, int iWave);
	void	SetAmplitude(int iParam, double fAmp);
	void	SetFrequency(int iParam, double fFreq);
	void	SetPulseWidth(int iParam, double fPW);
	void	SetMasterProp(int iProp, double fVal);
	void	SetMainProp(int iProp, const VARIANT_PROP& prop);
	void	SetPatch(CPatch *pPatch);
	bool	SetFrameRate(DWORD nFrameRate);
	void	SetPause(bool bIsPaused);
	void	SingleStep();
	void	SetEmpty();
	void	RandomPhase();
	void	SetZoom(double fZoom, bool bDamping);
	void	SetOrigin(DPoint ptOrigin, bool bDamping);
	bool	CaptureBitmap(UINT nFlags, CD2DSizeU szImage, ID2D1Bitmap1*& pBitmap);
	void	CaptureBitmap(UINT nFlags, SIZE szImage);

// Helpers
	static void		HandleError(HRESULT hr, LPCSTR pszSrcFileName, int nLineNum, LPCSTR pszSrcFileDate);
	static double	Wrap(double Val, double Limit);
	static double	Reflect(double Val, double Limit);
	void	UpdateHue(double DeltaTick);
	void	AddRing();
	void	ResizeCanvas();
	void	OnCopiesChange();
	void	OnTempoChange();
	void	OnOriginMotionChange();
	void	UpdateOrigin();
	void	UpdateZoom();
	void	TimerHook();
	void	DrawRing(
		ID2D1GeometrySink* pSink, D2D1_FIGURE_BEGIN nBeginType, 
		int iFirstPoint, int nPoints, int nVertices, bool bCurved) const;
	void	DrawOutline(ID2D1PathGeometry* pPath, const RING& ring, float fLineWidth) const;
	void	OnMasterPropChange(int iProp);
	void	OnMainPropChange(int iProp);
	void	OnMasterPropChange();
	void	OnMainPropChange();
	DPoint	GetClientSize() const;
	static CString	RenderCommandToString(const CRenderCmd& cmd);
	static double	RandDouble();
};

inline UINT_PTR CWhorldThread::GetRingCount() const
{
	return m_aRing.GetCount();
}

inline UINT_PTR CWhorldThread::GetFrameCount() const
{
	return m_nFrameCount;
}

inline bool CWhorldThread::IsPaused() const
{
	return m_bIsPaused;
}

inline DWORD CWhorldThread::GetFrameRate() const
{
	return m_nFrameRate;
}

inline DPoint CWhorldThread::GetClientSize() const
{
	return DPoint(m_szClient.width, m_szClient.height);
}
