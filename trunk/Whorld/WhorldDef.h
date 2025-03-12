// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		23apr05	add lightness, saturation
		02		27apr05	add background HLS
		03		18may05	add pinwheel
		04		22jul05	add line width
		05		13apr06	add even and odd curve
		06		13dec07	add even and odd shear
		07		09feb25	consolidate various def files here
		08		20feb25	add bitmap render commands
		09		26feb25	unsnarl parameter order
		10		01mar25	add commands to set origin coords individually
		11		10mar25	change ring spacing maxval to odd number for slider
		12		11mar25	add command to set legacy snapshot frame size
		13		12mar25	add reserved member to ring struct

        define parameter attributes

*/

#ifdef PARAMDEF	// parameters

//			name			minval	maxval	steps	scale	initval
PARAMDEF(	RingGrowth,		0,		10,		1000,	1,		1		)	// pixels of ring growth per tick
PARAMDEF(	RingSpacing,	1,		51,		1000,	1,		5		)	// gap between rings, in pixels
PARAMDEF(	LineWidth,		1,		50,		1000,	1,		1		)	// line width, in pixels
PARAMDEF(	PolySides,		3,		51,		48,		1,		5		)	// ring's number of sides, rounded to int
PARAMDEF(	RotateSpeed,	DTR(-5),DTR(5),	1000,	DTR(1),	0		)	// ring rotation per tick, in radians
PARAMDEF(	AspectRatio,	-3,		3,		1000,	1,		0		)	// 1 = double width, -1 = double height
PARAMDEF(	SkewRadius,		0,		2,		1000,	1,		0		)	// size of origin offset, in pixels
PARAMDEF(	SkewAngle,		-PI,	PI,		1000,	DTR(1),	0		)	// angle of origin offset, in radians
PARAMDEF(	StarFactor,		-3,		3,		1000,	1,		0		)	// 0 = no star, > 1 = convex, < 1 = concave
PARAMDEF(	Pinwheel,		-3,		3,		1000,	1,		0		)	// odd vertex rotation, in radians
PARAMDEF(	ColorSpeed,		0,		10,		1000,	1,		0.5		)	// hue change per tick, in degrees
PARAMDEF(	Lightness,		0,		1,		1000,	1,		0.5		)	// color lightness, from 0..1
PARAMDEF(	Saturation,		0,		1,		1000,	1,		1		)	// color saturation, from 0..1
PARAMDEF(	BkHue,			0,		360,	1000,	1,		0		)	// background color hue, in degrees
PARAMDEF(	BkLightness,	0,		1,		1000,	1,		0		)	// background color lightness, from 0..1
PARAMDEF(	BkSaturation,	0,		1,		1000,	1,		1		)	// background color saturation, from 0..1
PARAMDEF(	EvenCurve,		-3,		3,		1000,	1,		0		)	// even vertex curvature, as fraction of radius
PARAMDEF(	OddCurve,		-3,		3,		1000,	1,		0		)	// odd vertex curvature, as fraction of radius
PARAMDEF(	EvenShear,		-3,		3,		1000,	1,		0		)	// even vertex curvature asymmetry, as a ratio
PARAMDEF(	OddShear,		-3,		3,		1000,	1,		0		)	// odd vertex curvature asymmetry, as a ratio

#undef PARAMDEF
#endif // PARAMDEF

#ifdef PARAMPROPDEF	// parameter properties

//				name	type	prefix	variant	
#if !defined(PARAMPROPDEF_GLOBAL) || !PARAMPROPDEF_GLOBAL	// exclude global property
PARAMPROPDEF(	Val,	DOUBLE,	f,		dblVal	)	// parameter's base value, to which modulation is applied
PARAMPROPDEF(	Wave,	INT,	i,		intVal	)	// index of modulation waveform; see enum in COscillator.h
PARAMPROPDEF(	Amp,	DOUBLE,	f,		dblVal	)	// modulation amplitude; can be negative for phase inversion
PARAMPROPDEF(	Freq,	DOUBLE,	f,		dblVal	)	// modulation frequency, in Hertz
PARAMPROPDEF(	PW,		DOUBLE,	f,		dblVal	)	// modulation pulse width, from 0..1; only meaningful for pulse waveform
#endif
#if !defined(PARAMPROPDEF_GLOBAL) || PARAMPROPDEF_GLOBAL	// exclude all except global property
PARAMPROPDEF(	Global,	DOUBLE,	f,		dblVal	)	// parameter's global value, which is applied to all rings
#endif

#undef PARAMPROPDEF
#undef PARAMPROPDEF_GLOBAL
#endif // PARAMPROPDEF

#ifdef GLOBALPARAMDEF	// global parameters

GLOBALPARAMDEF(	LineWidth	)
GLOBALPARAMDEF(	PolySides	)
GLOBALPARAMDEF(	RotateSpeed	)
GLOBALPARAMDEF(	AspectRatio	)
GLOBALPARAMDEF(	SkewRadius	)
GLOBALPARAMDEF(	SkewAngle	)
GLOBALPARAMDEF(	StarFactor	)
GLOBALPARAMDEF(	Pinwheel	)
GLOBALPARAMDEF(	EvenCurve	)
GLOBALPARAMDEF(	OddCurve	)
GLOBALPARAMDEF(	EvenShear	)
GLOBALPARAMDEF(	OddShear	)

#undef GLOBALPARAMDEF
#endif // GLOBALPARAMDEF

#ifdef MASTERDEF	// master properties

//			name			type	prefix	initval
MASTERDEF(	Speed,			DOUBLE,	f,		1			)	// master speed, as a fraction; 1 = nominal
MASTERDEF(	Zoom,			DOUBLE,	f,		1			)	// zoom, as a fraction; 1 = nominal
MASTERDEF(	Damping,		DOUBLE,	f,		0.186261	)	// origin/zoom damping; 1 = none, 0 = stuck
MASTERDEF(	Trail,			DOUBLE,	f,		0			)	// origin trail; 0 = none, 1 = stuck
MASTERDEF(	Rings,			DOUBLE,	f,		MAX_RINGS	)	// maximum number of rings
MASTERDEF(	Tempo,			DOUBLE,	f,		100			)	// tempo, in BMP
MASTERDEF(	HueSpan,		DOUBLE,	f,		30			)	// if looping hue, loop length in degrees
MASTERDEF(	CanvasScale,	DOUBLE,	f,		1.4			)	// canvas size, as fraction of window size
MASTERDEF(	Copies,			DOUBLE,	f,		1			)	// number of instances
MASTERDEF(	Spread,			DOUBLE,	f,		100			)	// radius of instance origins, in pixels

#undef MASTERDEF
#endif // MASTERDEF

#ifdef MAINDEF	// main properties

//			name			type	prefix	initval		variant
MAINDEF(	Origin,			DPoint,	dpt,	INIT_ORIGIN,fltPt	)	// view origin, in normalized coordinates
MAINDEF(	DrawMode,		UINT,	n,		0,			intVal	)	// draw mode bitmask; see enum in CWhorldBase
MAINDEF(	OrgMotion,		INT,	n,		0,			intVal	)	// origin motion; see enum in CWhorldBase
MAINDEF(	Hue,			DOUBLE,	f,		0,			dblVal	)	// current hue in degrees
MAINDEF(	Mirror,			bool,	b,		false,		boolVal	)	// true if mirroring
MAINDEF(	Reverse,		bool,	b,		false,		boolVal	)	// true if rings growing inward
MAINDEF(	Convex,			bool,	b,		false,		boolVal	)	// true if drawing in descending size order
MAINDEF(	InvertColor,	bool,	b,		false,		boolVal	)	// true if inverting color
MAINDEF(	LoopHue,		bool,	b,		false,		boolVal	)	// true if looping hue
MAINDEF(	ZoomCenter,		bool,	b,		false,		boolVal	)	// true if zoom is window-centered

#undef MAINDEF
#endif // MAINDEF

#ifdef PARAMCOLDEF	// parameter columns

PARAMCOLDEF(NAME_STATIC)
PARAMCOLDEF(VAL_SLIDER)
PARAMCOLDEF(Val)
PARAMCOLDEF(Wave)
PARAMCOLDEF(Amp)
PARAMCOLDEF(Freq)
PARAMCOLDEF(PW)

#undef PARAMCOLDEF
#endif // PARAMCOLDEF

#ifdef GLOBALCOLDEF	// global parameter columns

GLOBALCOLDEF(NAME_STATIC)
GLOBALCOLDEF(VAL_SLIDER)
GLOBALCOLDEF(Val)

#undef GLOBALCOLDEF
#endif // GLOBALCOLDEF

#ifdef WAVEFORMDEF	// waveforms

WAVEFORMDEF(TRIANGLE)
WAVEFORMDEF(SINE)
WAVEFORMDEF(RAMP_UP)
WAVEFORMDEF(RAMP_DOWN)
WAVEFORMDEF(SQUARE)
WAVEFORMDEF(PULSE)
WAVEFORMDEF(RANDOM)
WAVEFORMDEF(RANDOM_RAMP)

#undef WAVEFORMDEF
#endif // WAVEFORMDEF

#ifdef MASTERCOLDEF	// master columns

MASTERCOLDEF(NAME_STATIC)
MASTERCOLDEF(VAL_SLIDER)
MASTERCOLDEF(VAL_EDIT)

#undef MASTERCOLDEF
#endif // MASTERCOLDEF

#ifdef RENDERCMDDEF	// render commands

// set parameter commands are defined separately via PARAMPROPDEF

//			 name				vartype
RENDERCMDDEF(SET_MASTER,		dblVal	)	// sets a master property; m_nParam: property index, m_prop: fProp
RENDERCMDDEF(SET_MAIN,			intVal	)	// sets a main property; m_nParam: property index, m_prop: variant property
RENDERCMDDEF(SET_PATCH,			byref	)	// sets a patch; m_nParam: unused, m_prop: CPatch pointer allocated on heap
RENDERCMDDEF(SET_EMPTY,			intVal	)	// removes all geometry; no parameters
RENDERCMDDEF(SET_FRAME_RATE,	intVal	)	// sets frame rate; m_nParam: integer frame rate in Hertz
RENDERCMDDEF(SET_PAUSE,			intVal	)	// pauses updates; m_nParam: non-zero to pause, zero to unpause
RENDERCMDDEF(SINGLE_STEP,		intVal	)	// single steps while paused; no parameters
RENDERCMDDEF(RANDOM_PHASE,		intVal	)	// randomizes phase of all oscillators; no parameters
RENDERCMDDEF(SET_ZOOM,			dblVal	)	// sets zoom; m_nParam: non-zero for damping, m_prop: double fZoom
RENDERCMDDEF(SET_ORIGIN,		fltPt	)	// sets origin; m_nParam: non-zero for damping, m_prop: POINTFLOAT ptOrigin
RENDERCMDDEF(SET_ORIGIN_X,		dblVal	)	// sets origin x-coord; m_nParam: non-zero for damping, m_prop: double
RENDERCMDDEF(SET_ORIGIN_Y,		dblVal	)	// sets origin y-coord; m_nParam: non-zero for damping, m_prop: double
RENDERCMDDEF(CAPTURE_BITMAP,	szVal	)	// capture a bitmap; m_nParam: image export flags, m_prop: SIZE szImage
RENDERCMDDEF(CAPTURE_SNAPSHOT,	intVal	)	// capture a snapshot; no parameters
RENDERCMDDEF(DISPLAY_SNAPSHOT,	byref	)	// display a snapshot; m_nParam: none, m_prop: CSnapshot* pSnapshot
RENDERCMDDEF(SET_DAMPED_GLOBAL,	dblVal	)	// sets a global parameter with damping; m_nParam: parameter index, m_prop: double
RENDERCMDDEF(SET_DRAW_MODE,		uintVal	)	// sets draw mode bits; m_nParam: mask, m_prop: value
RENDERCMDDEF(SET_SNAPSHOT_SIZE,	szVal	)	// sets legacy snapshot frame size; m_nParam: none, m_prop: SIZE szSnapshot

#undef RENDERCMDDEF
#endif // RENDERCMDDEF

#ifdef STATEDEF	// state members

STATEDEF(	D2D1_SIZE_F,	szTarget)	// target size in device-independent pixels
STATEDEF(	D2D1_COLOR_F,	clrBkgnd)	// background color
STATEDEF(	double,	fZoom)			// current zoom, as a scaling factor
STATEDEF(	int,	nRings)			// number of elements in ring array
STATEDEF(	bool,	bConvex)		// true if drawing in descending size order
STATEDEF(	BYTE,	nFlags)			// snapshot flags bitmask; see enum below
STATEDEF(	USHORT,	nReserved)		// reserved, must be zero	

#undef STATEDEF
#endif // STATEDEF

#ifdef RINGDEF	// ring members

RINGDEF(DPOINT,	ptShiftDelta)	// additional shear per tick
RINGDEF(double,	fRotDelta)		// additional rotation per tick
RINGDEF(double,	fHue)			// current hue
RINGDEF(double,	fLightness)		// current lightness
RINGDEF(double,	fSaturation)	// current saturation
RINGDEF(double,	fRot)			// rotation for all vertices, in radians
RINGDEF(double,	fRadius)		// radius of even vertices, in pixels
RINGDEF(DPOINT,	ptScale)		// anisotropic scaling
RINGDEF(DPOINT,	ptShift)		// shear, in pixels
RINGDEF(double,	fStarRatio)		// ratio of odd radii to even radii
RINGDEF(short,	nSides)			// polygon's number of sides
RINGDEF(short,	nDrawMode)		// see draw mode enum
RINGDEF(bool,	bDelete)		// true if ring should be deleted
RINGDEF(bool,	bSkipFill)		// true if ring should be skipped in fill mode
RINGDEF(short,	nReserved)		// reserved, must be zero
RINGDEF(D2D1_COLOR_F,	clrCur)	// current color
RINGDEF(double,	fPinwheel)		// additional rotation for odd vertices, in radians
RINGDEF(double,	fLineWidth)		// line width, in pixels
RINGDEF(DPOINT,	ptOrigin)		// origin in client coords relative to window center
RINGDEF(double,	fEvenCurve)		// even vertex curvature, as multiple of radius
RINGDEF(double,	fOddCurve)		// odd vertex curvature, as multiple of radius
RINGDEF(double,	fEvenShear)		// even vertex curve point asymmetry ratio
RINGDEF(double,	fOddShear)		// odd vertex curve point asymmetry ratio

#undef RINGDEF
#endif // RINGDEF

#ifdef GLOBRINGDEF	// global ring members

GLOBRINGDEF(double,	fRot)			// rotation for all vertices, in radians
GLOBRINGDEF(double,	fStarRatio)		// ratio of odd radii to even radii
GLOBRINGDEF(double,	fPinwheel)		// additional rotation for odd vertices
GLOBRINGDEF(DPOINT,	ptScale)		// anisotropic scaling
GLOBRINGDEF(DPOINT,	ptShift)		// shear, in pixels
GLOBRINGDEF(double,	fEvenCurve)		// even vertex curvature, as multiple of radius
GLOBRINGDEF(double,	fOddCurve)		// odd vertex curvature, as multiple of radius
GLOBRINGDEF(double,	fEvenShear)		// even vertex curve point asymmetry ratio
GLOBRINGDEF(double,	fOddShear)		// odd vertex curve point asymmetry ratio
GLOBRINGDEF(double,	fLineWidth)		// line width, in pixels
GLOBRINGDEF(int,	nPolySides)		// number of sides

#undef GLOBRINGDEF
#endif // GLOBRINGDEF
