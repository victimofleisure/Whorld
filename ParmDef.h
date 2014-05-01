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

        define parameter attributes

*/

// APPEND ONLY: Do not delete or reorder rows, otherwise pre-existing snapshots
// will not load correctly and may crash the app.  To change the order in which
// rows are displayed, edit CParmInfo::m_RowOrder.  To add a row, you must also:
//
// 1. Initialize the new CWhorldView::PARMS member in CWhorldView::m_DefParms,
// and implement the corresponding behavior in CWhorldView.cpp.
//
// 2. Add a string resource for the title; note that the resource symbol must
// have the form IDS_VP_*, where * is the row's tag, e.g. IDS_VP_RING_GROWTH.
//
// 3. Specify the display order, i.e. at what position the row should appear in
// dialogs, by inserting the row's tag somewhere in CParmInfo::m_RowOrder.

//		Name			Tag				MinVal	MaxVal	Steps	Scale
PARMDEF(RingGrowth,		RING_GROWTH,	0,		10,		1000,	1		)
PARMDEF(RingSpacing,	RING_SPACING,	1,		50,		1000,	1		)
PARMDEF(PolySides,		POLY_SIDES,		3,		50,		47,		1		)
PARMDEF(RotateSpeed,	ROTATE_SPEED,	DTR(-5),DTR(5),	1000,	DTR(1)	)
PARMDEF(AspectRatio,	ASPECT_RATIO,	-3,		3,		1000,	1		)
PARMDEF(SkewRadius,		SKEW_RADIUS,	0,		2,		1000,	1		)
PARMDEF(SkewAngle,		SKEW_ANGLE,		-PI,	PI,		1000,	DTR(1)	)
PARMDEF(StarFactor,		STAR_FACTOR,	-3,		3,		1000,	1		)
PARMDEF(ColorSpeed,		COLOR_SPEED,	0,		10,		1000,	1		)
PARMDEF(Lightness,		LIGHTNESS,		0,		1,		1000,	1		)
PARMDEF(Saturation,		SATURATION,		0,		1,		1000,	1		)
PARMDEF(BkHue,			BK_HUE,			0,		360,	1000,	1		)
PARMDEF(BkLightness,	BK_LIGHTNESS,	0,		1,		1000,	1		)
PARMDEF(BkSaturation,	BK_SATURATION,	0,		1,		1000,	1		)
PARMDEF(Pinwheel,		PINWHEEL,		-3,		3,		1000,	1		)
PARMDEF(LineWidth,		LINE_WIDTH,		1,		50,		1000,	1		)
PARMDEF(EvenCurve,		EVEN_CURVE,		-3,		3,		1000,	1		)
PARMDEF(OddCurve,		ODD_CURVE,		-3,		3,		1000,	1		)
PARMDEF(EvenShear,		EVEN_SHEAR,		-3,		3,		1000,	1		)
PARMDEF(OddShear,		ODD_SHEAR,		-3,		3,		1000,	1		)
