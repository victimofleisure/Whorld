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
		06		01mar25	add commands to set origin coords individually
		07		02mar25	implement global parameters
		08		04mar25	fix points buffer overrun caused by global ring sides

*/

#include "stdafx.h"
#include "Whorld.h"
#include "WhorldThread.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "hls.h"
#include "Statistics.h"
#include "SaveObj.h"
#include "Snapshot.h"

#define CHECK(x) { HRESULT hr = x; if (FAILED(hr)) { HandleError(hr, __FILE__, __LINE__, __DATE__); return false; }}
#define DTOF(x) static_cast<float>(x)

#define RENDER_CMD_NATTER 1	// set true to display render commands on console

const double CWhorldThread::MIN_ASPECT_RATIO = 1e-9;
const double CWhorldThread::MIN_STAR_RATIO = 1e-2;

static CStatistics stats(60);//@@@

CWhorldThread::CWhorldThread() 
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
	m_nLastPushErrorTime = 0;
}

bool CWhorldThread::CreateUserResources()
{
	// NOTE: objects created here must be released in DestroyUserResources
	CHECK(m_pD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(0, 255, 0, 0.5f), &m_pBkgndBrush));
	CHECK(m_pD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(0, 255, 0, 0.5f), &m_pDrawBrush));
//	m_pD2DDeviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);//@@@ test only!! 40% faster but ugly
	return true;
}

void CWhorldThread::DestroyUserResources()
{
	m_pBkgndBrush.Release();
	m_pDrawBrush.Release();
}

void CWhorldThread::OnResize()
{
	m_szTarget = m_pD2DDeviceContext->GetSize();
}

bool CWhorldThread::OnThreadCreate()
{
	DWORD	nFrameRate;
	if (!GetDisplayFrequency(nFrameRate))
		return false;
	OnSetFrameRate(nFrameRate);
	return true;
}

void CWhorldThread::HandleError(HRESULT hr, LPCSTR pszSrcFileName, int nLineNum, LPCSTR pszSrcFileDate)
{
	CString	sSrcFileName(pszSrcFileName);	// convert to Unicode
	CString	sSrcFileDate(pszSrcFileDate);
	CString	sMsg;
	sMsg.Format(_T("COM error 0x%x in %s line %d (%s)"), hr, sSrcFileName.GetString(), nLineNum, sSrcFileDate.GetString());
	sMsg += '\n' + FormatSystemError(hr);
	theApp.Log(sMsg);
	AfxMessageBox(sMsg);
}

void CWhorldThread::OnError(HRESULT hr, LPCSTR pszSrcFileName, int nLineNum, LPCSTR pszSrcFileDate)
{
	HandleError(hr, pszSrcFileName, nLineNum, pszSrcFileDate);	// static method
}

void CWhorldThread::Log(CString sMsg)
{
	theApp.Log(sMsg);
}

// Drawing

inline double CWhorldThread::Wrap(double fVal, double fLimit)
{
	double	r = fmod(fVal, fLimit);
	return fVal < 0 ? r + fLimit : r;
}

inline double CWhorldThread::Reflect(double fVal, double fLimit)
{
	double	m = fLimit * 2;
	double	r = Wrap(fVal, m);
	return r < fLimit ? r : m - r;
}

inline void CWhorldThread::UpdateHue(double fDeltaTick)
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

void CWhorldThread::OnHueSpanChange()
{
	if (m_main.bLoopHue) {	// if looping hue
		m_fHueLoopPos = 0;
		m_fHueLoopBase = m_fHue;
	}
}

void CWhorldThread::ResizeCanvas()
{
	double	fScale = m_master.fCanvasScale * m_fZoom;
	CKD2DRectF	rCanvas(0, 0, DTOF(m_szTarget.width * fScale), DTOF(m_szTarget.height * fScale));
	rCanvas.OffsetRect((m_szTarget.width - rCanvas.Width()) / 2, (m_szTarget.height - rCanvas.Height()) / 2);
	m_rCanvas = rCanvas;
}

void CWhorldThread::OnCopiesChange()
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

inline double CWhorldThread::RandDouble()
{
	return static_cast<double>(rand()) / RAND_MAX;
}

void CWhorldThread::OnTempoChange()
{
	if (m_master.fTempo) {	// if non-zero tempo
		m_oscOrigin.SetFreq(m_master.fTempo / 60);
	}
}

void CWhorldThread::OnOriginMotionChange()
{
	switch (m_main.nOrgMotion) {
	case OM_RANDOM:
		m_oscOrigin.Reset();
		break;
	}
}

void CWhorldThread::SetOriginTarget(DPoint ptOrigin, bool bDamping)
{
	// origin in render target coordinates (DIPs), NOT normalized
	m_ptOriginTarget = ptOrigin;	// update target
	if (!bDamping || m_bSnapshotMode) {	// if not damping
		m_ptOrigin = ptOrigin;	// go directly to target
	}
}

void CWhorldThread::UpdateOrigin()
{
	switch (m_main.nOrgMotion) {
	case OM_DRAG:
		{
			POINT	ptCursor;
			if (GetCursorPos(&ptCursor)) {
				ScreenToClient(m_hRenderWnd, &ptCursor);	// convert to client coords
				CRect	rClient;
				GetClientRect(m_hRenderWnd, &rClient);	// get client window rectangle
				DPoint	pt = DPoint(ptCursor) / rClient.Size();	// normalize cursor position
				pt.x = CLAMP(pt.x, 0, 1);	// limit to within client window
				pt.y = CLAMP(pt.y, 0, 1);
				m_ptOriginTarget = (pt - 0.5) * GetTargetSize();	// convert to DIPs
			}
		}
		break;
	case OM_RANDOM:
		if (m_master.fTempo) {	// if non-zero tempo
			if (m_oscOrigin.IsTrigger()) {	// if origin oscillator triggered
				DPoint	ptRand(RandDouble(), RandDouble());	// generate normalized random point
				m_ptOriginTarget = (ptRand - 0.5) * GetTargetSize();	// convert to DIPs
			}
		}
		break;
	}
	// increment origin by a fraction of its distance from target origin
	m_ptOrigin += (m_ptOriginTarget - m_ptOrigin) * m_master.fDamping;
}

void CWhorldThread::UpdateZoom()
{
	double	fPrevZoom = m_fZoom;	// save previous zoom
	// increment zoom by a fraction of its distance from target zoom
	m_fZoom += (m_fZoomTarget - m_fZoom) * m_master.fDamping;
	// apply zoom to ring origins to match V1 trail behavior
	double	fDeltaZoom = m_fZoom / fPrevZoom;	// delta is a scaling factor
	DPoint	ptLeadOrg(m_ptOrigin);
	POSITION	pos = m_aRing.GetHeadPosition();
	while (pos != NULL) {	// for each ring
		// subtract lead origin, scale by delta zoom, add lead origin
		RING&	rp = m_aRing.GetNext(pos);
		rp.ptOrigin = (DPoint(rp.ptOrigin) - ptLeadOrg) * fDeltaZoom + ptLeadOrg;
	}
}

void CWhorldThread::OnGlobalsChange()
{
	// set damped global parameter values to their targets
	for (int iGlobal = 0; iGlobal < GLOBAL_COUNT; iGlobal++) {	// for each global parameter
		int	iParam = MapGlobalToParam(iGlobal);	// map from global index to parameter index
		m_globs.a[iParam] = GetParamRow(iParam).fGlobal;	// set damped value from target
	}
}

void CWhorldThread::UpdateGlobals()
{
	// increment damped global parameter values towards their targets
	for (int iGlobal = 0; iGlobal < GLOBAL_COUNT; iGlobal++) {	// for each global parameter
		int	iParam = MapGlobalToParam(iGlobal);	// map from global index to parameter index
		const PARAM_ROW&	row = GetParamRow(iParam);	// dereference
		// increment parameter by a fraction of its distance from target parameter
		m_globs.a[iParam] += (row.fGlobal - m_globs.a[iParam]) * m_master.fDamping;
	}
}

void CWhorldThread::RemoveAllRings()
{
	m_aRing.RemoveAll();
	m_posDel = NULL;	// avoid bogus delete
}

void CWhorldThread::AddRing()
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
	ring.bDelete = false;
	// don't let stars get too thin, or they'll never die
	double	fRad = max(pow(2, m_params.fStarFactor), MIN_STAR_RATIO);
	// use trig to find distance from origin to middle of a side
	// b is unknown, A is at origin, c is radius, b = cos(A) * c
	ring.fStarRatio = cos(M_PI / ring.nSides) * fRad;
	ring.fPinwheel = M_PI / ring.nSides * m_params.fPinwheel;
	ring.fLineWidth = DTOF(m_params.fLineWidth);
	ring.nDrawMode = static_cast<short>(m_main.nDrawMode);
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

void CWhorldThread::TimerHook()
{
	int	nRings = Round(m_master.fRings);
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

__forceinline void CWhorldThread::DrawRing(
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

__forceinline void CWhorldThread::DrawOutline(ID2D1PathGeometry* pPath, const RING& ring, float fLineWidth) const
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

bool CWhorldThread::OnDraw()
{
//	CBenchmark b;//@@@
	ResizeCanvas();	// order matters: AddRing uses m_rCanvas
	if (!m_bIsPaused) {	// if unpaused
		TimerHook();
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
	while (posNext != NULL) {	// ring loop
		RING&	ring = bConvex ? m_aRing.GetPrev(posNext) : m_aRing.GetNext(posNext);
		double	fLineWidth = ring.fLineWidth + m_globRing.fLineWidth;
		DPoint	ptOrg(ring.ptOrigin);
		if (m_bSnapshotMode) {	// if we're displaying a snapshot
			fLineWidth *= m_fZoom;	// apply special scaling
			ptOrg *= m_fZoom;
		}
		CKD2DRectF	rBounds(m_rCanvas);
		rBounds.OffsetRect(DTOF(ptOrg.x), DTOF(ptOrg.y));
		int		nSides = ring.nSides + m_globRing.nPolySides;
		nSides = CLAMP(nSides, 1, MAX_SIDES);
		double	fRot = ring.fRot + m_globRing.fRot;
		double	afRot[2] = {fRot, fRot + ring.fPinwheel + M_PI / nSides * m_globRing.fPinwheel};
		double	fRad = ring.fRadius * m_fZoom;
		double	arRad[2] = {fRad, fRad * ring.fStarRatio * m_globRing.fStarRatio};
		DPoint	ptScale(DPoint(ring.ptScale) * m_globRing.ptScale);
		DPoint	aptRad[2] = {ptScale * arRad[0], ptScale * arRad[1]};
		DPoint	ptShift((DPoint(ring.ptShift) * m_fZoom + DPoint(m_globRing.ptShift) * fRad) 
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
				CD2DPointF	spt(DTOF(pt.x), DTOF(pt.y));	// single precision
				if (rBounds.PtInRect(spt))
					bRingVisible = true;
				DPoint	ptVec(pt - ptShift);	// vector from vertex to origin
				double	rLen = fCurveLenCW[bOdd];	// get clockwise curve vector length
				// previous segment's second control point
				*pPt++ = CD2DPointF(DTOF(pt.x - ptVec.y * rLen), DTOF(pt.y + ptVec.x * rLen));
				*pPt++ = spt;	// current segment's start point
				rLen = fCurveLenCCW[bOdd];	// get counter-clockwise curve vector length
				// current segment's first control point
				*pPt++ = CD2DPointF(DTOF(pt.x + ptVec.y * rLen), DTOF(pt.y - ptVec.x * rLen));
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
		m_nFrameCount++;
	}
//	stats.Print(b.Elapsed());//@@@
	return true;
}

void CWhorldThread::OnMasterPropChange(int iProp)
{
	switch (iProp) {
	case MASTER_Copies:
	case MASTER_Spread:
		OnCopiesChange();
		break;
	case MASTER_Zoom:
		OnSetZoom(m_master.fZoom, false);
		break;
	case MASTER_Tempo:
		OnTempoChange();
		break;
	case MASTER_HueSpan:
		OnHueSpanChange();
		break;
	}
}

void CWhorldThread::OnMainPropChange(int iProp)
{
	switch (iProp) {
	case MAIN_OrgMotion:
		OnOriginMotionChange();
		break;
	case MAIN_LoopHue:
		OnHueSpanChange();
		break;
	}
}

void CWhorldThread::OnMasterPropChange()
{
	for (int iProp = 0; iProp < MASTER_COUNT; iProp++) {	// for each master property
		OnMasterPropChange(iProp);
	}
}

void CWhorldThread::OnMainPropChange()
{
	for (int iProp = 0; iProp < MAIN_COUNT; iProp++) {	// for each main property
		OnMainPropChange(iProp);
	}
}

bool CWhorldThread::WriteCapturedBitmap(ID2D1Bitmap1* pBitmap, LPCTSTR pszImagePath)
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

CSnapshot* CWhorldThread::GetSnapshot() const
{
	int	nRings = static_cast<int>(m_aRing.GetCount());
	CSnapshot *pSnapshot = CSnapshot::Alloc(nRings);
	if (pSnapshot == NULL) {	// if allocation failed
		CHECK(E_OUTOFMEMORY);	// Ran out of memory
	}
	// save fixed-length members
	pSnapshot->m_state.clrBkgnd = m_clrBkgnd;
	pSnapshot->m_state.fZoom = m_fZoom;
	pSnapshot->m_state.nRings = nRings;
	pSnapshot->m_state.bConvex = m_main.bConvex;
	ZeroMemory(pSnapshot->m_state.baReserved, sizeof(pSnapshot->m_state.baReserved));
	pSnapshot->m_globRing = m_globRing;	// save global ring data
	// save ring list
	RING	*pRing	= pSnapshot->m_aRing;
	POSITION	posNext = m_aRing.GetHeadPosition();
	while (posNext != NULL) {	// for each ring in list
		*pRing++ = m_aRing.GetNext(posNext);	// copy to snapshot ring array
	}
	return pSnapshot;
}

void CWhorldThread::SetSnapshot(const CSnapshot* pSnapshot)
{
	ASSERT(pSnapshot != NULL);
	// restore fixed-length members
	m_clrBkgnd = pSnapshot->m_state.clrBkgnd;
	m_fZoom = pSnapshot->m_state.fZoom;
	int	nRings = pSnapshot->m_state.nRings;
	m_main.bConvex = pSnapshot->m_state.bConvex;
	m_globRing = pSnapshot->m_globRing;
	// restore ring list
	RemoveAllRings();	// empty ring list
	RING*	pRing = const_cast<RING*>(pSnapshot->m_aRing);
	for (int iRing = 0; iRing < nRings; iRing++) {	// for each snapshot ring
		POSITION	pos = m_aRing.AddTail(*pRing++);	// add ring to list
		// ring origins include zoom, but that causes distorted
		// zooming in snapshot mode, so unzoom the ring origin
		RING&	ring = m_aRing.GetNext(pos);
		ring.ptOrigin = DPoint(ring.ptOrigin) / m_fZoom;	// unzoom origin
		ring.fLineWidth /= m_fZoom;	// unzoom line width too
	}
}

void CWhorldThread::ExitSnapshotMode()
{
	m_bSnapshotMode = false;
	if (m_pPrevSnapshot != NULL) {	// if previous snapshot exists
		// snapshot is deleted when smart pointer goes out of scope
		CAutoPtr<CSnapshot>	pSnapshot(m_pPrevSnapshot);	// take ownership
		SetSnapshot(pSnapshot);	// restore snapshot
	}
}

CString	CWhorldThread::RenderCommandToString(const CRenderCmd& cmd)
{
	CString	sRet;	// string returned to caller
	CString	sCmdName(GetRenderCmdName(cmd.m_nCmd));	// get command name
	// try parameter commands first as they occupy a contiguous range
	if (cmd.m_nCmd >= RC_SET_PARAM_FIRST && cmd.m_nCmd <= RC_SET_PARAM_LAST) {
		int	iProp = cmd.m_nCmd - RC_SET_PARAM_FIRST;	// get property index
		sRet = sCmdName 
			+ _T(" '") + GetParamName(cmd.m_nParam) 
			+ _T("' ") + GetParamPropName(iProp)
			+ _T(" = ") + ParamToString(iProp, cmd.m_prop);
	} else {	// not a parameter command
		switch (cmd.m_nCmd) {
		case RC_SET_MASTER:
			sRet = sCmdName 
				+ _T(" '") + GetMasterName(cmd.m_nParam) 
				+ _T("' = ") + MasterToString(cmd.m_nParam, cmd.m_prop);
			break;
		case RC_SET_MAIN:
			sRet = sCmdName 
				+ _T(" '") + GetMainName(cmd.m_nParam) 
				+ _T("' = ") + MainToString(cmd.m_nParam, cmd.m_prop);
			break;
		default:
			switch (cmd.m_nCmd) {
			#define RENDERCMDDEF(name, vartype) case RC_##name: sRet = sCmdName \
				+ ' ' + ValToString(cmd.m_nParam) \
				+ ' ' + ValToString(cmd.m_prop.vartype); \
				break;
			#include "WhorldDef.h"	// generate cases for generic commands
			}
		}
	}
	return sRet;
}

// Commands

bool CWhorldThread::SetParam(int iParam, int iProp, VARIANT_PROP& prop)
{
	CRenderCmd	cmd(RC_SET_PARAM_Val + iProp, iParam);
	cmd.m_prop = prop;
	return PushCommand(cmd);
}

bool CWhorldThread::SetMasterProp(int iProp, double fVal)
{
	return PushCommand(CRenderCmd(RC_SET_MASTER, iProp, fVal));
}

bool CWhorldThread::SetMainProp(int iProp, VARIANT_PROP& prop)
{
	CRenderCmd	cmd(RC_SET_MAIN, iProp);
	cmd.m_prop = prop;
	return PushCommand(cmd);
}

bool CWhorldThread::SetPatch(const CPatch& patch)
{
	CRenderCmd	cmd(RC_SET_PATCH);
	// dynamically allocate a copy of the patch and enqueue a pointer
	// to it; the render thread is responsible for deleting the patch
	CPatch	*pPatch = new CPatch(patch);	// allocate new patch on heap
	cmd.m_prop.byref = pPatch;
	bool	bRetVal = PushCommand(cmd);
	if (!bRetVal) {	// if push command failed
		delete pPatch;	// we're responsible for cleaning up
		return false;	// return failure
	}
	return true;	// patch belongs to render thread now
}

bool CWhorldThread::SetFrameRate(DWORD nFrameRate)
{
	return PushCommand(CRenderCmd(RC_SET_FRAME_RATE, nFrameRate));
}

bool CWhorldThread::SetPause(bool bEnable)
{
	return PushCommand(CRenderCmd(RC_SET_PAUSE, bEnable));
}

bool CWhorldThread::SingleStep()
{
	return PushCommand(CRenderCmd(RC_SINGLE_STEP));
}

bool CWhorldThread::SetEmpty()
{
	return PushCommand(CRenderCmd(RC_SET_EMPTY));
}

bool CWhorldThread::RandomPhase()
{
	return PushCommand(CRenderCmd(RC_RANDOM_PHASE));
}

bool CWhorldThread::SetZoom(double fZoom, bool bDamping)
{
	return PushCommand(CRenderCmd(RC_SET_ZOOM, bDamping, fZoom));
}

bool CWhorldThread::SetOrigin(DPoint ptOrigin, bool bDamping)
{
	CRenderCmd	cmd(RC_SET_ORIGIN, bDamping);
	cmd.m_prop.fltPt = ptOrigin;
	return PushCommand(cmd);
}

bool CWhorldThread::SetOriginX(double fOriginX, bool bDamping)
{
	return PushCommand(CRenderCmd(RC_SET_ORIGIN_X, bDamping, fOriginX));
}

bool CWhorldThread::SetOriginY(double fOriginY, bool bDamping)
{
	return PushCommand(CRenderCmd(RC_SET_ORIGIN_Y, bDamping, fOriginY));
}

bool CWhorldThread::CaptureBitmap(UINT nFlags, SIZE szImage)
{
	CRenderCmd	cmd(RC_CAPTURE_BITMAP, nFlags);
	cmd.m_prop.szVal = szImage;
	return PushCommand(cmd);
}

bool CWhorldThread::CaptureSnapshot()
{
	return PushCommand(CRenderCmd(RC_CAPTURE_SNAPSHOT));
}

bool CWhorldThread::DisplaySnapshot(const CSnapshot* pSnapshot)
{
	CRenderCmd	cmd(RC_DISPLAY_SNAPSHOT);
	cmd.m_prop.byref = const_cast<CSnapshot*>(pSnapshot);
	return PushCommand(cmd);
}

bool CWhorldThread::SetDampedGlobal(int iParam, double fGlobal)
{
	return PushCommand(CRenderCmd(RC_SET_DAMPED_GLOBAL, iParam, fGlobal));
}

bool CWhorldThread::SetDrawMode(UINT nMask, UINT nVal)
{
	return PushCommand(CRenderCmd(RC_SET_DRAW_MODE, nMask, nVal));
}

bool CWhorldThread::PushCommand(const CRenderCmd& cmd)
{
	while (!CRenderThread::PushCommand(cmd)) {	// try to enqueue command
		// render command queue was full
		if (CWhorldApp::IsMainThread()) {	// if we're the user-interface thread
			// give the user a chance to retry the push
			if (AfxMessageBox(IDS_APP_ERR_RENDER_QUEUE_FULL, MB_RETRYCANCEL) != IDRETRY) {
				return false;	// user canceled, so stop retrying
			}
		} else {	// we're a worker thread
			// all times are in milliseconds
			const UINT	nMaxTotalTimeout = 256;	// maximum total duration of retry loop
			const UINT	nRetryBreakTimeout = 5000;	// duration of break from retries
			LONGLONG	nTimeNow = static_cast<LONGLONG>(GetTickCount64());
			// if we're in an error state, taking a break from doing retries
			if (m_nLastPushErrorTime + nRetryBreakTimeout > nTimeNow) {
				return false;	// push command fails immediately, no retries
			}
			// do a limited number of retries, separated by increasing timeouts
			UINT	nTotalTimeout = 0;
			UINT	nTimeoutLen = 0;	// zero means relinquish remainder of time slice
			// while total time spent sleeping remains within limit
			while (nTotalTimeout + nTimeoutLen < nMaxTotalTimeout) {
				Sleep(nTimeoutLen);	// do a timeout of the specified length
				if (CRenderThread::PushCommand(cmd)) {	// try to enqueue command
					// clear error state by resetting time of last error
					InterlockedExchange64(&m_nLastPushErrorTime, 0);
					return true;	// retry succeeded
				}
				nTotalTimeout += nTimeoutLen;	// add timeout to total time slept
				if (nTimeoutLen) {	// if non-zero timeout
					nTimeoutLen <<= 1;	// double timeout (exponential backoff)
				} else {	// zero timeout
					nTimeoutLen = 1;	// start doubling from one
				}
			}
			// retries failed, so take a longer break from doing retries,
			// to avoid blocking the worker thread on every attempted push
			InterlockedExchange64(&m_nLastPushErrorTime, nTimeNow);
			// notify main thread of unrecoverable error
			PostMsgToMainWnd(UWM_THREAD_ERROR_MSG, IDS_APP_ERR_RENDER_QUEUE_FULL);
			return false;	// we are in the retries failed error state
		}
	}
	return true;
}

// Command handlers

void CWhorldThread::OnSetParam(int iParam, double fVal)
{
	GetParamRow(iParam).fVal = fVal;
}

void CWhorldThread::OnSetWaveform(int iParam, int iWave)
{
	GetParamRow(iParam).iWave = iWave;
	m_aOsc[iParam].SetWaveform(iWave);
}

void CWhorldThread::OnSetAmplitude(int iParam, double fAmp)
{
	GetParamRow(iParam).fAmp = fAmp;
}

void CWhorldThread::OnSetFrequency(int iParam, double fFreq)
{
	GetParamRow(iParam).fFreq = fFreq;
	m_aOsc[iParam].SetFreq(fFreq);
}

void CWhorldThread::OnSetPulseWidth(int iParam, double fPW)
{
	GetParamRow(iParam).fPW = fPW;
	m_aOsc[iParam].SetPulseWidth(fPW);
}

void CWhorldThread::OnSetGlobalParam(int iParam, double fGlobal)
{
	GetParamRow(iParam).fGlobal = fGlobal;
	m_globs.a[iParam] = fGlobal;
}

void CWhorldThread::OnSetMasterProp(int iProp, double fVal)
{
	CPatch::SetMasterProp(iProp, fVal);
	OnMasterPropChange(iProp);
}

void CWhorldThread::OnSetMainProp(int iProp, const VARIANT_PROP& prop)
{
	CPatch::SetMainProp(iProp, prop);
	OnMainPropChange(iProp);
}

void CWhorldThread::OnSetPatch(const CPatch *pPatch)
{
	// assume dynamic allocation: recipient is responsible for deletion
	ASSERT(pPatch != NULL);	// patch pointer had better not be null
	CPatch&	patch = *this;	// upcast to patch data base class
	patch = *pPatch;	// copy patch data from buffer to bass class
	delete pPatch;	// delete patch data buffer
	OnMasterPropChange();	// handle master property changes
	OnMainPropChange();	// handle main property changes
	OnGlobalsChange();	// handle global parameter changes
	m_bFlushHistory = true;	// suppress interpolation to avoid glitch
}

bool CWhorldThread::OnSetFrameRate(DWORD nFrameRate)
{
	if (!nFrameRate) {
		ASSERT(0);	// zero frame rate is intolerable
		return false;	// avoid divide by zero when calculating period
	}
	m_nFrameRate = nFrameRate;
	m_oscOrigin.SetTimerFreq(nFrameRate);
	return true;
}

void CWhorldThread::OnSetPause(bool bIsPaused)
{
	m_bIsPaused = bIsPaused;
	if (!bIsPaused) {	// if unpausing
		ExitSnapshotMode();
	}
}

void CWhorldThread::OnSingleStep()
{
	if (m_bIsPaused) {
		TimerHook();
	}
}

void CWhorldThread::OnSetEmpty()
{
	RemoveAllRings();
	m_fRingOffset = 0;	// add a ring ASAP
}

void CWhorldThread::OnRandomPhase()
{
	for (int iParam = 0; iParam < PARAM_COUNT; iParam++) {
		m_aOsc[iParam].SetPhase(RandDouble());
	}
}

void CWhorldThread::OnSetZoom(double fZoom, bool bDamping)
{
	m_fZoomTarget = fZoom;
	if (!bDamping || m_bSnapshotMode) {	// if not damping
		m_fZoom = fZoom;	// go directly to target
	}
}

void CWhorldThread::OnSetOrigin(DPoint ptOrigin, bool bDamping)
{
	SetOriginTarget(DPoint((ptOrigin - 0.5) * GetTargetSize()), bDamping);	// denormalize origin
}

void CWhorldThread::OnSetOriginX(double fOriginX, bool bDamping)
{
	SetOriginTarget(DPoint((fOriginX - 0.5) * GetTargetSize().x,	// denormalize x-coord
		m_ptOriginTarget.y), bDamping);
}

void CWhorldThread::OnSetOriginY(double fOriginY, bool bDamping)
{
	SetOriginTarget(DPoint(m_ptOriginTarget.x, 
		(fOriginY - 0.5) * GetTargetSize().y), bDamping);	// denormalize y-coord
}

DPoint CWhorldThread::GetOrigin() const
{
	if (!m_szTarget.width || !m_szTarget.height) {
		return DPoint(0, 0);	// avoid divide by zero
	}
	return DPoint(m_ptOrigin) / GetTargetSize() + 0.5;
}

bool CWhorldThread::CaptureBitmap(UINT nFlags, CD2DSizeU szImage, ID2D1Bitmap1*& pBitmap)
{
	pBitmap = NULL;	// failsafe: clear destination bitmap pointer first
	if (nFlags & EF_USE_VIEW_SIZE) {	// if using view size as image size
		szImage = CSize(m_szTarget);	// override specified image size
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
		if (nFlags & EF_SCALE_TO_FIT) {	// if scaling to fit
			double	fScaleWidth = szImage.width / m_szTarget.width;
			double	fScaleHeight = szImage.height / m_szTarget.height;
			double	fScaleToFit = min(fScaleWidth, fScaleHeight);
			m_fZoom *= fScaleToFit; 
		}
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
	pBitmap = pReadableBitmap.Detach();	// detach bitmap from its smart pointer
	return true;
}

void CWhorldThread::OnCaptureBitmap(UINT nFlags, SIZE szImage)
{
	ID2D1Bitmap1*	pBitmap;
	CaptureBitmap(nFlags, CD2DSizeU(szImage), pBitmap);
	LPARAM	lParam = reinterpret_cast<LPARAM>(pBitmap);	
	PostMsgToMainWnd(UWM_BITMAP_CAPTURE, 0, lParam);	// post pointer to main window
}

bool CWhorldThread::OnCaptureSnapshot() const
{
	LPARAM	lParam = reinterpret_cast<LPARAM>(GetSnapshot());
	PostMsgToMainWnd(UWM_SNAPSHOT_CAPTURE, 0, lParam);	// post pointer to main window
	return true;
}

bool CWhorldThread::OnDisplaySnapshot(const CSnapshot* pSnapshot)
{
	if (pSnapshot == NULL) {	// if null snapshot pointer
		CHECK(E_INVALIDARG);	// One or more arguments are invalid
	}
	if (m_pPrevSnapshot == NULL) {
		m_pPrevSnapshot.Attach(GetSnapshot());
	}
	SetSnapshot(pSnapshot);
	m_bIsPaused = true;	// pause display; pointless otherwise
	m_bSnapshotMode = true;
	delete pSnapshot;	// assume snapshot was allocated on heap
	return true;
}

void CWhorldThread::OnSetDampedGlobal(int iParam, double fGlobal)
{
	GetParamRow(iParam).fGlobal = fGlobal;	// set target only
}

void CWhorldThread::OnSetDrawMode(UINT nMask, UINT nVal)
{
	m_main.nDrawMode &= ~nMask;	// clear specified bits
	m_main.nDrawMode |= (nVal & nMask);	// set specified bits
}

void CWhorldThread::OnRenderCommand(const CRenderCmd& cmd)
{
#if _DEBUG && RENDER_CMD_NATTER
	_fputts(RenderCommandToString(cmd) + '\n', stdout);
#endif
	// dispatch render command to appropriate handler
	switch (cmd.m_nCmd) {
	case RC_SET_PARAM_Val:
		OnSetParam(cmd.m_nParam, cmd.m_prop.dblVal);
		break;
	case RC_SET_PARAM_Wave:
		OnSetWaveform(cmd.m_nParam, cmd.m_prop.intVal);
		break;
	case RC_SET_PARAM_Amp:
		OnSetAmplitude(cmd.m_nParam, cmd.m_prop.dblVal);
		break;
	case RC_SET_PARAM_Freq:
		OnSetFrequency(cmd.m_nParam, cmd.m_prop.dblVal);
		break;
	case RC_SET_PARAM_PW:
		OnSetPulseWidth(cmd.m_nParam,  cmd.m_prop.dblVal);
		break;
	case RC_SET_PARAM_Global:
		OnSetGlobalParam(cmd.m_nParam, cmd.m_prop.dblVal);
		break;
	case RC_SET_MASTER:
		OnSetMasterProp(cmd.m_nParam, cmd.m_prop.dblVal);
		break;
	case RC_SET_MAIN:
		OnSetMainProp(cmd.m_nParam, cmd.m_prop);
		break;
	case RC_SET_PATCH:
		OnSetPatch(static_cast<CPatch*>(cmd.m_prop.byref));
		break;
	case RC_SET_EMPTY:
		OnSetEmpty();
		break;
	case RC_SET_FRAME_RATE:
		OnSetFrameRate(cmd.m_nParam);
		break;
	case RC_SET_PAUSE:
		OnSetPause(cmd.m_nParam != 0);
		break;
	case RC_SINGLE_STEP:
		OnSingleStep();
		break;
	case RC_RANDOM_PHASE:
		OnRandomPhase();
		break;
	case RC_SET_ZOOM:
		OnSetZoom(cmd.m_prop.dblVal, cmd.m_nParam != 0);
		break;
	case RC_SET_ORIGIN:
		OnSetOrigin(cmd.m_prop.fltPt, cmd.m_nParam != 0);
		break;
	case RC_SET_ORIGIN_X:
		OnSetOriginX(cmd.m_prop.dblVal, cmd.m_nParam != 0);
		break;
	case RC_SET_ORIGIN_Y:
		OnSetOriginY(cmd.m_prop.dblVal, cmd.m_nParam != 0);
		break;
	case RC_CAPTURE_BITMAP:
		OnCaptureBitmap(cmd.m_nParam, cmd.m_prop.szVal);
		break;
	case RC_CAPTURE_SNAPSHOT:
		OnCaptureSnapshot();
		break;
	case RC_DISPLAY_SNAPSHOT:
		OnDisplaySnapshot(static_cast<CSnapshot*>(cmd.m_prop.byref));
		break;
	case RC_SET_DAMPED_GLOBAL:
		OnSetDampedGlobal(cmd.m_nParam, cmd.m_prop.dblVal);
		break;
	case RC_SET_DRAW_MODE:
		OnSetDrawMode(cmd.m_nParam, cmd.m_prop.uintVal);
		break;
	default:
		NODEFAULTCASE;	// missing command case
	};
}
