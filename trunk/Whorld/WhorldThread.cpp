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
		03		27feb25	implement hue loop
		04		01mar25	add commands to set origin coords individually
		05		02mar25	implement global parameters
		06		09mar25	set target size in capture bitmap to fix origin shift
		07		09mar25	add export scale to fit
		08		14mar25	add movie recording and playback
		09		15mar25	move queue-related methods here
		10		25mar25	add random origin command
		11      11apr25	add antialiasing main property

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
#include "MainFrm.h"

#define CHECK(x) { HRESULT hr = x; if (FAILED(hr)) { HandleError(hr, __FILE__, __LINE__, __DATE__); return false; }}

#define RENDER_CMD_NATTER 1	// set true to display render commands on console

CWhorldThread::CWhorldThread() 
{
	m_nLastPushErrorTime = 0;
	m_nTaskItemsDone = 0;
	m_nNextTaskID = 0;
	m_nCancelTaskID = 0;
}

// The following methods provide thread-safe access to rendering functions.
// To be safe, they must not use any base class members except PushCommand.

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

bool CWhorldThread::RandomOrigin()
{
	return PushCommand(CRenderCmd(RC_RANDOM_ORIGIN));
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

bool CWhorldThread::SetSnapshotSize(SIZE szSnapshot)
{
	CRenderCmd	cmd(RC_SET_SNAPSHOT_SIZE);
	cmd.m_prop.szVal = szSnapshot;
	return PushCommand(cmd);
}

bool CWhorldThread::MovieRecord(LPCTSTR pszMoviePath)
{
	CRenderCmd	cmd(RC_MOVIE_RECORD);
	cmd.m_prop.byref = SafeStrDup(pszMoviePath);	// duplicate string
	return PushCommand(cmd);
}

bool CWhorldThread::MoviePlay(LPCTSTR pszMoviePath, bool bPaused)
{
	CRenderCmd	cmd(RC_MOVIE_PLAY, bPaused);
	cmd.m_prop.byref = SafeStrDup(pszMoviePath);	// duplicate string
	return PushCommand(cmd);
}

bool CWhorldThread::MoviePause(bool bEnable)
{
	return PushCommand(CRenderCmd(RC_MOVIE_PAUSE, bEnable));
}

bool CWhorldThread::MovieSeek(LONGLONG iFrame)
{
	return PushCommand(CRenderCmd(RC_MOVIE_SEEK, 0, iFrame));
}

bool CWhorldThread::MovieExport(const CMovieExportParams& params, LONG& nTaskID)
{
	nTaskID = GetNextTaskID();
	CRenderCmd	cmd(RC_MOVIE_EXPORT, nTaskID);
	cmd.m_prop.byref = new CMovieExportParams(params);	// duplicate parameters
	return PushCommand(cmd);
}

// Helper methods

bool CWhorldThread::PushCommand(const CRenderCmd& cmd)
{
	while (!CRenderThread::PushCommand(cmd)) {	// try to enqueue command
		// enqueue failed because render command queue was full
		if (CWhorldApp::IsMainThread()) {	// if we're the user-interface thread
			// if not already handling render queue full error
			if (!theApp.GetMainFrame()->InRenderFullError()) {
				// give the user a chance to retry enqueuing the command
				if (AfxMessageBox(IDS_APP_ERR_RENDER_QUEUE_FULL, MB_RETRYCANCEL) != IDRETRY) {
					return false;	// user canceled, so stop retrying
				}
			}
		} else {	// we're a worker thread
			// all times are in milliseconds
			const UINT	nMaxTotalTimeout = 256;	// maximum total duration of retry loop
			const UINT	nRetryBreakTimeout = 2500;	// duration of break from retries
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
				if (CRenderThread::PushCommand(cmd)) {	// retry enqueuing command
					// success: clear error state by zeroing time of last error
					InterlockedExchange64(&m_nLastPushErrorTime, 0);
					return true;	// retry succeeded
				}
				// retry failed: increase timeout and try again if permitted
				nTotalTimeout += nTimeoutLen;	// add timeout to total time slept
				if (nTimeoutLen) {	// if non-zero timeout
					nTimeoutLen <<= 1;	// double timeout (exponential backoff)
				} else {	// zero timeout
					nTimeoutLen = 1;	// start doubling from one
				}
			}
			// all retries have failed, so take a break from doing retries,
			// to avoid blocking the worker thread on every attempted push
			InterlockedExchange64(&m_nLastPushErrorTime, nTimeNow);
			// notify main thread that an unrecoverable error occurred
			PostMsgToMainWnd(UWM_RENDER_QUEUE_FULL);
			return false;	// we are in the retries failed error state
		}
	}
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
	case MAIN_Antialiasing:
		SetAntialiasing(m_main.bAntialiasing);
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

LONG CWhorldThread::GetNextTaskID()
{
	return InterlockedIncrement(&m_nNextTaskID);
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

inline CWhorldThread::CSetAntialiasing::CSetAntialiasing(bool bEnable)
{
	m_bPrevEnable = theApp.m_thrRender.m_main.bAntialiasing;	// save state
	theApp.m_thrRender.SetAntialiasing(bEnable);
}

inline CWhorldThread::CSetAntialiasing::~CSetAntialiasing()
{
	theApp.m_thrRender.SetAntialiasing(m_bPrevEnable);	// restore state
}

// Command handlers: these run in the render thread's context

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
	return CWhorldDraw::SetFrameRate(nFrameRate);
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
		if (m_movie.IsReading()) {
			if (m_bIsMoviePaused) {
				m_bMovieSingleStep = true;
			}
		} else {
			TimerHook();
		}
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

void CWhorldThread::OnRandomOrigin()
{
	DPoint	ptRand(RandDouble(), RandDouble());	// generate normalized random point
	m_ptOriginTarget = (ptRand - 0.5) * m_szTarget;	// convert to DIPs
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
	SetOriginTarget(DPoint((ptOrigin - 0.5) * m_szTarget), bDamping);	// denormalize origin
}

void CWhorldThread::OnSetOriginX(double fOriginX, bool bDamping)
{
	SetOriginTarget(DPoint((fOriginX - 0.5) * m_szTarget.width,	// denormalize x-coord
		m_ptOriginTarget.y), bDamping);
}

void CWhorldThread::OnSetOriginY(double fOriginY, bool bDamping)
{
	SetOriginTarget(DPoint(m_ptOriginTarget.x, 
		(fOriginY - 0.5) * m_szTarget.height), bDamping);	// denormalize y-coord
}

DPoint CWhorldThread::GetOrigin() const
{
	if (!m_szTarget.width || !m_szTarget.height) {
		return DPoint(0, 0);	// avoid divide by zero
	}
	return DPoint(m_ptOrigin) / m_szTarget + 0.5;
}

void CWhorldThread::OnCaptureBitmap(UINT nFlags, SIZE szImage)
{
	CSetAntialiasing	setAA(true);	// enable antialiasing; dtor restores state
	ID2D1Bitmap1*	pBitmap;
	CWhorldDraw::CaptureBitmap(nFlags, CD2DSizeU(szImage), &pBitmap);
	LPARAM	lParam = reinterpret_cast<LPARAM>(pBitmap);	
	PostMsgToMainWnd(UWM_BITMAP_CAPTURE, 0, lParam);	// post pointer to main window
}

void CWhorldThread::OnCaptureSnapshot() const
{
	CSnapshot	*pSnapshot = GetSnapshot();
	LPARAM	lParam = reinterpret_cast<LPARAM>(pSnapshot);
	PostMsgToMainWnd(UWM_SNAPSHOT_CAPTURE, 0, lParam);	// post pointer to main window
}

bool CWhorldThread::OnDisplaySnapshot(const CSnapshot* pSnapshot)
{
	if (pSnapshot == NULL) {	// if null snapshot pointer
		CHECK(E_INVALIDARG);	// One or more arguments are invalid
	}
	EnterSnapshotMode();
	SetSnapshot(pSnapshot);
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

void CWhorldThread::OnSetSnapshotSize(SIZE szSnapshot)
{
	// if we're currently displaying a legacy snapshot
	if (m_dsSnapshot.nFlags & CSnapshot::SF_V1) {
		m_dsSnapshot.szTarget = CD2DSizeF(szSnapshot);	// update its frame size
	}
}

void CWhorldThread::OnMovieRecord(LPCTSTR pszMoviePath)
{
	if (pszMoviePath != NULL) {	// if path specified
		// start recording
		CString	sPath(pszMoviePath);
		delete [] pszMoviePath;
		m_movie.SetFrameRate(static_cast<float>(m_nFrameRate));
		m_movie.SetTargetSize(m_szTarget);
		if (!m_movie.Open(sPath, true)) {
			OnMovieError();
		}
	} else {	// no path; stop recording
		if (!m_movie.Close()) {
			OnMovieError();
		}
	}
}

void CWhorldThread::OnMoviePlay(LPCTSTR pszMoviePath, bool bPaused)
{
	if (pszMoviePath != NULL) {	// if path specified
		// start playback
		CString	sPath(pszMoviePath);
		delete [] pszMoviePath;
		if (!m_movie.Open(sPath, false)) {
			OnMovieError();
			return;
		}
		m_bIsMoviePaused = bPaused;
		EnterSnapshotMode();
	} else {	// no path; stop playback
		m_movie.Close();
	}
}

void CWhorldThread::OnMoviePause(bool bEnable)
{
	m_bIsMoviePaused = bEnable;
}

void CWhorldThread::OnMovieSeek(LONGLONG iFrame)
{
	if (m_movie.SeekFrame(iFrame)) {	// if seek succeeds
		if (m_bIsMoviePaused) {	// if movie is paused
			m_bMovieSingleStep = true;	// set single step flag
			ReadMovieFrame();	// read exactly one frame
		}
	} else {	// seek failed
		OnMovieError();
	}
}


void CWhorldThread::OnMovieExport(const CMovieExportParams* pParams, LONG nTaskID)
{
	ASSERT(pParams != NULL);
	CAutoPtr<const CMovieExportParams>	pMEP(pParams);	// take ownership of parameters
	CString	sFolderPath(pMEP->m_sFolderPath);
	m_bIsMoviePaused = true;	// pause playback during export
	// end frame is inclusive, hence the plus one
	LONGLONG	nFrames = pMEP->m_nEndFrame - pMEP->m_nStartFrame + 1;
	if (!m_movie.SeekFrame(pMEP->m_nStartFrame)) {
		OnMovieError();
		return;	// fail
	}
	CSetAntialiasing	setAA(true);	// enable antialiasing; dtor restores state
	m_nTaskItemsDone = 0;	// watch out for races when reading this
	// for each frame in specified range of frames
	for (LONGLONG iFrame = 0; iFrame < nFrames; iFrame++) {
		if (m_nCancelTaskID >= nTaskID) {	// if task canceled
			break;	// abort
		}
		m_bMovieSingleStep = true;
		if (!ReadMovieFrame()) {	// if can't read frame
			break;	// fail
		}
		CComPtr<ID2D1Bitmap1> pBitmap;
		CWhorldDraw::CaptureBitmap(pMEP->m_nExportFlags, pMEP->m_szFrame, &pBitmap);
		WriteCapturedBitmap(pBitmap, MakeImageSequenceFileName(sFolderPath, iFrame));
		m_nTaskItemsDone = iFrame;
	}
	PostMsgToMainWnd(UWM_RENDER_TASK_DONE, nTaskID);	// notify main thread
}

CString CWhorldThread::MakeImageSequenceFileName(CString sFolderPath, LONGLONG iFrame)
{
	CString	sSeqNum;
	sSeqNum.Format(_T("%06lld"), iFrame);
	return sFolderPath + _T("\\img") + sSeqNum + _T(".png");
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
	case RC_RANDOM_ORIGIN:
		OnRandomOrigin();
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
	case RC_SET_SNAPSHOT_SIZE:
		OnSetSnapshotSize(cmd.m_prop.szVal);
		break;
	case RC_MOVIE_RECORD:
		OnMovieRecord(static_cast<LPCTSTR>(cmd.m_prop.byref));
		break;
	case RC_MOVIE_PLAY:
		OnMoviePlay(static_cast<LPCTSTR>(cmd.m_prop.byref), cmd.m_nParam != 0);
		break;
	case RC_MOVIE_PAUSE:
		OnMoviePause(cmd.m_nParam != 0);
		break;
	case RC_MOVIE_SEEK:
		OnMovieSeek(cmd.m_prop.intVal);
		break;
	case RC_MOVIE_EXPORT:
		OnMovieExport(static_cast<const CMovieExportParams*>(cmd.m_prop.byref), cmd.m_nParam);
		break;
	default:
		NODEFAULTCASE;	// missing command case
	};
}
