// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23feb25	initial version
		01		09mar25	add flags bitmask to drawing state
		02      11mar25	get target size from options

*/

#include "stdafx.h"
#include "Whorld.h"	// for legacy snapshot sizes
#include "SnapshotV1.h"
#include "Snapshot.h"

void CSnapshotV1::ThrowBadFormat(CArchive &ar)
{
	AfxThrowArchiveException(CArchiveException::badIndex, ar.m_strFileName);
}

CSnapshot* CSnapshotV1::Read(CFile& fIn)
{
	fIn.SeekToBegin();
	CArchive	ar(&fIn, CArchive::load);
	ar.m_strFileName = fIn.GetFilePath();
	int	nSig, nVersion;
	ar >> nSig;
	ar >> nVersion;
	if (nSig != SNAPSHOT_SIG || nVersion > SNAPSHOT_VERSION)
		ThrowBadFormat(ar);
	int	nParmsSize;
	ar >> nParmsSize;
	if (nParmsSize > sizeof(PARMS_V1))
		ThrowBadFormat(ar);
	PARMS_V1	parms;
	ar.Read(&parms, nParmsSize);
	int	nRings, nRingSize;
	ar >> nRingSize;
	ar >> nRings;
	CAutoPtr<CSnapshot>	pSnapshot(CSnapshot::Alloc(nRings));
	ZeroMemory(pSnapshot->m_aRing, sizeof(RING) * nRings);
	RING_V1	ring;
	ZeroMemory(&ring, sizeof(RING_V1));
	if (nRingSize > sizeof(RING_V1))
		ThrowBadFormat(ar);
	for (int iRing = 0; iRing < nRings; iRing++) {	// for each ring
		ar.Read(&ring, nRingSize);	// read ring into buffer
		CvtRing(ring, pSnapshot->m_aRing[iRing]);	// convert to V2
	}
	if (nVersion > 3) {	// if video overlay is supported
		CString	sVideoPath;
		DWORD	nVideoFrame;
		ar >> sVideoPath;
		ar >> nVideoFrame;
	}
	int	nStateSize;
	ar >> nStateSize;
	if (nStateSize > sizeof(DRAW_STATE_V1))
		ThrowBadFormat(ar);
	DRAW_STATE_V1	drawState;
	ZeroMemory(&drawState, sizeof(DRAW_STATE_V1));
	ar.Read(&drawState, nStateSize);
	ZeroMemory(&pSnapshot->m_drawState, sizeof(DRAW_STATE));
	CvtState(drawState, pSnapshot->m_drawState);
	pSnapshot->m_drawState.nRings = nRings;
	GLOB_RING_V1	globring;
	pSnapshot->m_globRing = m_globalRingDefault;
	if (nVersion > 4) {	// if global parameters are supported
		int	nGlobRingSize;
		ar >> nGlobRingSize;
		if (nRingSize > sizeof(GLOB_RING))
			ThrowBadFormat(ar);
		ZeroMemory(&globring, sizeof(GLOB_RING));
		ar.Read(&globring, nGlobRingSize);
		CvtGlobRing(globring, pSnapshot->m_globRing);
	}
	return pSnapshot.Detach();
}

inline D2D1::ColorF CSnapshotV1::CvtColor(COLORREF clr)
{
	return D2D1::ColorF(
		static_cast<FLOAT>(GetRValue(clr)) / 255.0f,
		static_cast<FLOAT>(GetGValue(clr)) / 255.0f,
		static_cast<FLOAT>(GetBValue(clr)) / 255.0f);
}

void CSnapshotV1::CvtRing(const RING_V1& ringOld, RING& ringNew)
{
	ringNew.ptShiftDelta	= ringOld.Shift;
	ringNew.fRotDelta		= ringOld.RotDelta;
	ringNew.fHue			= ringOld.Hue;
	ringNew.fLightness		= ringOld.Lightness;
	ringNew.fSaturation		= ringOld.Saturation;
	ringNew.fRot			= ringOld.Rot;
	ringNew.fRadius			= ringOld.Steps;
	ringNew.ptScale			= ringOld.Scale;	
	ringNew.ptShift			= ringOld.Shift;
	ringNew.fStarRatio		= ringOld.StarRatio;
	ringNew.nSides			= ringOld.Sides;
	ringNew.nDrawMode		= ringOld.DrawMode;
	ringNew.bDelete			= ringOld.Delete;
	ringNew.bSkipFill		= false;
	ringNew.clrCur			= CvtColor(ringOld.Color);
	ringNew.fPinwheel		= ringOld.Pinwheel;
	ringNew.fLineWidth		= max(ringOld.LineWidth, 1);
	ringNew.ptOrigin		= ringOld.Origin;
	ringNew.fEvenCurve		= ringOld.EvenCurve;
	ringNew.fOddCurve		= ringOld.OddCurve;
	ringNew.fEvenShear		= ringOld.EvenShear;
	ringNew.fOddShear		= ringOld.OddShear;
}

void CSnapshotV1::CvtState(const DRAW_STATE_V1& stateOld, DRAW_STATE& stateNew)
{
	// V1 snapshot format didn't include a frame size, so user must specify it
	stateNew.szTarget		= CD2DSizeF(theApp.m_options.GetLegacySnapshotSize());
	stateNew.clrBkgnd		= CvtColor(stateOld.BkColor);
	stateNew.fZoom			= stateOld.Zoom;
	stateNew.bConvex		= stateOld.Convex;
	stateNew.nFlags			= CSnapshot::SF_V1;
}

void CSnapshotV1::CvtGlobRing(const GLOB_RING_V1& grOld, GLOB_RING& grNew)
{
	grNew.fRot				= grOld.Rot;
	grNew.fStarRatio		= grOld.StarRatio;
	grNew.fPinwheel			= grOld.Pinwheel;
	grNew.ptScale			= grOld.Scale;
	grNew.ptShift			= grOld.Shift;
	grNew.fEvenCurve		= grOld.EvenCurve;
	grNew.fOddCurve			= grOld.OddCurve;
	grNew.fEvenShear		= grOld.EvenShear;
	grNew.fOddShear			= grOld.OddShear;
	grNew.fLineWidth		= grOld.LineWidth;
	grNew.nPolySides		= grOld.PolySides;
}
