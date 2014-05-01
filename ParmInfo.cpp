// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21may05	initial version
		01		26may05	add origin and draw mode
		02		08jun05	add mirror
		03		22jul05	add row order table
		04		27jul05	use CFormatIO
		05		19feb06	make write functions const
		06		23mar06	use BYTE instead of INT for booleans
		07		13apr06	add even and odd curve
        08      17apr06	move I/O and non-parm data into CPatch
		09		10dec07	add global parameters
		10		13dec07	add even and odd shear
		11		15jan08	add globals for line width and poly sides
		12		28jan08	support Unicode

		parameter container
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "ParmInfo.h"
#include "WhorldView.h"

// automatically generate table of row data from ParmDef.h macro
#undef PARMDEF
#define PARMDEF(Name, Tag, MinVal, MaxVal, Steps, Scale) \
	{_T(#Name), IDS_VP_##Tag, MinVal, MaxVal, Steps, Scale},
const CParmInfo::ROWDATA CParmInfo::m_RowData[ROWS] = {
#include "ParmDef.h"
};

// rows are displayed in this order, which is NOT the same as the order in
// which they're stored; the storage order must not change, see ParmDef.h
const int CParmInfo::m_RowOrder[ROWS] = {
	RING_GROWTH,	// pixels of ring growth per tick
	RING_SPACING,	// gap between rings, in pixels
	LINE_WIDTH,		// line width, in pixels
	POLY_SIDES,		// ring's number of sides, rounded to int
	ROTATE_SPEED,	// ring rotation per tick, in radians
	ASPECT_RATIO,	// 1 = double width, -1 = double height
	SKEW_RADIUS,	// size of origin offset, in pixels
	SKEW_ANGLE,		// angle of origin offset, in radians
	STAR_FACTOR,	// 0 = no star, > 1 = convex, < 1 = concave
	PINWHEEL,		// odd vertex rotation, in radians
	COLOR_SPEED,	// hue change per tick, in degrees
	LIGHTNESS,		// color lightness, from 0..1
	SATURATION,		// color saturation, from 0..1
	BK_HUE,			// background color hue, in degrees
	BK_LIGHTNESS,	// background color lightness, from 0..1
	BK_SATURATION,	// background color saturation, from 0..1
	EVEN_CURVE,		// even vertex curvature, as fraction of radius
	ODD_CURVE,		// odd vertex curvature, as fraction of radius
	EVEN_SHEAR,		// even vertex curvature asymmetry, as a ratio
	ODD_SHEAR		// odd vertex curvature asymmetry, as a ratio
};

const int CParmInfo::m_GlobParm[GLOBAL_PARMS] = {
	CWhorldView::LINE_WIDTH,
	CWhorldView::POLY_SIDES,
	CWhorldView::ROTATE_SPEED,
	CWhorldView::ASPECT_RATIO,
	CWhorldView::SKEW_RADIUS,
	CWhorldView::SKEW_ANGLE,
	CWhorldView::STAR_FACTOR,
	CWhorldView::PINWHEEL,
	CWhorldView::EVEN_CURVE,
	CWhorldView::ODD_CURVE,
	CWhorldView::EVEN_SHEAR,
	CWhorldView::ODD_SHEAR,
};

void CParmInfo::SetDefaults()
{
	ZeroMemory(&m_Row, sizeof(m_Row));
	CWhorldView::PARMS	vp;
	CWhorldView::GetDefaults(vp);
	for (int i = 0; i < ROWS; i++) {
		m_Row[i].Val = ((double *)&vp)[i];
		m_Row[i].PW = .5;
	}
}
