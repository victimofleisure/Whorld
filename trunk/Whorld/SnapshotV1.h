// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23feb25	initial version

*/

#pragma once

#include "WhorldBase.h"
#include "Snapshot.h"

class CSnapshotV1 : public CWhorldBase {
public:
// Operations
	static CSnapshot* Read(CFile& fIn);
	static void ThrowBadFormat(CArchive &ar);

protected:
// Types
	struct PARMS_V1 {
		double	Parm[20];		// geometry parameters
	};
	struct RING_V1 {
		double	RotDelta;		// additional rotation per tick
		DPOINT	ShiftDelta;		// additional shear per tick
		double	Hue;			// ring's current hue
		float	Lightness;		// ring's current lightness
		float	Saturation;		// ring's current saturation
		// mini-ring begins here
		double	Rot;			// rotation for all vertices, in radians
		double	Steps;			// radius of even vertices, in pixels
		DPOINT	Scale;			// anisotropic scaling
		DPOINT	Shift;			// shear, in pixels
		double	StarRatio;		// ratio of odd radii to even radii
		short	Sides;			// polygon's number of sides
		bool	Delete;			// true if this ring should be deleted
		bool	Reverse;		// true if ring was born growing inward
		int		Color;			// ring's current color, in RGB
		double	Pinwheel;		// additional rotation for odd vertices
		short	LineWidth;		// pen width, in pixels, or 0 to use DC pen
		short	DrawMode;		// see draw mode enum
		float	Spacing;		// distance between rings, in pixels
		DPOINT	Origin;			// origin in client coords relative to window center
		float	EvenCurve;		// even vertex curvature, as multiple of radius
		float	OddCurve;		// odd vertex curvature, as multiple of radius
		float	EvenShear;		// even vertex curve point asymmetry ratio
		float	OddShear;		// odd vertex curve point asymmetry ratio
	};
	struct GLOBRING_V1 {
		double	Rot;			// rotation for all vertices, in radians
		double	StarRatio;		// ratio of odd radii to even radii
		double	Pinwheel;		// additional rotation for odd vertices
		DPOINT	Scale;			// anisotropic scaling
		DPOINT	Shift;			// shear, in pixels
		float	EvenCurve;		// even vertex curvature, as multiple of radius
		float	OddCurve;		// odd vertex curvature, as multiple of radius
		float	EvenShear;		// even vertex curve point asymmetry ratio
		float	OddShear;		// odd vertex curve point asymmetry ratio
		int		LineWidth;		// line width, in pixels
		int		PolySides;		// number of sides
	};
	struct STATE_V1 {
		double	RingOffset;		// size of gap since last ring, in pixels
		double	CanvasScale;	// canvas size, as a fraction of client window
		double	Hue;			// hue of new rings, in degrees
		int		Color;			// color of new rings, in RGB
		int		BkColor;		// background color, in RGB
		int		DrawMode;		// see draw mode enum
		bool	Mirror;			// true if we're mirroring
		bool	HueLoop;		// true if hue is being looped
		bool	Convex;			// true if rings are drawn in descending size order
		BYTE	VideoROPIdx;	// index of video raster operation
		DPOINT	NormOrg;		// normalized origin: 0,0 = upper left, 1,1 = lower right
		DPOINT	MirrorOrg;		// distance from origin to window center, in client coords
		double	HueLoopPos;		// hue loop position, in degrees
		double	HueLoopLength;	// length of hue loop, in degrees
		float	HueLoopBase;	// hue loop's base hue, in degrees
		int		InvertMask;		// 0xffffff if inverting color, else zero
		double	Zoom;			// magnification factor, 1 = normal
		double	Trail;			// origin propagation: 0 = no trail, 1 = permanent origin
		DPOINT	VideoOrg;		// video origin in mirror mode
	};

// Helpers
	static void	CvtRing(const RING_V1& ringOld, RING& ringNew);
	static void	CvtState(const STATE_V1& stateOld, CSnapshot::STATE& stateNew);
	static void CvtGlobRing(const GLOBRING_V1& grOld, GLOBRING& grNew);
	static D2D1::ColorF CvtColor(COLORREF clr);
};
