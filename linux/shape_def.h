// Port-only shape primitives, layered on top of the upstream Whorld parameters.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// This file is NOT part of trunk/Whorld/WhorldDef.h, so the Windows build is
// completely unaffected. The Linux port appends these to the upstream parameter
// list so that the sidebar, oscillators, globals, MIDI/gamepad/audio mapping,
// and .whp patch I/O all work for them automatically (parameters are matched by
// name, and unknown names are ignored by both loaders).
//
// Snapshots (.whs) and movies (.whm) must stay byte-compatible with Windows, and
// the Windows ring struct has no room for shape data, so a primitive is
// approximated by a regular polygon when a snapshot or movie is written.
//
// Define SHAPEDEF(name, minval, maxval, steps, initval) before including this
// file; it is undefined again at the end, exactly like WhorldDef.h.

#ifdef SHAPEDEF

//		name			minval	maxval	steps	initval
// Primitive is a selector; see the Primitive enum in whorld_core.h.
SHAPEDEF(	Primitive,		0,		6,		6,		0		)	// shape primitive: polygon, circle, square, triangle, star, cross, line
SHAPEDEF(	PolyCorner,		0,		1,		1000,	0		)	// polygon corner rounding, as a fraction of the shortest edge
SHAPEDEF(	CircleStart,	0,		360,	1000,	0		)	// arc start angle, in degrees, measured clockwise from the top
SHAPEDEF(	CircleSweep,	0,		360,	1000,	360		)	// arc sweep, in degrees; 360 draws a full circle
SHAPEDEF(	SquareCorner,	0,		1,		1000,	0		)	// square corner rounding
SHAPEDEF(	TriCorner,		0,		1,		1000,	0		)	// triangle corner rounding
SHAPEDEF(	StarPoints,		3,		24,		21,		5		)	// star point count, rounded to int
SHAPEDEF(	StarInner,		0,		1,		1000,	.5		)	// star inner radius, as a fraction of the outer radius
SHAPEDEF(	StarSharp,		0,		1,		1000,	0		)	// star point rounding; 0 = sharp, 1 = rounded
SHAPEDEF(	CrossWidth,		0,		1,		1000,	.4		)	// cross arm half-thickness, as a fraction of the ring radius
SHAPEDEF(	CrossLength,	0,		1,		1000,	1		)	// cross arm half-length, as a fraction of the ring radius
SHAPEDEF(	BarLength,		0,		2,		1000,	1		)	// line half-length, as a fraction of the ring radius
SHAPEDEF(	BarThickness,	0,		1,		1000,	.2		)	// line thickness, as a fraction of the ring radius

#undef SHAPEDEF
#endif // SHAPEDEF
