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
		03		25feb25	apply zoom to ring origins to match V1 trail behavior
		04		25feb25	add previous curve flag to fix degenerate ring
		05		27feb25	implement hue loop
		06		02mar25	implement global parameters
		07		04mar25	fix points buffer overrun caused by global ring sides
		08		09mar25	set target size in capture bitmap to fix origin shift
		09		09mar25	add export scale to fit
		10		09mar25	fix color shift in legacy snapshots
		11		11mar25	disentangle snapshot zoom
		12		12mar25	remove needless point ctors; implement view-centric zoom
		13		14mar25	add movie recording and playback
		14		15mar25	move queue-related methods to separate class

*/

#include "stdafx.h"
#include "Whorld.h"
#include "WhorldDraw.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "hls.h"
#include "Statistics.h"
#include "SaveObj.h"
#include "Snapshot.h"
#include "MainFrm.h"

#define CHECK(x) { HRESULT hr = x; if (FAILED(hr)) { HandleError(hr, __FILE__, __LINE__, __DATE__); return false; }}
#define DTOF(x) static_cast<float>(x)
#define FTOD(x) static_cast<double>(x)

const double CWhorldDraw::MIN_ASPECT_RATIO = 1e-9;
const double CWhorldDraw::MIN_STAR_RATIO = 1e-2;

static CStatistics stats(60);//@@@

#define SHOW_MOVIE_FRAME_NUMBERS 0	// allowed in Debug build only

CWhorldDraw::CWhorldDraw() 
	: m_oscOrigin(DEFAULT_FRAME_RATE, 1)
{
	SetDefaults();
	m_params = m_paramDefault;
	ZeroMemory(&m_globs, sizeof(m_globs));
	m_globRing = m_globalRingDefault;
	SetParamDefaults(m_aPrevParam);
	ZeroMemory(m_aPt, sizeof(m_aPt));
	m_nFrameCount = 0;
	m_posDel = NULL;
	m_rCanvas.SetRectEmpty();
	m_fNewGrowth = 0;
	m_nMaxRings = INT_MAX;
	m_bIsPaused = false;
	m_bSnapshotMode = false;
	m_bFlushHistory = false;
	m_bCopying = false;
	m_fRingOffset = 0;
	m_fHue = 0;
	m_clrRing = D2D1::ColorF(0);
	m_clrBkgnd = D2D1::ColorF(0);
	m_fHueLoopPos = 0;
	m_fHueLoopBase = 0;
	m_fCopyTheta = 0;
	m_fCopyDelta = 0;
	m_ptOrigin = DPoint(0, 0);
	m_ptOriginTarget = DPoint(0, 0);
	m_fZoom = 1;
	m_fZoomTarget = 1;
	m_nFrameRate = DEFAULT_FRAME_RATE;
	ZeroMemory(&m_dsSnapshot, sizeof(m_dsSnapshot));
	m_bIsMoviePaused = false;
	m_bMovieSingleStep = false;
}

bool CWhorldDraw::CreateUserResources()
{
	// NOTE: objects created here must be released in DestroyUserResources
	CHECK(m_pD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(0, 255, 0, 0.5f), &m_pBkgndBrush));
	CHECK(m_pD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(0, 255, 0, 0.5f), &m_pDrawBrush));
//	m_pD2DDeviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);//@@@ test only!! 40% faster but ugly
	return true;
}

void CWhorldDraw::DestroyUserResources()
{
	m_pBkgndBrush.Release();
	m_pDrawBrush.Release();
}

void CWhorldDraw::OnResize()
{
	m_szTarget = m_pD2DDeviceContext->GetSize();
#if _DEBUG
//	printf("%f %f (%f)\n", m_szTarget.width, m_szTarget.height, m_szTarget.width / m_szTarget.height);//@@@
#endif
}

bool CWhorldDraw::OnThreadCreate()
{
	DWORD	nFrameRate;
	if (!GetDisplayFrequency(nFrameRate))
		return false;
	SetFrameRate(nFrameRate);
	return true;
}

void CWhorldDraw::HandleError(HRESULT hr, LPCSTR pszSrcFileName, int nLineNum, LPCSTR pszSrcFileDate)
{
	CString	sSrcFileName(pszSrcFileName);	// convert to Unicode
	CString	sSrcFileDate(pszSrcFileDate);
	CString	sErrorMsg;
	sErrorMsg.Format(_T("Error %d (0x%x) in %s line %d (%s)"), hr, hr,
		sSrcFileName.GetString(), nLineNum, sSrcFileDate.GetString());
	sErrorMsg += '\n' + FormatSystemError(hr);
	theApp.WriteLogEntry(sErrorMsg);
	AfxMessageBox(sErrorMsg);
}

void CWhorldDraw::OnError(HRESULT hr, LPCSTR pszSrcFileName, int nLineNum, LPCSTR pszSrcFileDate)
{
	HandleError(hr, pszSrcFileName, nLineNum, pszSrcFileDate);	// static method
}

// Drawing

inline double CWhorldDraw::Wrap(double fVal, double fLimit)
{
	double	r = fmod(fVal, fLimit);
	return fVal < 0 ? r + fLimit : r;
}

inline double CWhorldDraw::Reflect(double fVal, double fLimit)
{
	double	m = fLimit * 2;
	double	r = Wrap(fVal, m);
	return r < fLimit ? r : m - r;
}

inline void CWhorldDraw::UpdateHue(double fDeltaTick)
{
	if (m_main.bLoopHue) {	// if looping hue
		if (m_master.fHueSpan) {	// if non-zero loop length
			m_fHueLoopPos += m_params.fColorSpeed * fDeltaTick;
			m_fHue = m_fHueLoopBase -
				Reflect(m_fHueLoopPos, m_master.fHueSpan);
		} else {	// avoid divide by zero
			m_fHue = m_fHueLoopBase;
		}
	} else {	// not looping hue
		m_fHue += m_params.fColorSpeed * fDeltaTick;
	}
	m_fHue = Wrap(m_fHue, 360);
}

void CWhorldDraw::OnHueSpanChange()
{
	if (m_main.bLoopHue) {	// if looping hue
		m_fHueLoopPos = 0;
		m_fHueLoopBase = m_fHue;
	}
}

void CWhorldDraw::ResizeCanvas()
{
	double	fScale = m_master.fCanvasScale * m_fZoom;
	CKD2DRectF	rCanvas(0, 0, DTOF(m_szTarget.width * fScale), DTOF(m_szTarget.height * fScale));
	rCanvas.OffsetRect((m_szTarget.width - rCanvas.Width()) / 2, (m_szTarget.height - rCanvas.Height()) / 2);
	m_rCanvas = rCanvas;
}

void CWhorldDraw::OnCopiesChange()
{
	int	nCount = Round(m_master.fCopies);
	if (nCount > 1) {	// if multiple copies desired
		m_fCopyDelta = M_PI * 2 / nCount;
		m_fCopyTheta = m_fCopyDelta / 2;
		m_bCopying = m_master.fSpread != 0;
	} else {	// one copy only
		m_bCopying = false;
	}
}

inline double CWhorldDraw::RandDouble()
{
	return FTOD(rand()) / RAND_MAX;
}

void CWhorldDraw::OnTempoChange()
{
	if (m_master.fTempo) {	// if non-zero tempo
		m_oscOrigin.SetFreq(m_master.fTempo / 60);
	}
}

void CWhorldDraw::OnOriginMotionChange()
{
	switch (m_main.nOrgMotion) {
	case OM_RANDOM:
		m_oscOrigin.Reset();
		break;
	}
}

void CWhorldDraw::SetOriginTarget(DPoint ptOrigin, bool bDamping)
{
	// origin in render target coordinates (DIPs), NOT normalized
	m_ptOriginTarget = ptOrigin;	// update target
	if (!bDamping || m_bSnapshotMode) {	// if not damping
		m_ptOrigin = ptOrigin;	// go directly to target
	}
}

void CWhorldDraw::UpdateOrigin()
{
	switch (m_main.nOrgMotion) {
	case OM_DRAG:
		{
			POINT	ptCursor;
			if (GetCursorPos(&ptCursor)) {
				ScreenToClient(m_hRenderWnd, &ptCursor);	// convert to client coords
				CRect	rClient;
				GetClientRect(m_hRenderWnd, &rClient);	// get client window rectangle
				DPoint	pt(DPoint(ptCursor) / rClient.Size());	// normalize cursor position
				pt.x = CLAMP(pt.x, 0, 1);	// limit to within client window
				pt.y = CLAMP(pt.y, 0, 1);
				m_ptOriginTarget = (pt - 0.5) * m_szTarget;	// convert to DIPs
			}
		}
		break;
	case OM_RANDOM:
		if (m_master.fTempo) {	// if non-zero tempo
			if (m_oscOrigin.IsTrigger()) {	// if origin oscillator triggered
				DPoint	ptRand(RandDouble(), RandDouble());	// generate normalized random point
				m_ptOriginTarget = (ptRand - 0.5) * m_szTarget;	// convert to DIPs
			}
		}
		break;
	}
	// increment origin by a fraction of its distance from target origin
	m_ptOrigin += (m_ptOriginTarget - m_ptOrigin) * m_master.fDamping;
}

void CWhorldDraw::UpdateZoom()
{
	double	fPrevZoom = m_fZoom;	// save previous zoom
	// increment zoom by a fraction of its distance from target zoom
	m_fZoom += (m_fZoomTarget - m_fZoom) * m_master.fDamping;
	// apply zoom to ring origins to match V1 trail behavior
	double	fDeltaZoom = m_fZoom / fPrevZoom;	// delta is a scaling factor
	DPoint	ptLeadOrg;
	if (m_main.bZoomCenter) {	// if zooming relative to center of view
		ptLeadOrg = DPoint(0, 0);	// zoom rings in view-centric space
		m_ptOrigin *= fDeltaZoom;
		m_ptOriginTarget *= fDeltaZoom;
	} else {	// zoom relative to center of rings
		ptLeadOrg = m_ptOrigin;	// zoom rings in ring-centric space
	}
	POSITION	pos = m_aRing.GetHeadPosition();
	while (pos != NULL) {	// for each ring
		// subtract lead origin, scale by delta zoom, add lead origin
		RING&	rp = m_aRing.GetNext(pos);
		rp.ptOrigin = (DPoint(rp.ptOrigin) - ptLeadOrg) * fDeltaZoom + ptLeadOrg;
	}
}

void CWhorldDraw::OnGlobalsChange()
{
	// set damped global parameter values to their targets
	for (int iGlobal = 0; iGlobal < GLOBAL_COUNT; iGlobal++) {	// for each global parameter
		int	iParam = MapGlobalToParam(iGlobal);	// map from global index to parameter index
		m_globs.a[iParam] = GetParamRow(iParam).fGlobal;	// set damped value from target
	}
}

void CWhorldDraw::UpdateGlobals()
{
	// increment damped global parameter values towards their targets
	for (int iGlobal = 0; iGlobal < GLOBAL_COUNT; iGlobal++) {	// for each global parameter
		int	iParam = MapGlobalToParam(iGlobal);	// map from global index to parameter index
		const PARAM_ROW&	row = GetParamRow(iParam);	// dereference
		// increment parameter by a fraction of its distance from target parameter
		m_globs.a[iParam] += (row.fGlobal - m_globs.a[iParam]) * m_master.fDamping;
	}
}

void CWhorldDraw::RemoveAllRings()
{
	m_aRing.RemoveAll();
	m_posDel = NULL;	// avoid bogus delete
}

void CWhorldDraw::AddRing()
{
	m_params.fRingSpacing = max(m_params.fRingSpacing, 1);
	m_params.fPolySides = CLAMP(m_params.fPolySides, 3, MAX_SIDES);
	m_params.fLightness = CLAMP(m_params.fLightness, 0, 1);
	m_params.fSaturation = CLAMP(m_params.fSaturation, 0, 1);
	double	fR, fG, fB;
	CHLS::hls2rgb(m_fHue, m_params.fLightness, m_params.fSaturation, fR, fG, fB);
	m_clrRing = D2D1::ColorF(DTOF(fR), DTOF(fG), DTOF(fB));
	RING	ring;
	bool	bReverse = m_params.fRingGrowth < 0;
	double	fRingOffset = bReverse ? -m_fRingOffset : m_fRingOffset;
	ring.fRotDelta = m_params.fRotateSpeed;
	ring.fRot = ring.fRotDelta * fRingOffset - ring.fRotDelta * m_fNewGrowth;
	if (m_params.fRingGrowth >= 0)
		ring.fRadius = m_fRingOffset - m_fNewGrowth;
	else {	// if inward growth, start at outermost edge of canvas
		ring.fRadius = (max(m_rCanvas.Width(), m_rCanvas.Height()) / 2 
			/ m_fZoom - m_fRingOffset) - m_fNewGrowth;
	}
	// don't let rings get too flat, or they'll never die
	double	fAspect = max(pow(2, m_params.fAspectRatio), MIN_ASPECT_RATIO);
	ring.ptScale.x = fAspect > 1 ? fAspect : 1;
	ring.ptScale.y = fAspect < 1 ? 1 / fAspect : 1;
	ring.ptShiftDelta.x = sin(m_params.fSkewAngle) * m_params.fSkewRadius;
	ring.ptShiftDelta.y = cos(m_params.fSkewAngle) * m_params.fSkewRadius;
	ring.ptShift.x = ring.ptShiftDelta.x * fRingOffset - ring.ptShiftDelta.x * m_fNewGrowth;
	ring.ptShift.y = ring.ptShiftDelta.y * fRingOffset - ring.ptShiftDelta.y * m_fNewGrowth;
	if (m_bCopying) {
		ring.ptShift.x += sin(m_fCopyTheta) * m_master.fSpread;
		ring.ptShift.y += cos(m_fCopyTheta) * m_master.fSpread;
		m_fCopyTheta += m_fCopyDelta;
	}
	ring.clrCur = m_clrRing;
	ring.nSides = static_cast<short>(Round(m_params.fPolySides));
	ring.nDrawMode = static_cast<short>(m_main.nDrawMode);
	ring.bDelete = false;
	ring.nReserved = 0;
	// don't let stars get too thin, or they'll never die
	double	fRad = max(pow(2, m_params.fStarFactor), MIN_STAR_RATIO);
	// use trig to find distance from origin to middle of a side
	// b is unknown, A is at origin, c is radius, b = cos(A) * c
	ring.fStarRatio = cos(M_PI / ring.nSides) * fRad;
	ring.fPinwheel = M_PI / ring.nSides * m_params.fPinwheel;
	ring.fLineWidth = DTOF(m_params.fLineWidth);
	ring.fHue = m_fHue;
	ring.fLightness = m_params.fLightness;
	ring.fSaturation = m_params.fSaturation;
	ring.ptOrigin = m_ptOrigin;
	ring.fEvenCurve = m_params.fEvenCurve;
	ring.fOddCurve = m_params.fOddCurve / ring.fStarRatio;
	ring.fEvenShear = m_params.fEvenShear;
	ring.fOddShear = m_params.fOddShear;
	POSITION	posStart = bReverse ? m_aRing.GetTailPosition() : m_aRing.GetHeadPosition();
	if (posStart != NULL) {	// if ring list has elements
		const RING& ringStart = bReverse ? m_aRing.GetPrev(posStart) : m_aRing.GetNext(posStart);
		double	fRingSpacing = bReverse ? -m_params.fRingSpacing : m_params.fRingSpacing;
		ring.bSkipFill = fabs(ring.fRadius + fRingSpacing - ringStart.fRadius) > m_params.fRingSpacing * 2;
	} else {	// ring list is empty
		ring.bSkipFill = bReverse;
	}
	// add ring to list
	if (bReverse)
		m_aRing.AddTail(ring);
	else
		m_aRing.AddHead(ring);
	while (m_aRing.GetCount() > m_nMaxRings) {
		if (bReverse)
			m_aRing.RemoveHead();
		else
			m_aRing.RemoveTail();
	}
}

void CWhorldDraw::TimerHook()
{
	int	nRings = max(Round(m_master.fRings), 0);	// keep ring count positive
	m_nMaxRings = nRings >= MAX_RINGS ? INT_MAX : nRings;
	UpdateOrigin();
	UpdateZoom();
	UpdateGlobals();
	double	fSpeed = m_main.bReverse ? -m_master.fSpeed : m_master.fSpeed;
	double	afPrevClock[PARAM_COUNT];
	for (int iParam = 0; iParam < PARAM_COUNT; iParam++) {
		afPrevClock[iParam] = m_aOsc[iParam].GetClock();
		m_aOsc[iParam].SetTimerFreq(m_nFrameRate);
	}
	if (m_bFlushHistory) {	// if flushing history
		m_aPrevParam = m_aParam;	// suppress oscillator interpolation
		for (int iParam = 0; iParam < PARAM_COUNT; iParam++) {	// for each parameter
			// set parameter's oscillator frequency without compensating phase
			m_aOsc[iParam].SetFreqSync(m_aParam.row[iParam].fFreq);
		}
		m_bFlushHistory = false;	// reset flushing history flag
	}
	static const int RG = PARAM_RingGrowth;
	const PARAM_ROW&	rowRG = m_aParam.row[RG];
	m_aOsc[RG].SetClock(afPrevClock[RG] + 1);
	m_fNewGrowth = (rowRG.fVal + m_aOsc[RG].GetVal() * rowRG.fAmp) * fSpeed;
	double	fAbsGrowth = fabs(m_fNewGrowth);
	double	fPrevFracTick = 0;
	while (m_fRingOffset > 0) {
		double	fFracTick = fAbsGrowth ? (1 - m_fRingOffset / fAbsGrowth) : 0;
		for (int iParam = 0; iParam < PARAM_COUNT; iParam++) {
			const PARAM_ROW&	prev = m_aPrevParam.row[iParam];
			const PARAM_ROW&	cur = m_aParam.row[iParam];
			m_aOsc[iParam].SetClock(afPrevClock[iParam] + fFracTick);
			double	fAmp = prev.fAmp + (cur.fAmp - prev.fAmp) * fFracTick;
			m_params.a[iParam] = prev.fVal + (cur.fVal - prev.fVal) * fFracTick
				+ m_aOsc[iParam].GetVal() * fAmp;
		}
		m_params.fRingGrowth *= fSpeed;
		m_params.fColorSpeed *= fSpeed;
		UpdateHue(fFracTick - fPrevFracTick);
		AddRing();
		m_fRingOffset -= m_params.fRingSpacing;
		fPrevFracTick = fFracTick;
	}
	m_fRingOffset += fAbsGrowth;
	m_params.fRingGrowth = m_fNewGrowth;
	m_aOsc[RG].SetClock(afPrevClock[RG] + 1);
	for (int iParam = 1; iParam < PARAM_COUNT; iParam++) {	// skip ring growth; assume it's first row
		const PARAM_ROW&	cur = m_aParam.row[iParam];
		m_aOsc[iParam].SetClock(afPrevClock[iParam] + 1);
		if (cur.fFreq != m_aPrevParam.row[iParam].fFreq)
			m_aOsc[iParam].SetFreq(cur.fFreq);
		m_params.a[iParam] = cur.fVal + m_aOsc[iParam].GetVal() * cur.fAmp;
	}
	m_aPrevParam = m_aParam;
	m_params.fColorSpeed *= fSpeed;
	UpdateHue(1 - fPrevFracTick);
	double	fR, fG, fB;
	m_params.fBkLightness = CLAMP(m_params.fBkLightness, 0, 1);
	m_params.fBkSaturation = CLAMP(m_params.fBkSaturation, 0, 1);
	CHLS::hls2rgb(m_params.fBkHue, m_params.fBkLightness, m_params.fBkSaturation, fR, fG, fB);
	m_clrBkgnd = D2D1::ColorF(DTOF(fR), DTOF(fG), DTOF(fB));
	// update rings
	POSITION	posNext = m_aRing.GetHeadPosition();
	DPoint	ptPrevOrg(m_ptOrigin);
	double	fTrail = 1 - m_master.fTrail;
	bool	bReverse = m_params.fRingGrowth < 0;
	POSITION	posCur = NULL;
	m_posDel = NULL;
	while (posNext != NULL) {	// ring loop
		posCur = posNext;
		// update ring origins
		RING&	ring = m_aRing.GetNext(posNext);
		ring.ptOrigin.x += (ptPrevOrg.x - ring.ptOrigin.x) * fTrail;
		ring.ptOrigin.y += (ptPrevOrg.y - ring.ptOrigin.y) * fTrail;
		ptPrevOrg = ring.ptOrigin;
		// increment cumulative ring properties
		ring.fRadius += m_params.fRingGrowth;
		ring.fRot += ring.fRotDelta * m_params.fRingGrowth;
		ring.ptShift.x += ring.ptShiftDelta.x * m_params.fRingGrowth;
		ring.ptShift.y += ring.ptShiftDelta.y * m_params.fRingGrowth;
		if (bReverse) {	// if reverse
			if (ring.fRadius <= 0) {
				m_aRing.RemoveAt(posCur);
			}
		} else {	// forward
			if (ring.bDelete) {
				m_aRing.RemoveAt(posCur);
				m_posDel = posNext;	// save position after last deletion for cascade
			}
		}
	}
	m_globRing.fRot = m_globs.fRotateSpeed / 5 * 180;
	m_globRing.fStarRatio = max(pow(2, m_globs.fStarFactor), MIN_STAR_RATIO);
	m_globRing.fPinwheel = m_globs.fPinwheel;
	double	r = max(pow(2, m_globs.fAspectRatio), MIN_ASPECT_RATIO);
	m_globRing.ptScale.x = r > 1 ? r : 1;
	m_globRing.ptScale.y = r < 1 ? 1 / r : 1;
	m_globRing.ptShift.x = sin(m_globs.fSkewAngle) * m_globs.fSkewRadius;
	m_globRing.ptShift.y = cos(m_globs.fSkewAngle) * m_globs.fSkewRadius;
	m_globRing.fEvenCurve = m_globs.fEvenCurve;
	m_globRing.fOddCurve = m_globs.fOddCurve / m_globRing.fStarRatio;
	m_globRing.fEvenShear = m_globs.fEvenShear + 1;
	m_globRing.fOddShear = m_globs.fOddShear + 1;
	m_globRing.fLineWidth = DTOF(m_globs.fLineWidth);
	m_globRing.nPolySides = Round(m_globs.fPolySides);
}

#if _DEBUG	// if Debug build, check results

#define OPEN_GEOMETRY_SINK \
	CComPtr<ID2D1PathGeometry>	pPath; \
	CHECK(m_pD2DFactory->CreatePathGeometry(&pPath)); \
	CComPtr<ID2D1GeometrySink> pSink; \
	CHECK(pPath->Open(&pSink));

#define CLOSE_GEOMETRY_SINK \
	CHECK(pSink->Close());

#else	// Release build: don't check results

#define OPEN_GEOMETRY_SINK \
	CComPtr<ID2D1PathGeometry>	pPath; \
	m_pD2DFactory->CreatePathGeometry(&pPath); \
	CComPtr<ID2D1GeometrySink> pSink; \
	pPath->Open(&pSink);

#define CLOSE_GEOMETRY_SINK \
	pSink->Close();

#endif	// _DEBUG

__forceinline void CWhorldDraw::DrawRing(
	ID2D1GeometrySink* pSink, D2D1_FIGURE_BEGIN nBeginType, 
	int iFirstPoint, int nPoints, int nVertices, bool bCurved) const
{
	pSink->BeginFigure(m_aPt[iFirstPoint], nBeginType);
	if (bCurved) {	// if ring is curved
		pSink->AddBeziers(reinterpret_cast<const D2D1_BEZIER_SEGMENT*>(&m_aPt[iFirstPoint + 1]), nVertices);
	} else {	// ring is straight
		pSink->AddLines(&m_aPt[iFirstPoint + 1], nPoints - 1);
	}
	pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
}

__forceinline void CWhorldDraw::DrawOutline(ID2D1PathGeometry* pPath, const RING& ring, float fLineWidth) const
{
	ID2D1Brush	*pBrush;
	if (ring.nDrawMode & DM_OUTLINE) {	// if ring is outlined
		pBrush = m_pBkgndBrush;	// use background color
	} else {	// ring isn't outlined
		// outline anyway to ensure rings overlap completely
		fLineWidth = 1;	// override line width
		pBrush = m_pDrawBrush;	// use fill color
	}
	m_pD2DDeviceContext->DrawGeometry(pPath, pBrush, fLineWidth);
}

bool CWhorldDraw::OnDraw()
{
//	CBenchmark b;//@@@
	ResizeCanvas();	// order matters: AddRing uses m_rCanvas
	if (!m_bIsPaused) {	// if unpaused
		TimerHook();	// do time-based updates
	} else {	// paused
		if (m_movie.IsReading()) {	// if playing movie
			ReadMovieFrame();	// read its next frame
		}
	}
	m_pD2DDeviceContext->Clear(m_clrBkgnd);	// clear to specified color
	m_pBkgndBrush->SetColor(m_clrBkgnd);
	bool	bConvex = m_main.bConvex;
	POSITION	posNext = bConvex ? m_aRing.GetTailPosition() : m_aRing.GetHeadPosition();
	D2D1_COLOR_F	clrPrev;
	bool	bPrevSkipFill = false;
	if (bConvex) {
		POSITION	posTail = posNext;
		if (posTail != NULL) {
			const RING&	ringTail = m_aRing.GetPrev(posTail);
			clrPrev = ringTail.clrCur;
			bPrevSkipFill = ringTail.bSkipFill;
		}
	}
	DPoint	ptWndCenter(m_szTarget.width / 2, m_szTarget.height / 2);
	int		nPoints = 0;			// number of points in current ring
	int		nPrevPoints = 0;		// number of points in previous ring
	int		nPrevVertices = 0;		// number of vertices in previous ring if it's curved
	bool	bPrevCurved = false;	// true if previous ring is curved, else it's straight
	double	fZoom = m_fZoom;
	double	fSnapshotScale = 0;		// prevents uninitialized variable compiler warning
	if (m_bSnapshotMode) {	// if we're displaying a snapshot
		double	fSnapshotHorzScale = FTOD(m_szTarget.width) / m_dsSnapshot.szTarget.width;
		double	fSnapshotVertScale = FTOD(m_szTarget.height) / m_dsSnapshot.szTarget.height;
		fZoom *= min(fSnapshotHorzScale, fSnapshotVertScale);
		fSnapshotScale = fZoom;
		fZoom *= m_dsSnapshot.fZoom;
	}
	while (posNext != NULL) {	// ring loop
		RING&	ring = bConvex ? m_aRing.GetPrev(posNext) : m_aRing.GetNext(posNext);
		double	fLineWidth = ring.fLineWidth + m_globRing.fLineWidth;
		DPoint	ptOrg(ring.ptOrigin);
		if (m_bSnapshotMode) {	// if we're displaying a snapshot
			ptOrg *= fSnapshotScale;	// apply snapshot scaling to ring's origin
			fLineWidth *= fSnapshotScale;	// and line width
			if (m_dsSnapshot.nFlags & CSnapshot::SF_V1) {	// if V1 snapshot
				// V1 shifted filled ring colors by one ring in convex mode, so V2's
				// fix for that must be disabled to display a V1 snapshot correctly.
				clrPrev = ring.clrCur;	// emulate legacy convex mode color behavior
			}
		}
		CKD2DRectF	rBounds(m_rCanvas);
		rBounds.OffsetRect(DTOF(ptOrg.x), DTOF(ptOrg.y));
		int		nSides = ring.nSides + m_globRing.nPolySides;
		nSides = CLAMP(nSides, 1, MAX_SIDES);
		double	fRot = ring.fRot + m_globRing.fRot;
		double	afRot[2] = {fRot, fRot + ring.fPinwheel + M_PI / nSides * m_globRing.fPinwheel};
		double	fRad = ring.fRadius * fZoom;
		double	arRad[2] = {fRad, fRad * ring.fStarRatio * m_globRing.fStarRatio};
		DPoint	ptScale(DPoint(ring.ptScale) * m_globRing.ptScale);
		DPoint	aptRad[2] = {ptScale * arRad[0], ptScale * arRad[1]};
		DPoint	ptShift((DPoint(ring.ptShift) * fZoom + DPoint(m_globRing.ptShift) * fRad) 
			+ ptOrg + ptWndCenter);
		double	fDelta = M_PI / nSides;
		int		nVertices = nSides * 2;	// two vertices per side
		bool	bRingVisible = false;
		double	fCurveLenCW[2];
		fCurveLenCW[0] = ring.fEvenCurve + m_globRing.fEvenCurve;
		fCurveLenCW[1] = ring.fOddCurve + m_globRing.fOddCurve;
		bool	bCurved = fCurveLenCW[0] != 0 || fCurveLenCW[1] != 0;
		if (bCurved) {	// if ring is curved
			nPoints = nVertices * 3 + 1;	// three per Bezier plus one for start point
		} else {	// ring is straight
			nPoints = nVertices + 1;	// one extra for end point, same as start point
		}
		if (ring.nDrawMode & DM_FILL) {	// if ring is filled
			memcpy(m_aPt + nPoints, m_aPt, nPrevPoints * sizeof(D2D_POINT_2F));
		}
		if (bCurved) {	// if ring is curved
			double	fCurveLenCCW[2];
			fCurveLenCCW[0] = fCurveLenCW[0] * (ring.fEvenShear + m_globRing.fEvenShear);
			fCurveLenCCW[1] = fCurveLenCW[1] * (ring.fOddShear + m_globRing.fOddShear);
			// current ring's final control point clobbers previous ring's first point
			D2D_POINT_2F	ptPrevRingStart = m_aPt[nPoints];	// so make a backup
			D2D_POINT_2F	*pPt = &m_aPt[2];	// first two points are set after loop
			for (int iVert = 0; iVert < nVertices; iVert++) {	// innermost loop
				BOOL	bOdd = iVert & 1;
				double	fTheta = fDelta * iVert + afRot[bOdd];
				DPoint	pt;	// compute curve points from real vertex
				pt.x = sin(fTheta) * aptRad[bOdd].x + ptShift.x;
				pt.y = cos(fTheta) * aptRad[bOdd].y + ptShift.y;
				D2D1_POINT_2F	ptStart = {DTOF(pt.x), DTOF(pt.y)};	// single precision
				if (rBounds.PtInRect(ptStart))
					bRingVisible = true;
				DPoint	ptVec(pt - ptShift);	// vector from vertex to origin
				double	rLen = fCurveLenCW[bOdd];	// get clockwise curve vector length
				D2D1_POINT_2F 	ptCtrl2 =
					{DTOF(pt.x - ptVec.y * rLen), DTOF(pt.y + ptVec.x * rLen)};
				*pPt++ = ptCtrl2;	// previous segment's second control point
				*pPt++ = ptStart;	// current segment's start point
				rLen = fCurveLenCCW[bOdd];	// get counter-clockwise curve vector length
				D2D1_POINT_2F	ptCtrl1 =
					{DTOF(pt.x + ptVec.y * rLen), DTOF(pt.y - ptVec.x * rLen)};
				*pPt++ = ptCtrl1;	// current segment's first control point
			}
			m_aPt[0] = m_aPt[nPoints - 1];	// set start point
			m_aPt[1] = m_aPt[nPoints];	// set first segment's first control point
			m_aPt[nPoints] = ptPrevRingStart;	// restore previous ring's start point
		} else {	// ring is straight
			for (int iVert = 0; iVert < nVertices; iVert++) {	// innermost loop
				int	bOdd = iVert & 1;
				double	fTheta = fDelta * iVert + afRot[bOdd];
				D2D_POINT_2F	pt;
				pt.x = DTOF(sin(fTheta) * aptRad[bOdd].x + ptShift.x);
				pt.y = DTOF(cos(fTheta) * aptRad[bOdd].y + ptShift.y);
				m_aPt[iVert] = pt;
				if (rBounds.PtInRect(pt))
					bRingVisible = true;
			}
			m_aPt[nPoints - 1] = m_aPt[0];	// close figure
		}
		if (!m_bIsPaused)	// if unpaused
			ring.bDelete = !bRingVisible;	// mark invisible ring for deletion
		if (ring.nDrawMode & DM_FILL) {	// if ring is filled
			if (!ring.bSkipFill && !bPrevSkipFill) {	// if not skipping ring
				if (!bConvex || nPrevPoints) {	// if concave or previous ring is valid
					OPEN_GEOMETRY_SINK;
					DrawRing(pSink, D2D1_FIGURE_BEGIN_FILLED, 0, nPoints, nVertices, bCurved);
					if (nPrevPoints) {	// if previous ring is valid
						DrawRing(pSink, D2D1_FIGURE_BEGIN_FILLED, nPoints, nPrevPoints, nPrevVertices, bPrevCurved);
					}
					CLOSE_GEOMETRY_SINK;
					m_pDrawBrush->SetColor(bConvex ? clrPrev : ring.clrCur);
					m_pD2DDeviceContext->FillGeometry(pPath, m_pDrawBrush);
					DrawOutline(pPath, ring, DTOF(fLineWidth));
				}
				if (bConvex && posNext == NULL) {	// if convex and last ring, draw bullseye
					OPEN_GEOMETRY_SINK;
					DrawRing(pSink, D2D1_FIGURE_BEGIN_FILLED, 0, nPoints, nVertices, bCurved);
					CLOSE_GEOMETRY_SINK;
					m_pDrawBrush->SetColor(ring.clrCur);
					m_pD2DDeviceContext->FillGeometry(pPath, m_pDrawBrush);
					DrawOutline(pPath, ring, DTOF(fLineWidth));
				}
			}
		} else {	// ring is line
			OPEN_GEOMETRY_SINK;
			DrawRing(pSink, D2D1_FIGURE_BEGIN_HOLLOW, 0, nPoints, nVertices, bCurved);
			CLOSE_GEOMETRY_SINK;
			m_pDrawBrush->SetColor(ring.clrCur);
			m_pD2DDeviceContext->DrawGeometry(pPath, m_pDrawBrush, DTOF(fLineWidth));
		}
		nPrevPoints = nPoints;
		nPrevVertices = nVertices;
		clrPrev = ring.clrCur;
		bPrevSkipFill = ring.bSkipFill;
		bPrevCurved = bCurved;
	}
	if (!m_bIsPaused) {	// if unpaused
		if (m_posDel != NULL) {  // if delete requested
			RING&	ring = m_aRing.GetNext(m_posDel);
			ring.bDelete = true;	// cascade delete
		}
		if (m_movie.IsWriting()) {
			if (!m_movie.Write(GetSnapshot())) {
				OnMovieError();
			}
		}
	} else {	// paused
		// if we're displaying a snapshot and it should be letterboxed
		if (m_bSnapshotMode && theApp.m_options.m_Snapshot_bLetterbox) {
			DrawSnapshotLetterbox();	// sets background brush color
		}
		// if we're playing a movie and frame numbers are desired
		if (m_movie.IsReading() && SHOW_MOVIE_FRAME_NUMBERS) {
			DrawMovieFrameNumber();	// sets foreground brush color
		}
	}
	m_nFrameCount++;
//	stats.Print(b.Elapsed());//@@@
	return true;
}

void CWhorldDraw::DrawSnapshotLetterbox()
{
	const float fMinBarEpsilon = 0.5;	// half a device-independent pixel
	m_pBkgndBrush->SetColor(D2D1::ColorF(0));	// set background color to black
	double	fSnapshotHorzScale = FTOD(m_szTarget.width) / m_dsSnapshot.szTarget.width;
	double	fSnapshotVertScale = FTOD(m_szTarget.height) / m_dsSnapshot.szTarget.height;
	if (fSnapshotHorzScale > fSnapshotVertScale) {	// if target is too wide for snapshot
		// draw vertical bars left and right of snapshot
		float	fBarWidth = DTOF((m_szTarget.width - (m_dsSnapshot.szTarget.width * fSnapshotVertScale)) / 2);
		if (fBarWidth < fMinBarEpsilon)	// if width is less than epsilon
			return;	// don't draw infinitesimal bars
		m_pD2DDeviceContext->FillRectangle(CD2DRectF(0, 0, fBarWidth, m_szTarget.height), m_pBkgndBrush);
		m_pD2DDeviceContext->FillRectangle(
			CD2DRectF(m_szTarget.width - fBarWidth, 0, m_szTarget.width, m_szTarget.height), m_pBkgndBrush);
	} else {	// if target is too narrow for snapshot
		// draw horizontal bars above and below snapshot
		float	fBarHeight = DTOF((m_szTarget.height - (m_dsSnapshot.szTarget.height * fSnapshotHorzScale)) / 2);
		if (fBarHeight < fMinBarEpsilon)	// if height is less than epsilon
			return;	// don't draw infinitesimal bars
		m_pD2DDeviceContext->FillRectangle(CD2DRectF(0, 0, m_szTarget.width, fBarHeight), m_pBkgndBrush);
		m_pD2DDeviceContext->FillRectangle(
			CD2DRectF(0, m_szTarget.height - fBarHeight, m_szTarget.width, m_szTarget.height), m_pBkgndBrush);
	}
}

bool CWhorldDraw::CaptureBitmap(UINT nFlags, CD2DSizeU szImage, ID2D1Bitmap1** pBitmap)
{
	ASSERT(pBitmap != NULL);
	*pBitmap = NULL;	// failsafe: clear destination bitmap pointer first
	if (nFlags & EF_USE_VIEW_SIZE) {	// if using view size as image size
		szImage.width = Round(m_szTarget.width);	// override specified image size
		szImage.height = Round(m_szTarget.height);
	}
	//
	// 1) Create the capture bitmap, which can be a GPU target but isn't readable by the CPU.
	//
	D2D1_BITMAP_PROPERTIES1 propsCapture = {
		{DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED}, 0, 0,
		D2D1_BITMAP_OPTIONS_TARGET,	// target option is incompatible with CPU read
	};
	CComPtr<ID2D1Bitmap1>	pCaptureBitmap;
	CHECK(m_pD2DDeviceContext->CreateBitmap(szImage, NULL, 0, &propsCapture, &pCaptureBitmap));
	//
	// 2) Draw into the capture bitmap.
	//
	m_pD2DDeviceContext->SetTarget(pCaptureBitmap);	// make capture bitmap the render target
	m_pD2DDeviceContext->BeginDraw();	// start drawing
	{
		CSaveObj<bool>	savePaused(m_bIsPaused, true);	// save and set pause state
		CSaveObj<double>	saveZoom(m_fZoom);	// save zoom
		CSaveObj<CD2DSizeF>	saveTargetSize(m_szTarget);	// save target size
		UINT	nScaleType = nFlags & EF_SCALE_FIT_BOTH;
		if (nScaleType) {	// if scaling
			double	fHorzScale = FTOD(szImage.width) / max(m_szTarget.width, 1);
			double	fVertScale = FTOD(szImage.height) / max(m_szTarget.height, 1);
			double	fScale;
			switch (nScaleType) {
			case EF_SCALE_FIT_WIDTH:
				fScale = fHorzScale;	// fit width only
				break;
			case EF_SCALE_FIT_HEIGHT:
				fScale = fVertScale;	// fit height only
				break;
			default:	// fit both width and height
				fScale = min(fHorzScale, fVertScale);
			}
			m_fZoom *= fScale;	// apply scale to zoom
		}
		m_szTarget = CD2DSizeF(szImage);	// set target size to image size
		OnDraw();	// draw as usual; geometry is frozen because we're paused
		// state is restored automatically when CSaveObj instances goes out of scope
	}
	m_pD2DDeviceContext->EndDraw();	// end drawing
	m_pD2DDeviceContext->SetTarget(m_pTargetBitmap);	// restore the swap chain render target
	//
	// 3) Create the CPU-readable bitmap.
	//
	D2D1_BITMAP_PROPERTIES1 propsReadable = {
		{DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED}, 0, 0,
		D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
	};
	CComPtr<ID2D1Bitmap1> pReadableBitmap;
	CHECK(m_pD2DDeviceContext->CreateBitmap(szImage, NULL, 0, &propsReadable, &pReadableBitmap));
	//
	// 4) Copy from the capture bitmap to the readable bitmap.
	//
	CHECK(pReadableBitmap->CopyFromBitmap(NULL, pCaptureBitmap, NULL));
	//
	// 5) Return readable bitmap to caller for mapping and writing.
	//
	*pBitmap = pReadableBitmap.Detach();	// detach bitmap from its smart pointer
	return true;
}

bool CWhorldDraw::WriteCapturedBitmap(ID2D1Bitmap1* pBitmap, LPCTSTR pszImagePath)
{
	// This method finishes capturing a bitmap and writes the image to a file.
	// It's called by the main thread in response to the message posted above.
	// For thread safety, this method is static and can't access instance data.
	// The mapping to CPU memory is done here because it's relatively slow.
	//
	ASSERT(pBitmap != NULL);	// bitmap pointer can't be null
	ASSERT(pszImagePath != NULL);	// image path can't be null either
	//
	// 1) Map the bitmap into CPU memory.
	//
	D2D1_MAPPED_RECT mapped = {};
	CHECK(pBitmap->Map(D2D1_MAP_OPTIONS_READ, &mapped));	// map bitmap into CPU memory
	//
	// 2) Encode the bitmap to PNG with WIC.
	//
	CD2DSizeU	sz(pBitmap->GetPixelSize());	// get bitmap size in device-dependent pixels
	CComPtr<IWICImagingFactory> pWICFactory;
	CHECK(CoCreateInstance(CLSID_WICImagingFactory, NULL,	// create WIC factory
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWICFactory)));
	CComPtr<IWICBitmapEncoder> pEncoder;
	CHECK(pWICFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder));	// create encoder
	CComPtr<IWICStream> pStream;
	CHECK(pWICFactory->CreateStream(&pStream));	// create WIC stream
	CHECK(pStream->InitializeFromFilename(pszImagePath, GENERIC_WRITE)); // initialize stream
	CHECK(pEncoder->Initialize(pStream, WICBitmapEncoderNoCache));	// initialize encoder with stream
	CComPtr<IWICBitmapFrameEncode>	pFrame;
	CHECK(pEncoder->CreateNewFrame(&pFrame, NULL));	// create WIC frame
	CHECK(pFrame->Initialize(NULL));	// initialize frame encoder
	CHECK(pFrame->SetSize(sz.width, sz.height));	// set desired image dimensions
	WICPixelFormatGUID formatGUID = GUID_WICPixelFormat32bppBGRA;	// pixel format GUID
	CHECK(pFrame->SetPixelFormat(&formatGUID));	// set desired pixel format
	UINT cbStride = mapped.pitch;	// size of bitmap scanline in bytes
	UINT cbBufferSize = cbStride * sz.height;	// buffer size is pitch times height
	pFrame->WritePixels(sz.height, cbStride, cbBufferSize, mapped.bits);	// encode frame scanlines
	CHECK(pFrame->Commit());	// commit frame to image
	CHECK(pEncoder->Commit());	// commit all image changes and close stream
	CHECK(pBitmap->Unmap());	// unmap bitmap from memory
	return true;
}

CSnapshot* CWhorldDraw::GetSnapshot() const
{
	int	nRings = static_cast<int>(m_aRing.GetCount());
	CSnapshot *pSnapshot = CSnapshot::Alloc(nRings);
	if (pSnapshot == NULL) {	// if allocation failed
		CHECK(E_OUTOFMEMORY);	// Ran out of memory
	}
	GetSnapshotDrawState(nRings, pSnapshot->m_drawState);	// save drawing state
	pSnapshot->m_globRing = m_globRing;	// save global ring data
	// save ring list
	RING	*pRing	= pSnapshot->m_aRing;
	POSITION	posNext = m_aRing.GetHeadPosition();
	while (posNext != NULL) {	// for each ring in list
		*pRing++ = m_aRing.GetNext(posNext);	// copy to snapshot ring array
	}
	return pSnapshot;
}

void CWhorldDraw::SetSnapshot(const CSnapshot* pSnapshot)
{
	ASSERT(pSnapshot != NULL);
	int	nRings = SetSnapshotDrawState(pSnapshot->m_drawState);	// restore drawing state
	m_globRing = pSnapshot->m_globRing;	// restore global ring data
	// restore ring list
	RemoveAllRings();	// empty ring list
	RING*	pRing = const_cast<RING*>(pSnapshot->m_aRing);
	for (int iRing = 0; iRing < nRings; iRing++) {	// for each snapshot ring
		m_aRing.AddTail(*pRing++);	// add ring to list
	}
}

void CWhorldDraw::EnterSnapshotMode()
{
	if (m_pPrevSnapshot == NULL) {	// if not already in snapshot mode
		m_pPrevSnapshot.Attach(GetSnapshot());	// take snapshot of current state
		m_fZoom = 1;	// in snapshot mode, zoom is relative to snapshot's zoom
	}
	m_bIsPaused = true;	// pause display; pointless otherwise
	m_bSnapshotMode = true;
}

void CWhorldDraw::ExitSnapshotMode()
{
	m_bSnapshotMode = false;
	if (m_pPrevSnapshot != NULL) {	// if previous snapshot exists
		// snapshot is deleted when smart pointer goes out of scope
		CAutoPtr<const CSnapshot>	pSnapshot(m_pPrevSnapshot);	// take ownership
		SetSnapshot(pSnapshot);	// restore snapshot
	}
}

inline void CWhorldDraw::GetSnapshotDrawState(int nRings, DRAW_STATE& drawState) const
{
	ASSERT(!m_bSnapshotMode);	// taking a snapshot of a snapshot is not allowed
	drawState.szTarget = m_szTarget;
	drawState.fZoom = m_fZoom;
	drawState.clrBkgnd = m_clrBkgnd;
	drawState.nRings = nRings;
	drawState.bConvex = m_main.bConvex;
	drawState.nReserved = 0;
	drawState.nFlags = 0;
}

inline int CWhorldDraw::SetSnapshotDrawState(const DRAW_STATE& drawState)
{
	m_dsSnapshot = drawState;	// copy entire draw state
	m_clrBkgnd = drawState.clrBkgnd;	// update background color
	m_main.bConvex = drawState.bConvex;	// update convex flag
	return drawState.nRings;	// return ring count
}

void CWhorldDraw::DumpSnapshot() const
{
	CString	sFrameCount;
	sFrameCount.Format(_T("%lld"), m_nFrameCount);
	CString	sPath(_T("DumpSnapshot") + sFrameCount + _T(".txt"));
	CStdioFile	fOut(sPath, CFile::modeWrite | CFile::modeCreate);
	int	nRings = static_cast<int>(m_aRing.GetCount());
	DRAW_STATE	drawState;
	GetSnapshotDrawState(nRings, drawState);
	fOut.WriteString(CSnapshot::FormatState(drawState));
	POSITION	posNext = m_aRing.GetHeadPosition();
	for (int iRing = 0; iRing < nRings; iRing++) {	// for each ring
		const RING&	ring = m_aRing.GetNext(posNext);
		fOut.WriteString(CSnapshot::FormatRing(iRing, ring));
	}
	fOut.WriteString(CSnapshot::FormatGlobRing(m_globRing));
}

bool CWhorldDraw::SetFrameRate(DWORD nFrameRate)
{
	if (!nFrameRate) {
		ASSERT(0);	// zero frame rate is intolerable
		return false;	// avoid divide by zero when calculating period
	}
	m_nFrameRate = nFrameRate;
	m_oscOrigin.SetTimerFreq(nFrameRate);
	return true;
}

bool CWhorldDraw::ReadMovieFrame()
{
	if (m_movie.IsEndOfFile()) {	// if end of movie reached
		return true;	// nothing to do
	}
	// if movie is paused and single step flag isn't set
	if (m_bIsMoviePaused && !m_bMovieSingleStep) {
		return true;	// nothing to do
	}
	m_bMovieSingleStep = false;	// reset single step flag
	CAutoPtr<const CSnapshot>	pSnapshot(m_movie.Read());
	if (pSnapshot == NULL) {	// if read snapshot failed
		OnMovieError();
		return false;
	}
	SetSnapshot(pSnapshot);
	return true;
}

void CWhorldDraw::OnMovieError()
{
	CSnapMovie::ERROR_STATE	errLast;
	m_movie.GetLastErrorState(errLast);
	m_movie.Close();	// close movie to avoid flood of errors
	HandleError(errLast.nError, errLast.pszSrcFileName, errLast.nLineNum, errLast.pszSrcFileDate);
}

bool CWhorldDraw::DrawMovieFrameNumber()
{
#if SHOW_MOVIE_FRAME_NUMBERS
#ifndef _DEBUG
#error	// movie frame numbers are not allowed in Release build
#endif
	#pragma comment(lib, "dwrite.lib")	// link DirectWrite library
	// Ideally we shouldn't create the text format interface every time,
	// but it only takes a few microseconds and this is test code for now.
	CComPtr<IDWriteFactory> pDWriteFactory;
    CHECK(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&pDWriteFactory)));	// create factory
	CComPtr<IDWriteTextFormat>	pWriteTextFormat;
	const float nFontSize = 50;	// in DIPs
	CHECK(pDWriteFactory->CreateTextFormat(_T("Arial"), NULL, DWRITE_FONT_WEIGHT_NORMAL, 
		DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, nFontSize, L"en-us", &pWriteTextFormat));
	m_pDrawBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
	CString	sFrame;
	// minus one to account for movie read incrementing its index after it reads
	sFrame.Format(_T("%lld"), m_movie.GetReadFrameIdx() - 1);
	m_pD2DDeviceContext->DrawText(sFrame, sFrame.GetLength(), pWriteTextFormat, 
		CD2DRectF(10, 0, m_szTarget.width, nFontSize), m_pDrawBrush);
#endif // SHOW_MOVIE_FRAME_NUMBERS
	return true;	// this method is optimized away in Release build
}
