// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		23apr05	tweak performance: no CPen, use GDI directly
		02		27apr05	add print support, background HLS
		03		04may05	make PolySides a double
		04		07may05	add export bitmap
		05		09may05	add StepRings
		06		10may05	add direct draw
		07		11may05	move GDI back buffer to its own object
		08		11may05	printing must blit, else lines are too thin
		09		18may05	add pinwheel
		10		19may05	add archive signature
		11		20may05	add ring fill and outline
		12		20may05	use fast round in Draw
		13		20may05	move direct draw to derived view
		14		26may05	add set origin
		15		02jun05	make cascading delete explicit
		16		03jun05	replace width and height with frame
        17		05jun05	add mirror
		18		07jun05	mirror resize must adjust origin 
		19		08jun05	add printing in mirror mode
		20		09jun05	add rotate hue
		21		10jun05	create back buffer in OnCreate
		22		20jun05	add x-ray fill mode
		23		20jun05	erase with ExtTextOut instead of FillRect
		24		20jun05	add mirror orgin to state
		25		27jun05	add optional size to export
		26		01jul05	remove unused bk hue from state
		27		02jul05	wrap negative hue
		28		04jul05	add hue loop
		29		08jul05	add zoom
		30		11jul05	OnPrint must leave DC's mapping unchanged
		31		20jul05	relay button downs to parent frame
		32		22jul05	add line width
		33		26jul05	add print from bitmap
		34		04aug05	add invert and inward growth
		35		11aug05	add per-ring draw mode
		36		12aug05	don't invalidate in SetDrawMode
		37		15aug05	in export, don't access empty PrevLineWidth
		38		27aug05	move oscillators here
		39		30aug05	move bitmap resizing into MakeDIB
		40		01sep05	in Serialize, don't call Mirror needlessly
		41		27sep05	zoom must persist, make it a state member
		42		13oct05	fix hue loop slippage
        43      14oct05	add exclusive mode
		44		18oct05	in Draw, update rings before drawing instead of after
		45		18oct05	in TimerHook, don't interpolate LFO frequency
		46		23oct05	add HLS color to ring
		47		24oct05	remove SetExclusive
		48		25oct05	add mini-rings that exclude deltas and HLS
		49		06nov05	in OnKeyDown, don't relay modifier keys
		50		10nov05	add max rings
		51		21dec05	in SetHueLoopLength, try to avoid hue jump
		52		22dec05	add repaint flag to Mirror
		53		03mar06	add convex
        54      04mar06	add trail
		55		15mar06	add SetGlobalOrigin
		56		22mar06	in Draw, optimize origin clamping
		57		23mar06	in SetHue, set hue loop base and pos
		58		28mar06	add zoom type
		59		29mar06	remove origin clamping
		60		12apr06	in SetZoom, set m_st.Zoom before calling SetWndSize
		61		13apr06	add even and odd curve
		62		18apr06	add freeframe conditionals
		63		27apr06	make curve decision per ring instead of per frame
		64		27apr06	replace memmove calls with inline assembler
		65		29apr06	if convex fill, use previous ring's color
		66		29apr06	replace Draw's CRect calls with inlines
		67		29apr06	only calculate iorg if making curves
		68		06may06	add video overlay
		69		07jun06	include video in snapshot
		70		12jun06	move video ROP into state
		71		19jun06	iorg must include skew, else skew distorts curves
		72		24jun06	add Copies and Spread
		73		29jun06	add video origin
		74		15sep06	move ring updating from Draw to TimerHook
		75		10dec07	add global parameters
		76		13dec07	add even and odd shear
		77		15jan08	add globals for line width and poly sides
		78		24jan08	compute curve points from real vertices
		79		25jan08	in ClearScreen, zero RingOffset to add a ring ASAP
		80		28jan08	support Unicode
		81		29jan08	add static casts to fix warnings
		82		29jan08	in MakeDIB, init vars to fix warnings
		83		30jan08	add OnChar to relay char messages to main frame

        Whorld view
 
*/

// WhorldView.cpp : implementation of the CWhorldView class
//

#include "stdafx.h"
#include "Resource.h"
#include "WhorldView.h"
#include <math.h>
#include "hls.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const double CWhorldView::MIN_ASPECT_RATIO	= 1e-9;
const double CWhorldView::MIN_STAR_RATIO	= 1e-2;

const CWhorldView::PARMS CWhorldView::m_DefParms = {
	1,	// RingGrowth 
	5,	// RingSpacing 
	5,	// PolySides 
	0,	// AspectRatio 
	0,	// RotateSpeed 
	0,	// SkewRadius
	0,	// SkewAngle 
	0,	// StarFactor 
	.5,	// ColorSpeed
	.5,	// Lightness 
	1,	// Saturation 
	0,	// BkHue
	0,	// BkLightness
	1,	// BkSaturation
	0,	// Pinwheel
	1,	// LineWidth
	0,	// EvenCurve
	0,	// OddCurve
	0,	// EvenShear
	0	// OddShear
};

const CWhorldView::STATE CWhorldView::m_DefState = {
	0,	// RingOffset
	3,	// CanvasScale
	0,	// Hue
	0,	// Color
	0,	// BkColor
	0,	// DrawMode
	0,	// Mirror
	0,	// HueLoop
	0,	// Convex
	0,	// VideoROPIdx
	{.5, .5},	// NormOrg
	{0, 0},	// MirrorOrg
	0,	// HueLoopPos
	60,	// HueLoopLength
	0,	// HueLoopBase
	0,	// InvertMask
	1,	// Zoom
	0,	// Trail
	{1, 1}	// VideoOrg
};

const CWhorldView::GLOBRING CWhorldView::m_DefGlobRing = {
	0,	// Rot
	1,	// StarRatio
	0,	// Pinwheel
	{1, 1},	// Scale
	{0, 0},	// Shift
	0,	// EvenCurve
	0,	// OddCurve
	1,	// EvenShear
	1,	// OddShear
	0,	// LineWidth
	0,	// PolySides
};

CWhorldView	*CWhorldView::m_MainView;

/////////////////////////////////////////////////////////////////////////////
// CWhorldView

CWhorldView::CWhorldView()
{
	m_Parms = m_DefParms;
	m_st = m_DefState;
	m_PrevInfo.SetDefaults();
	m_Size = CSize(0, 0);
	m_Origin = CPoint(0, 0);
	m_Canvas.SetRectEmpty();
	m_NewGrowth = 0;
	m_Options = 0;
	m_MaxRings = INT_MAX;
	m_DelPos = NULL;
	m_FlushHistory = FALSE;
	m_MiniRings = FALSE;
	m_Paused = FALSE;
	m_ZoomType = ZT_RING_ORIGIN;
	m_Copying = FALSE;
	m_CopyCount = 1;
	m_CopySpread = 0;
	m_CopyTheta = 0;
	m_CopyDelta = 0;
	m_GlobRing = m_DefGlobRing;
}

CWhorldView::~CWhorldView()
{
}

#ifndef WHORLDFF

IMPLEMENT_DYNCREATE(CWhorldView, CView)

BEGIN_MESSAGE_MAP(CWhorldView, CView)
	//{{AFX_MSG_MAP(CWhorldView)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

#endif

void CWhorldView::ClearScreen()
{
	m_Ring.RemoveAll();
	m_DelPos = NULL;	// avoid bogus delete
	m_st.RingOffset = 0;	// add a ring ASAP
	if (m_Paused)
		Invalidate();
}

inline void CWhorldView::MirrorToNorm(const DPOINT& Mirror, DPOINT& Norm) const
{
	Norm.x = m_Size.cx ? (Mirror.x + m_Size.cx / 2) / m_Size.cx : 0;
	Norm.y = m_Size.cy ? (Mirror.y + m_Size.cy / 2) / m_Size.cy : 0;
}

inline void CWhorldView::NormToMirror(const DPOINT& Norm, DPOINT& Mirror) const
{
	Mirror.x = Norm.x * m_Size.cx - m_Size.cx / 2;
	Mirror.y = Norm.y * m_Size.cy - m_Size.cy / 2;
}

inline void CWhorldView::UpdateClientOrg()
{
	int	ox = round(m_Size.cx * m_st.NormOrg.x);
	int	oy = round(m_Size.cy * m_st.NormOrg.y);
	m_Origin = CPoint(ox, oy);
}

void CWhorldView::ResizeCanvas()
{
	double	scale = m_st.CanvasScale * m_st.Zoom;
	CRect	r(0, 0, round(m_Size.cx * scale), round(m_Size.cy * scale));
	r.OffsetRect((m_Size.cx - r.Width()) / 2, (m_Size.cy - r.Height()) / 2);
	m_Canvas = r;
	UpdateClientOrg();
}

void CWhorldView::SetWndSize(CSize sz)
{
	m_Size = sz;
	MirrorToNorm(m_st.MirrorOrg, m_st.NormOrg);	// update normalized origin
	ResizeCanvas();
}

void CWhorldView::SetZoom(double Zoom)
{
	double	dz = Zoom / m_st.Zoom;	// delta zoom
	static const DPOINT DPZERO = {0, 0};
	DPOINT	zorg = m_ZoomType == ZT_WND_CENTER ? DPZERO : m_st.MirrorOrg;
	POSITION	pos = m_Ring.GetHeadPosition();
	while (pos != NULL) {
		RING&	rp = m_Ring.GetNext(pos);
		rp.Origin.x = (rp.Origin.x - zorg.x) * dz + zorg.x;
		rp.Origin.y = (rp.Origin.y - zorg.y) * dz + zorg.y;
	}
	if (m_ZoomType == ZT_WND_CENTER) {
		m_st.MirrorOrg.x *= dz;
		m_st.MirrorOrg.y *= dz;
	}
	m_st.Zoom = Zoom;	// order matters: SetWndSize uses m_st.Zoom
	SetWndSize(m_Size);
	if (m_Paused)
		Invalidate();
}

void CWhorldView::SetNormOrigin(const DPOINT& pt)
{
	m_st.NormOrg = pt;
	NormToMirror(m_st.NormOrg, m_st.MirrorOrg);	// update mirror origin
	UpdateClientOrg();
	if (m_Paused)
		Invalidate();
}

void CWhorldView::SetOrigin(const CPoint& pt)
{
	DPOINT	org = {double(pt.x) / m_Size.cx, double(pt.y) / m_Size.cy};
	SetNormOrigin(org);
}

void CWhorldView::SetGlobalNormOrigin(const DPOINT& pt)
{
	DPOINT	delta = m_st.MirrorOrg;
	SetNormOrigin(pt);
	delta.x -= m_st.MirrorOrg.x;
	delta.y -= m_st.MirrorOrg.y;
	POSITION	pos = m_Ring.GetHeadPosition();
	for (int i = 0; i < m_Ring.GetCount(); i++) {
		RING&	rp = m_Ring.GetNext(pos);
		rp.Origin.x -= delta.x;
		rp.Origin.y -= delta.y;
	}
}

void CWhorldView::SetGlobalOrigin(const CPoint& pt)
{
	DPOINT	org = {double(pt.x) / m_Size.cx, double(pt.y) / m_Size.cy};
	SetGlobalNormOrigin(org);
}

void CWhorldView::SetConvex(bool Enable)
{
	m_st.Convex = Enable;
	if (m_Paused)
		Invalidate();
}

void CWhorldView::SetHue(double Hue)
{
	if (m_st.HueLoop) {
		m_st.HueLoopBase = float(Hue);
		m_st.HueLoopPos = 0;
	} else
		m_st.Hue = Hue;
}

CSize CWhorldView::GetPaneSize() const
{
	if (m_st.Mirror)	// round up, else odd sizes cause an unpainted edge
		return(CSize((m_Size.cx + 1) / 2, (m_Size.cy + 1) / 2));
	return(m_Size);
}

void CWhorldView::InvertDrawMode(int ModeMask)
{
	POSITION	pos = m_Ring.GetHeadPosition();
	for (int i = 0; i < m_Ring.GetCount(); i++)
		m_Ring.GetNext(pos).DrawMode ^= ModeMask;
	m_st.DrawMode ^= ModeMask;
	if (m_Paused)
		Invalidate();
}

void CWhorldView::SetDrawModeAllRings(int Mode)
{
	POSITION	pos = m_Ring.GetHeadPosition();
	for (int i = 0; i < m_Ring.GetCount(); i++)
		m_Ring.GetNext(pos).DrawMode = static_cast<short>(Mode);
	m_st.DrawMode = Mode;
	if (m_Paused)
		Invalidate();
}

void CWhorldView::Mirror(bool Enable)
{
	m_st.Mirror = Enable;
#ifndef WHORLDFF
	m_gdi.SetMirror(Enable);
	SendMessage(WM_SIZE, 0, MAKELONG(m_Size.cx, m_Size.cy));
#endif
	if (m_Paused)
		Invalidate();	// size msg doesn't invalidate unless size changed
}

void CWhorldView::RotateHue(double Degrees)
{
	POSITION	pos = m_Ring.GetHeadPosition();
	while (pos != NULL) {
		RING&	Ring = m_Ring.GetNext(pos);
		double	r, g, b;
		Ring.Hue = Ring.Hue + Degrees;
		if (Ring.Hue > 360)
			Ring.Hue -= 360;
		else if (Ring.Hue < 0)
			Ring.Hue += 360;
		CHLS::hls2rgb(Ring.Hue, Ring.Lightness, Ring.Saturation, r, g, b);
		Ring.Color = RGB(round(r * 255), round(g * 255), round(b * 255)) 
			^ m_st.InvertMask;
	}
	if (m_st.HueLoop)
		m_st.HueLoopBase += float(Degrees);
	else
		m_st.Hue += Degrees;
	if (m_Paused)
		Invalidate();
}

void CWhorldView::InvertColor(bool Enable)
{
	if (Enable == IsColorInverted())
		return;	// nothing to do
	POSITION	pos = m_Ring.GetHeadPosition();
	while (pos != NULL) {
		RING&	Ring = m_Ring.GetNext(pos);
		Ring.Color ^= 0xffffff;
	}
	m_st.InvertMask = Enable ? 0xffffff : 0;
	if (m_Paused)
		Invalidate();
}

void CWhorldView::AddRing()
{
	m_Parms.RingSpacing = max(m_Parms.RingSpacing, 1);
	m_Parms.PolySides = CLAMP(m_Parms.PolySides, 3, MAX_SIDES);
	m_Parms.Lightness = CLAMP(m_Parms.Lightness, 0, 1);
	m_Parms.Saturation = CLAMP(m_Parms.Saturation, 0, 1);
	double	r, g, b;
	CHLS::hls2rgb(m_st.Hue, m_Parms.Lightness, m_Parms.Saturation, r, g, b);
	m_st.Color = RGB(round(r * 255), round(g * 255), round(b * 255));
	RING	Ring;
	Ring.Reverse = m_Parms.RingGrowth < 0;
	double	Offset = Ring.Reverse ? -m_st.RingOffset : m_st.RingOffset;
	Ring.RotDelta = m_Parms.RotateSpeed;
	Ring.Rot = Ring.RotDelta * Offset - Ring.RotDelta * m_NewGrowth;
	if (m_Parms.RingGrowth >= 0)
		Ring.Steps = m_st.RingOffset - m_NewGrowth;
	else {	// if inward growth, start at outermost edge of canvas
		Ring.Steps = (max(m_Canvas.Width(), m_Canvas.Height()) / 2 
			/ m_st.Zoom - m_st.RingOffset) - m_NewGrowth;
	}
	// don't let rings get too flat, or they'll never die
	r = max(pow(2, m_Parms.AspectRatio), MIN_ASPECT_RATIO);
	Ring.Scale.x = r > 1 ? r : 1;
	Ring.Scale.y = r < 1 ? 1 / r : 1;
	Ring.ShiftDelta.x = sin(m_Parms.SkewAngle) * m_Parms.SkewRadius;
	Ring.ShiftDelta.y = cos(m_Parms.SkewAngle) * m_Parms.SkewRadius;
	Ring.Shift.x = Ring.ShiftDelta.x * Offset - Ring.ShiftDelta.x * m_NewGrowth;
	Ring.Shift.y = Ring.ShiftDelta.y * Offset - Ring.ShiftDelta.y * m_NewGrowth;
	if (m_Copying) {
		Ring.Shift.x += sin(m_CopyTheta) * m_CopySpread;
		Ring.Shift.y += cos(m_CopyTheta) * m_CopySpread;
		m_CopyTheta += m_CopyDelta;
	}
	Ring.Color = m_st.Color ^ m_st.InvertMask;
	Ring.Sides = static_cast<short>(round(m_Parms.PolySides));
	Ring.Delete = FALSE;
	// don't let stars get too thin, or they'll never die
	r = max(pow(2, m_Parms.StarFactor), MIN_STAR_RATIO);
	// use trig to find distance from origin to middle of a side
	// b is unknown, A is at origin, c is radius, b = cos(A) * c
	Ring.StarRatio = cos(PI / Ring.Sides) * r;
	Ring.Pinwheel = PI / Ring.Sides * m_Parms.Pinwheel;
	Ring.LineWidth = static_cast<short>(round(m_Parms.LineWidth));
	if (Ring.LineWidth == 1)
		Ring.LineWidth = 0;	// use DC pen, it's much faster
	Ring.DrawMode = static_cast<short>(m_st.DrawMode);
	Ring.Spacing = float(m_Parms.RingSpacing);
	Ring.Hue = m_st.Hue;
	Ring.Lightness = float(m_Parms.Lightness);
	Ring.Saturation = float(m_Parms.Saturation);
	Ring.Origin = m_st.MirrorOrg;
	Ring.EvenCurve = float(m_Parms.EvenCurve);
	Ring.OddCurve = float(m_Parms.OddCurve / Ring.StarRatio);
	Ring.EvenShear = float(m_Parms.EvenShear);
	Ring.OddShear = float(m_Parms.OddShear);
	// add ring to list
	if (Ring.Reverse)
		m_Ring.AddTail(Ring);
	else
		m_Ring.AddHead(Ring);
	while (m_Ring.GetCount() > m_MaxRings) {
		if (Ring.Reverse)
			m_Ring.RemoveHead();
		else
			m_Ring.RemoveTail();
	}
}

inline double CWhorldView::Wrap(double Val, double Limit)
{
	double	r = fmod(Val, Limit);
	return(Val < 0 ? r + Limit : r);
}

inline double CWhorldView::Reflect(double Val, double Limit)
{
	double	m = Limit * 2;
	double	r = Wrap(Val, m);
	return(r < Limit ? r : m - r);
}

inline void CWhorldView::UpdateHue(double DeltaTick)
{
	if (m_st.HueLoop) {
		if (m_st.HueLoopLength) {
			m_st.HueLoopPos += m_Parms.ColorSpeed * DeltaTick;
			m_st.Hue = m_st.HueLoopBase + 
				Reflect(m_st.HueLoopPos, m_st.HueLoopLength);
		} else
			m_st.Hue = m_st.HueLoopBase;
	} else
		m_st.Hue += m_Parms.ColorSpeed * DeltaTick;
	m_st.Hue = Wrap(m_st.Hue, 360);
}

void CWhorldView::TimerHook(const CParmInfo& Info, const PARMS& GlobParm, double Speed)
{
#ifndef WHORLDFF
	if (m_VideoList.IsPlaying())
		m_VideoList.GetCurVideo().TimerHook();
#endif
	double	PrevClock[ROWS];
	int	i;
	for (i = 0; i < ROWS; i++) {
		PrevClock[i] = m_Osc[i].GetClock();
		m_Osc[i].SetWaveform(Info.m_Row[i].Wave);
		m_Osc[i].SetPulseWidth(Info.m_Row[i].PW);
	}
	if (m_FlushHistory) {
		m_PrevInfo = Info;	// suppress interpolation
		for (i = 0; i < ROWS; i++)
			m_Osc[i].SetFreq(Info.m_Row[i].Freq);
		m_FlushHistory = FALSE;
	}
	static const int RG = RING_GROWTH;
	const ROW	*cur = &Info.m_Row[RG];
	m_Osc[RG].SetClock(PrevClock[RG] + 1);
	if (cur->Freq != m_PrevInfo.m_Row[RG].Freq)
		m_Osc[RG].SetFreq(cur->Freq);
	m_NewGrowth = (cur->Val + m_Osc[RG].GetVal() * cur->Amp) * Speed;
	double	AbsGrowth = fabs(m_NewGrowth);
	double	PrevFracTick = 0;
	while (m_st.RingOffset > 0) {
		double	FracTick = AbsGrowth ? (1 - m_st.RingOffset / AbsGrowth) : 0;
		for (int i = 0; i < ROWS; i++) {
			const ROW	*prv = &m_PrevInfo.m_Row[i];
			const ROW	*cur = &Info.m_Row[i];
			m_Osc[i].SetClock(PrevClock[i] + FracTick);
			double	Amp = prv->Amp + (cur->Amp - prv->Amp) * FracTick;
			((double *)&m_Parms)[i] = prv->Val + (cur->Val - prv->Val) * FracTick
				+ m_Osc[i].GetVal() * Amp;
		}
		m_Parms.RingGrowth *= Speed;
		m_Parms.ColorSpeed *= Speed;
		UpdateHue(FracTick - PrevFracTick);
		AddRing();
		m_st.RingOffset -= m_Parms.RingSpacing;
		PrevFracTick = FracTick;
	}
	m_st.RingOffset += AbsGrowth;
	m_Parms.RingGrowth = m_NewGrowth;
	m_Osc[RG].SetClock(PrevClock[RG] + 1);
	for (i = 1; i < ROWS; i++) {	// skip ring growth; assume it's first row
		const ROW	*cur = &Info.m_Row[i];
		m_Osc[i].SetClock(PrevClock[i] + 1);
		if (cur->Freq != m_PrevInfo.m_Row[i].Freq)
			m_Osc[i].SetFreq(cur->Freq);
		((double *)&m_Parms)[i] = cur->Val + m_Osc[i].GetVal() * cur->Amp;
	}
	m_Parms.ColorSpeed *= Speed;
	UpdateHue(1 - PrevFracTick);
	double	r, g, b;
	m_Parms.BkLightness = CLAMP(m_Parms.BkLightness, 0, 1);
	m_Parms.BkSaturation = CLAMP(m_Parms.BkSaturation, 0, 1);
	CHLS::hls2rgb(m_Parms.BkHue, m_Parms.BkLightness, m_Parms.BkSaturation, r, g, b);
	m_st.BkColor = RGB(round(r * 255), round(g * 255), round(b * 255));
	m_PrevInfo = Info;
	// update rings
	POSITION	NextPos = m_Ring.GetHeadPosition();
	DPOINT	PrevOrg = m_st.MirrorOrg;
	double	Trail = 1 - m_st.Trail;
	bool	Reverse = m_Parms.RingGrowth < 0;
	POSITION	CurPos = NULL;
	m_DelPos = NULL;
	while (NextPos != NULL) {
		CurPos = NextPos;
		// update ring origins
		RING&	rp = m_Ring.GetNext(NextPos);
		rp.Origin.x += (PrevOrg.x - rp.Origin.x) * Trail;
		rp.Origin.y += (PrevOrg.y - rp.Origin.y) * Trail;
		PrevOrg = rp.Origin;
		// increment cumulative ring properties
		rp.Steps += m_Parms.RingGrowth;
		rp.Rot += rp.RotDelta * m_Parms.RingGrowth;
		rp.Shift.x += rp.ShiftDelta.x * m_Parms.RingGrowth;
		rp.Shift.y += rp.ShiftDelta.y * m_Parms.RingGrowth;
		if (Reverse) {
			if (rp.Steps <= 0) {
				m_Ring.RemoveAt(CurPos);
			}
		} else {
			if (rp.Delete) {
				m_Ring.RemoveAt(CurPos);
				m_DelPos = NextPos;	// save position after last deletion for cascade
			}
		}
	}
	m_GlobRing.Rot = GlobParm.RotateSpeed / 5 * 180;
	m_GlobRing.StarRatio = max(pow(2, GlobParm.StarFactor), MIN_STAR_RATIO);
	m_GlobRing.Pinwheel = GlobParm.Pinwheel;
	r = max(pow(2, GlobParm.AspectRatio), MIN_ASPECT_RATIO);
	m_GlobRing.Scale.x = r > 1 ? r : 1;
	m_GlobRing.Scale.y = r < 1 ? 1 / r : 1;
	m_GlobRing.Shift.x = sin(GlobParm.SkewAngle) * GlobParm.SkewRadius;
	m_GlobRing.Shift.y = cos(GlobParm.SkewAngle) * GlobParm.SkewRadius;
	m_GlobRing.EvenCurve = float(GlobParm.EvenCurve);
	m_GlobRing.OddCurve = float(GlobParm.OddCurve / m_GlobRing.StarRatio);
	m_GlobRing.EvenShear = float(GlobParm.EvenShear + 1);
	m_GlobRing.OddShear = float(GlobParm.OddShear + 1);
	m_GlobRing.LineWidth = round(GlobParm.LineWidth);
	m_GlobRing.PolySides = round(GlobParm.PolySides);

	Invalidate();
}

void CWhorldView::SetTimerFreq(double Freq)
{
	for (int i = 0; i < ROWS; i++)
		m_Osc[i].SetTimerFreq(Freq);
}

void CWhorldView::StepRings(bool Forward)
{
	double	delta = Forward ? m_Parms.RingGrowth : -m_Parms.RingGrowth;
	POSITION	pos = m_Ring.GetHeadPosition();
	while (pos != NULL) {
		RING&	Ring = m_Ring.GetNext(pos);
		Ring.Steps += delta;
		Ring.Rot += Ring.RotDelta * delta;
		Ring.Shift.x += Ring.ShiftDelta.x * delta;
		Ring.Shift.y += Ring.ShiftDelta.y * delta;
	}
}

void CWhorldView::SetCanvasScale(double Scale)
{
	m_st.CanvasScale = Scale;
	ResizeCanvas();
}

void CWhorldView::SetOptions(int Options)
{
	m_Options = Options;
}

void CWhorldView::LoopHue(bool Enable)
{
	if (Enable != m_st.HueLoop) {
		if (Enable) {	// don't let color cycle any further
			if (m_Parms.ColorSpeed > 0) {
				m_st.HueLoopPos = m_st.HueLoopLength;
				m_st.HueLoopBase = float(m_st.Hue - m_st.HueLoopLength);
			} else {
				m_st.HueLoopPos = 0;
				m_st.HueLoopBase = float(m_st.Hue);
			}
		}
		m_st.HueLoop = Enable;
	}
}

void CWhorldView::SetHueLoopLength(double Length)
{
	if (m_st.HueLoop) {	// try to avoid hue jump
		m_st.HueLoopPos = m_st.HueLoopLength ?
			Reflect(m_st.HueLoopPos, m_st.HueLoopLength) : 0;
	}
	m_st.HueLoopLength = Length;
}

void CWhorldView::SetLineWidth(const CDWordArray& LineWidth)
{
	POSITION	pos = m_Ring.GetHeadPosition();
	for (int i = 0; i < m_Ring.GetCount(); i++)
		m_Ring.GetNext(pos).LineWidth = short(LineWidth[i]);
}

void CWhorldView::GetLineWidth(CDWordArray& LineWidth) const
{
	POSITION	pos = m_Ring.GetHeadPosition();
	LineWidth.SetSize(m_Ring.GetCount());
	for (int i = 0; i < m_Ring.GetCount(); i++)
		LineWidth[i] = m_Ring.GetNext(pos).LineWidth;
}

void CWhorldView::ScaleLineWidth(double Scaling, CDWordArray& PrevLineWidth, int& PrevGlobLineWidth)
{
	CDWordArray	LineWidth;
	GetLineWidth(LineWidth);
	PrevLineWidth.Copy(LineWidth);
	PrevGlobLineWidth = m_GlobRing.LineWidth;
	for (int i = 0; i < LineWidth.GetSize(); i++)
		LineWidth[i] = round(max(LineWidth[i], 1) * Scaling);
	SetLineWidth(LineWidth);
	m_GlobRing.LineWidth = round(m_GlobRing.LineWidth * Scaling);
}

void CWhorldView::SetOrigin(const DPOINT_ARRAY& Origin)
{
	POSITION	pos = m_Ring.GetHeadPosition();
	for (int i = 0; i < m_Ring.GetCount(); i++)
		m_Ring.GetNext(pos).Origin = Origin[i];
}

void CWhorldView::ScaleOrigin(double Scaling, DPOINT_ARRAY& PrevOrigin)
{
	PrevOrigin.SetSize(m_Ring.GetCount());
	POSITION	pos = m_Ring.GetHeadPosition();
	for (int i = 0; i < m_Ring.GetCount(); i++) {
		RING&	rp = m_Ring.GetNext(pos);
		PrevOrigin[i] = rp.Origin;
		rp.Origin.x *= Scaling;
		rp.Origin.y *= Scaling;
	}
}

void CWhorldView::SetHLSFromRGB()
{
	POSITION	pos = m_Ring.GetHeadPosition();
	while (pos != NULL) {
		RING&	Ring = m_Ring.GetNext(pos);
		int	c = Ring.Color;
		double	h, l, s;
		CHLS::rgb2hls(GetRValue(c) / 255.0, GetGValue(c) / 255.0, 
			GetBValue(c) / 255.0, h, l, s);
		Ring.Hue = h;
		Ring.Lightness = float(l);
		Ring.Saturation = float(s);
	}
}

void CWhorldView::SetCopyCount(int Count)
{
	m_CopyCount = Count;
	if (Count > 1) {
		m_CopyDelta = PI * 2 / Count;
		m_CopyTheta = m_CopyDelta / 2;
		m_Copying = m_CopySpread != 0;
	} else
		m_Copying = FALSE;
}

void CWhorldView::SetCopySpread(int Spread)
{
	m_CopySpread = Spread;
	m_Copying = Spread != 0 && m_CopyCount > 1;
}

/////////////////////////////////////////////////////////////////////////////
// CWhorldView drawing

static inline void QWCopyDown(void *dest, void *src, DWORD count)
{
	// overlap is OK provided dest >= src; buffers must be dword-aligned
	__asm {
		mov		ecx, count	// number of qwords to copy
		shl		ecx, 1		// convert from qwords to dwords
		mov		eax, ecx
		dec		eax
		shl		eax, 2		// byte offset of last dword
		mov		edi, dest
		add		edi, eax	// pointer to last dword in dest
		mov		esi, src
		add		esi, eax	// pointer to last dword in src
		std					// copy down in case of overlap
		rep		movsd
		cld					// restore direction flag
	}
}

static inline bool ftest(const float& f)	// efficiently test float for non-zero
{
	return(*((int *)&f) != 0);	// assume float zero == int zero
}

static inline void FastOffsetRect(RECT& r, const POINT& pt)
{
	r.left += pt.x;
	r.top += pt.y;
	r.right += pt.x;
	r.bottom += pt.y;
}

static inline BOOL FastPtInRect(const RECT& r, const POINT& pt)
{
	return(pt.x >= r.left && pt.y >= r.top && pt.x < r.right && pt.y < r.bottom);
}

void CWhorldView::Draw(HDC dc)
{
	SetBkColor(dc, m_st.BkColor);
	CRect	r(0, 0, m_Size.cx, m_Size.cy);
	ExtTextOut(dc, 0, 0, ETO_OPAQUE, r, NULL, 0, NULL);	// as in CDC::FillSolidRect
	bool	Convex = m_st.Convex;
	POSITION	HeadPos = m_Ring.GetHeadPosition();
	POSITION	NextPos = Convex ? m_Ring.GetTailPosition() : HeadPos;
	POSITION	CurPos = NULL;
	bool	GotDCPen = FALSE;
	int		PrevDrawMode = -1;
	char	PrevReverse = -1;
	double	PrevRadius = 1e9;
	bool	Reverse = m_Parms.RingGrowth < 0;
	CPoint	WndCenter(m_Size.cx / 2, m_Size.cy / 2);
	bool	PrevCurved = FALSE;	// true if previous ring has curves
	int		Points;				// number of points in current ring
	int		PrevPoints = 0;		// number of points in previous ring
	int		PrevColor = 0;
	while (NextPos != NULL) {
		CurPos = NextPos;
		RING&	rp = Convex ? m_Ring.GetPrev(NextPos) : m_Ring.GetNext(NextPos);
		DPOINT	org;
		org.x = WndCenter.x + rp.Origin.x;
		org.y = WndCenter.y + rp.Origin.y;
		CPoint	irorg(round(rp.Origin.x), round(rp.Origin.y));
		CRect	Bounds(m_Canvas);
		FastOffsetRect(Bounds, irorg);
		int		sides = rp.Sides + m_GlobRing.PolySides;
		sides = max(sides, 1);
		int		LineWidth = rp.LineWidth + m_GlobRing.LineWidth;
		int		DrawMode = rp.DrawMode;
		double	tr = rp.Rot + m_GlobRing.Rot;
		double	rot[2] = {tr, tr + rp.Pinwheel + PI / sides * m_GlobRing.Pinwheel};
		double	steps = rp.Steps * m_st.Zoom;
		double	rad[2] = {steps, steps * rp.StarRatio * m_GlobRing.StarRatio};
		double	xscale = rp.Scale.x * m_GlobRing.Scale.x;
		double	yscale = rp.Scale.y * m_GlobRing.Scale.y;
		double	xrad[2] = {rad[0] * xscale, rad[1] * xscale};
		double	yrad[2] = {rad[0] * yscale, rad[1] * yscale};
		double	xshift = (rp.Shift.x * m_st.Zoom + m_GlobRing.Shift.x * steps) + org.x;
		double	yshift = (rp.Shift.y * m_st.Zoom + m_GlobRing.Shift.y * steps) + org.y;
		double	delta = PI / sides;
		int		vertices = sides << 1;	// two vertices for each side
		bool	RingVisible = FALSE;
		float	CurveLenCW[2];
		float	CurveLenCCW[2];
		CurveLenCW[0] = rp.EvenCurve + m_GlobRing.EvenCurve;
		CurveLenCW[1] = rp.OddCurve + m_GlobRing.OddCurve;
		bool	Curved = ftest(CurveLenCW[0]) || ftest(CurveLenCW[1]);
		if (Curved) {
			Points = vertices * 3 + 2;	// one extra for duplicate control point
			if (DrawMode & DM_FILL)
				QWCopyDown(m_pa + Points, m_pa, PrevPoints);
			DPoint	CurveOrigin(xshift, yshift);	// use skewed origin
			CurveLenCCW[0] = CurveLenCW[0] * (rp.EvenShear + m_GlobRing.EvenShear);
			CurveLenCCW[1] = CurveLenCW[1] * (rp.OddShear + m_GlobRing.OddShear);
			POINT	*pp = m_pa;	// pointer to array of points
			for (int i = 0; i < vertices; i++) {	// innermost loop
				int	odd = i & 1;
				double	theta = delta * i + rot[odd];
				DPoint	pt;	// compute curve points from real vertex
				pt.x = sin(theta) * xrad[odd] + xshift;
				pt.y = cos(theta) * yrad[odd] + yshift;
				POINT	ipt = pt;	// round to integer point
				if (FastPtInRect(Bounds, ipt))	// avoid function call
					RingVisible = TRUE;
				DPoint	cv = pt - CurveOrigin;	// vector from vertex to origin
				double	r = CurveLenCW[odd];	// get clockwise curve vector length
				// previous curve's 2nd control point
				*pp++ = CPoint(round(pt.x - cv.y * r), round(pt.y + cv.x * r));
				*pp++ = ipt;	// current curve's start point
				r = CurveLenCCW[odd];	// get counter-clockwise curve vector length
				// current curve's 1st control point
				*pp++ = CPoint(round(pt.x + cv.y * r), round(pt.y - cv.x * r));
			}
			*pp++ = m_pa[0];	// copy final control point
			*pp++ = m_pa[1];	// close the shape
		} else {
			Points = vertices;
			if (DrawMode & DM_FILL)
				QWCopyDown(m_pa + Points, m_pa, PrevPoints);
			for (int i = 0; i < vertices; i++) {	// innermost loop
				int	odd = i & 1;
				double	theta = delta * i + rot[odd];
				POINT	pt;
				pt.x = round(sin(theta) * xrad[odd] + xshift);
				pt.y = round(cos(theta) * yrad[odd] + yshift);
				m_pa[i] = pt;
				if (FastPtInRect(Bounds, pt))	// avoid function call
					RingVisible = TRUE;
			}
		}
		rp.Delete = !RingVisible;
		if (DrawMode != PrevDrawMode) {
			if (DrawMode & DM_FILL) {
				SelectObject(dc, GetStockObject(DC_BRUSH));
				if (!(DrawMode & DM_OUTLINE))
					SelectObject(dc, GetStockObject(NULL_PEN));
			}
			SetROP2(dc, (DrawMode & DM_XRAY) ? R2_XORPEN : R2_COPYPEN);
			PrevDrawMode = DrawMode;
			GotDCPen = FALSE;
		}
		if (DrawMode & DM_FILL) {
			if (DrawMode & DM_OUTLINE) {
				if (LineWidth > 0) {
					m_Pen.DeleteObject();
					m_Pen.CreatePen(PS_SOLID, LineWidth, m_st.BkColor);
					SelectObject(dc, m_Pen);
					GotDCPen = FALSE;
				} else {
					SetDCPenColor(dc, m_st.BkColor);
					if (!GotDCPen) {
						SelectObject(dc, GetStockObject(DC_PEN));
						GotDCPen = TRUE;
					}
				}
			}
			SetDCBrushColor(dc, Convex ? PrevColor : rp.Color);
			if (Curved || PrevCurved) {
				if (char(rp.Reverse) == PrevReverse) {
					BeginPath(dc);
					if (Curved) {
						PolyBezier(dc, &m_pa[1], Points - 1);
					} else {
						m_pa[Points] = m_pa[0];	// close the shape
						Polyline(dc, m_pa, Points + 1);
					}
					if (PrevCurved) {
						PolyBezier(dc, &m_pa[Points + 1], PrevPoints - 1);
					} else {
						m_pa[Points + PrevPoints] = m_pa[Points];	// close the shape
						Polyline(dc, &m_pa[Points], PrevPoints + 1);
					}
					EndPath(dc);
					StrokeAndFillPath(dc);
				}
				if (Reverse) {
					if (rp.Steps < rp.Spacing * 2 && rp.Steps < PrevRadius) {
						BeginPath(dc);
						if (Convex)
							SetDCBrushColor(dc, rp.Color);
						if (Curved) {
							PolyBezier(dc, &m_pa[1], Points - 1);	// fill innermost ring
						} else {
							m_pa[Points] = m_pa[0];	// close the shape
							Polyline(dc, m_pa, Points + 1);
						}
						EndPath(dc);
						StrokeAndFillPath(dc);
					}
				} else {
					if (CurPos == HeadPos) {
						BeginPath(dc);
						if (Convex)
							SetDCBrushColor(dc, rp.Color);
						if (Curved) {
							PolyBezier(dc, &m_pa[1], Points - 1);	// fill innermost ring
						} else {
							m_pa[Points] = m_pa[0];	// close the shape
							Polyline(dc, m_pa, Points + 1);
						}
						EndPath(dc);
						StrokeAndFillPath(dc);
					}
				}
			} else {
				if (PrevPoints && char(rp.Reverse) == PrevReverse)  {
					int	sa[2] = {Points, PrevPoints};
					PolyPolygon(dc, m_pa, sa, 2);
				}
				if (Reverse) {
					if (rp.Steps < rp.Spacing * 2 && rp.Steps < PrevRadius) {
						if (Convex)
							SetDCBrushColor(dc, rp.Color);
						Polygon(dc, m_pa, Points);	// fill innermost ring
					}
				} else {
					if (CurPos == HeadPos) {
						if (Convex)
							SetDCBrushColor(dc, rp.Color);
						Polygon(dc, m_pa, Points);	// fill innermost ring
					}
				}
			}
		} else {
			if (LineWidth > 0) {
				m_Pen.DeleteObject();
				m_Pen.CreatePen(PS_SOLID, LineWidth, rp.Color);
				SelectObject(dc, m_Pen);
				GotDCPen = FALSE;
			} else {
				SetDCPenColor(dc, rp.Color);
				if (!GotDCPen) {
					SelectObject(dc, GetStockObject(DC_PEN));
					GotDCPen = TRUE;
				}
			}
			if (Curved) {
				PolyBezier(dc, &m_pa[1], Points - 1);
			} else {
				m_pa[Points] = m_pa[0];	// close the shape
				Polyline(dc, m_pa, Points + 1);
			}
		}
		PrevPoints = Points;
		PrevRadius = rp.Steps;
		PrevReverse = rp.Reverse;
		PrevColor = rp.Color;
		PrevCurved = Curved;
	}
	if (m_DelPos != NULL) {
		RING&	rp = m_Ring.GetNext(m_DelPos);
		rp.Delete = TRUE;	// cascade delete
	}
#ifndef WHORLDFF
	if (m_VideoList.IsPlaying()) {
		CVideo	*vp = &m_VideoList.GetCurVideo();
		vp->UpdateSurface();
		CSize	FrmSz(m_Size);
		CSize	SrcSz(vp->GetFrameSize());
		CPoint	SrcPt(0, 0);
		if (m_st.Mirror) {	// only need to blit upper-left quadrant
			FrmSz.cx = (FrmSz.cx + 1) >> 1;
			FrmSz.cy = (FrmSz.cy + 1) >> 1;
			SrcSz.cx = (SrcSz.cx + 1) >> 1;
			SrcSz.cy = (SrcSz.cy + 1) >> 1;
			SrcPt.x = SrcSz.cx - round(SrcSz.cx * m_st.VideoOrg.x);
			SrcPt.y = round(SrcSz.cy * m_st.VideoOrg.y);
		}
		HDC	sdc;
		vp->GetDC(&sdc);
		StretchBlt(dc, 0, FrmSz.cy - 1, FrmSz.cx, -FrmSz.cy, sdc,
			SrcPt.x, SrcPt.y, SrcSz.cx, SrcSz.cy, m_VideoList.GetROP());
		vp->ReleaseDC(sdc);
	}
#endif
}

#ifndef WHORLDFF

void CWhorldView::OnDraw(CDC* pDC)
{
	Draw(m_gdi.GetDC());
	m_gdi.Blt(pDC);
}

void CWhorldView::SetVideoROP(int ROPIdx)
{
	ROPIdx = CLAMP(ROPIdx, 0, CVideoList::ROPS - 1);
	m_VideoList.SetROPIdx(ROPIdx);
	if (m_Paused)
		Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CTestView printing

BOOL CWhorldView::OnPreparePrinting(CPrintInfo* pInfo)
{
	pInfo->SetMaxPage(1);	// default to one page
	AfxGetMainWnd()->SendMessage(UWM_VIEWPRINT, PRN_PREPARE, (LPARAM)pInfo);
	BOOL	retc = DoPreparePrinting(pInfo);
	if (!retc)
		AfxGetMainWnd()->SendMessage(UWM_VIEWPRINT, PRN_CANCEL, (LPARAM)pInfo);
	return(retc);
}

void CWhorldView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	AfxGetMainWnd()->SendMessage(UWM_VIEWPRINT, PRN_BEGIN, (LPARAM)pInfo);
}

void CWhorldView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	CView::OnPrepareDC(pDC, pInfo);
	if (pInfo != NULL) {
		pDC->SetMapMode(MM_ISOTROPIC);	
		pDC->SetWindowExt(m_Size.cx, -m_Size.cy);	// printing is upside-down
	}
}

void CWhorldView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	AfxGetMainWnd()->SendMessage(UWM_VIEWPRINT, PRN_PRINT, (LPARAM)pInfo);
	// Drawing directly to a printer DC takes advantage of the printer's higher
	// resolution, but it doesn't always reproduce the screen colors accurately
	// in XOR mode.  We may prefer to trade smooth edges for color fidelity, by 
	// drawing to a screen-resolution bitmap and blitting it to the printer DC.
	if (m_Options & OPT_PRINT_FROM_BITMAP) {
		CDC	dc;
		CBitmap	bm;
		dc.CreateCompatibleDC(pDC);	// printer-compatible DC
		bm.CreateCompatibleBitmap(pDC, m_Size.cx, m_Size.cy);	// screen-res bitmap
		dc.SelectObject(&bm);
		if (m_st.Mirror) {	// printer-compatible DC doesn't support mirror blit
			HBITMAP	dib = MakeDIB();	// draw to device-independent bitmap
			if (dib != NULL) {
				dc.SelectObject(dib);
				pDC->BitBlt(0, 0, m_Size.cx, m_Size.cy, &dc, 0, 0, SRCCOPY);
				DeleteObject(dib);
			}
		} else {
			Draw(dc.m_hDC);
			pDC->BitBlt(0, 0, m_Size.cx, m_Size.cy, &dc, 0, 0, SRCCOPY);
		}
	} else {	// draw directly to printer DC
		// DC pen doesn't scale; force line widths to be > 0 so we don't use it
		CDWordArray	PrevLineWidth;
		int	PrevGlobLineWidth;
		ScaleLineWidth(1, PrevLineWidth, PrevGlobLineWidth);
		if (m_st.Mirror) {	// manipulate window extent and origin to make mirrors
			// when multiple pages are printed, the same DC object is passed
			// for each page, so the following operations must be repeatable
			CSize	ps = GetPaneSize();
			CRect	r(0, 0, ps.cx, ps.cy);
			pDC->IntersectClipRect(r);	// clip to a single pane
			m_st.Mirror = FALSE;	// else video overlay leaves blank lines
			Draw(pDC->m_hDC);	// upper left pane
			pDC->SetWindowExt(-m_Size.cx, -m_Size.cy);
			pDC->SetWindowOrg(m_Size.cx, 0);
			pDC->OffsetClipRgn(-ps.cx, 0);
			Draw(pDC->m_hDC);	// upper right pane
			pDC->SetWindowExt(m_Size.cx, m_Size.cy);
			pDC->SetWindowOrg(0, m_Size.cy);
			pDC->OffsetClipRgn(-ps.cx, -ps.cy);
			Draw(pDC->m_hDC);	// lower left pane
			pDC->SetWindowExt(-m_Size.cx, m_Size.cy);
			pDC->SetWindowOrg(m_Size.cx, m_Size.cy);
			pDC->OffsetClipRgn(-ps.cx, 0);
			Draw(pDC->m_hDC);	// lower right pane
			pDC->SetWindowExt(m_Size.cx, -m_Size.cy);
			pDC->SetWindowOrg(0, 0);
			pDC->OffsetClipRgn(-ps.cx, -ps.cy);
			m_st.Mirror = TRUE;
			// we're back where we started
		} else {
			CRect	r(0, 0, m_Size.cx, m_Size.cy);
			pDC->IntersectClipRect(r);	// our rect can be smaller than page
			Draw(pDC->m_hDC);
		}
		if (PrevLineWidth.GetSize()) {	// if line widths were changed
			SetLineWidth(PrevLineWidth);	// restore them
			m_GlobRing.LineWidth = PrevGlobLineWidth;
		}
	}
}

void CWhorldView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	AfxGetMainWnd()->SendMessage(UWM_VIEWPRINT, PRN_END, (LPARAM)pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CWhorldView export

HBITMAP	CWhorldView::MakeDIB(const CSize *DstSize, const CSize *SrcSize, bool ScaleToFit)
{
	CSize	PrevSize;
	double	PrevZoom = 0;	// prevents warning
	CDWordArray	PrevLineWidth;
	int	PrevGlobLineWidth = 0;	// prevents warning
	DPOINT_ARRAY	PrevOrigin;
	if (DstSize != NULL) {
		PrevSize = m_Size;
		PrevZoom = m_st.Zoom;
		if (ScaleToFit) {
			if (SrcSize == NULL)
				SrcSize = &m_Size;	// source size defaults to current window size
			double	Scaling = double(DstSize->cx) / SrcSize->cx;	// get scale from x-axis
			ScaleLineWidth(Scaling, PrevLineWidth, PrevGlobLineWidth);
			ScaleOrigin(Scaling, PrevOrigin);
			m_st.Zoom = PrevZoom * Scaling;
		}
		SetWndSize(*DstSize);
	}
	CClientDC	ViewDC(this);
	CDC	dc;
	HBITMAP	dib = NULL;
	if (dc.CreateCompatibleDC(&ViewDC)) {
		BITMAPINFO	bi;
		ZeroMemory(&bi, sizeof(BITMAPINFO));
		bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth = m_Size.cx;
		bi.bmiHeader.biHeight = m_Size.cy;
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = 24;	// true color: no palette
		dib = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, NULL, NULL, 0);
		if (dib != NULL) {
			HGDIOBJ	PrevObj = SelectObject(dc, dib);
			if (PrevObj != NULL) {
				Draw(dc.m_hDC);
				if (m_st.Mirror) {
					CSize	ps = GetPaneSize();
					int	w = ps.cx;
					int	h = ps.cy;
					StretchBlt(dc, w, 0, w, h, dc, w - 1, 0, -w, h, SRCCOPY);
					StretchBlt(dc, 0, h, w, h, dc, 0, h - 1, w, -h, SRCCOPY);
					StretchBlt(dc, w, h, w, h, dc, w - 1, h - 1, -w, -h, SRCCOPY);
				}
				SelectObject(dc, PrevObj);	// restore default object
			} else {
				DeleteObject(dib);
				dib = NULL;
			}
		}
	}
	if (DstSize != NULL) {
		if (PrevLineWidth.GetSize())
			SetLineWidth(PrevLineWidth);
		m_GlobRing.LineWidth = PrevGlobLineWidth;
		if (PrevOrigin.GetSize())
			SetOrigin(PrevOrigin);
		m_st.Zoom = PrevZoom;
		SetWndSize(PrevSize);
	}
	return(dib);
}

bool CWhorldView::ExportBitmap(CFile& File, const CSize *DstSize, const CSize *SrcSize, 
							   bool ScaleToFit, int Resolution)
{
	bool	retc = FALSE;
	HBITMAP	dib = MakeDIB(DstSize, SrcSize, ScaleToFit);
	if (dib != NULL) {
		DIBSECTION	ds;
		if (GetObject(dib, sizeof(DIBSECTION), &ds)) {
			BITMAPFILEHEADER	bfh;
			ZeroMemory(&bfh, sizeof(BITMAPFILEHEADER));
			bfh.bfType = 0x4d42;	// BM
			bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
			bfh.bfSize = bfh.bfOffBits + ds.dsBmih.biSizeImage;
			static const double INCHES_PER_METER = .0254;
			int	MetricRes = round(Resolution / INCHES_PER_METER);
			ds.dsBmih.biXPelsPerMeter = MetricRes;
			ds.dsBmih.biYPelsPerMeter = MetricRes;
			TRY {
				File.Write(&bfh, sizeof(BITMAPFILEHEADER));
				File.Write(&ds.dsBmih, sizeof(BITMAPINFOHEADER));
				File.Write(ds.dsBm.bmBits, ds.dsBmih.biSizeImage);
				retc = TRUE;
			}
			CATCH(CFileException, e)
			{
			}
			END_CATCH
		}
		DeleteObject(dib);
	}
	return(retc);
}

/////////////////////////////////////////////////////////////////////////////
// CWhorldView serialization

void CWhorldView::ThrowBadFormat(CArchive &ar)
{
	AfxThrowArchiveException(CArchiveException::badIndex, ar.m_strFileName);
}

void CWhorldView::Serialize(CArchive &ar)
{
	// if new members are added here, MINI_SNAP_INTS and GetMiniSnapshotSize
	// may need updating; the failure symptoms include bad format exceptions
	// thrown while seeking within a movie, or upon exiting the movie player
	if (ar.IsStoring()) {
		ar << SNAPSHOT_SIG;
		ar << SNAPSHOT_VERSION;
		ar << sizeof(PARMS);
		ar.Write(&m_Parms, sizeof(PARMS));
		if (m_MiniRings) {
			ar << MINI_RING_SIZE;
			ar << m_Ring.GetCount();
			POSITION	pos = m_Ring.GetHeadPosition();
			for (int i = 0; i < m_Ring.GetCount(); i++)
				ar.Write(((char *)&m_Ring.GetNext(pos)) + MINI_RING_OFS, MINI_RING_SIZE);
			int	VideoIdx = -1;
			DWORD	VideoFrame = 0;
			if (m_VideoList.IsPlaying()) {
				VideoFrame = m_VideoList.GetCurVideo().GetCurFrame();
				VideoIdx = m_VideoList.GetHistoryIdx();
			}
			ar << VideoIdx;
			ar << VideoFrame;
		} else {
			ar << sizeof(RING);
			ar << m_Ring.GetCount();
			POSITION	pos = m_Ring.GetHeadPosition();
			for (int i = 0; i < m_Ring.GetCount(); i++)
				ar.Write(&m_Ring.GetNext(pos), sizeof(RING));
			CString	VideoPath;
			DWORD	VideoFrame = 0;
			if (m_VideoList.IsPlaying()) {
				CVideo&	vp = m_VideoList.GetCurVideo();
				VideoPath = vp.GetPath();
				VideoFrame = vp.GetCurFrame();
			}
			ar << VideoPath;
			ar << VideoFrame;
		}
		m_st.VideoROPIdx = static_cast<BYTE>(m_VideoList.GetROPIdx());
		ar << sizeof(STATE);
		ar.Write(&m_st, sizeof(STATE));
		ar << sizeof(GLOBRING);
		ar.Write(&m_GlobRing, sizeof(GLOBRING));
	} else {
		bool	PrevMirror = m_st.Mirror;
		int	Sig, Version, Rings, RingSize, ParmsSize, StateSize;
		ar >> Sig;
		ar >> Version;
		if (Sig != SNAPSHOT_SIG || Version > SNAPSHOT_VERSION)
			ThrowBadFormat(ar);
		ar >> ParmsSize;
		if (ParmsSize > sizeof(PARMS))
			ThrowBadFormat(ar);
		m_Parms = m_DefParms;	// init to default values
		ar.Read(&m_Parms, ParmsSize);
		ar >> RingSize;
		ar >> Rings;
		m_Ring.RemoveAll();
		m_DelPos = NULL;	// avoid bogus delete
		RING	Ring;
		ZeroMemory(&Ring, sizeof(RING));	// init to default values
		if (m_MiniRings) {
			if (RingSize > MINI_RING_SIZE)
				ThrowBadFormat(ar);
			for (int i = 0; i < Rings; i++) {
				ar.Read(((char *)&Ring) + MINI_RING_OFS, RingSize);
				m_Ring.AddTail(Ring);
			}
			if (Version > 3) {	// if video overlay is supported
				int	VideoIdx;
				DWORD	VideoFrame;
				ar >> VideoIdx;
				ar >> VideoFrame;
				if (VideoIdx >= 0)
					m_VideoList.LoadFrame(VideoIdx, VideoFrame);
				else
					m_VideoList.Deselect();
			} else
				m_VideoList.Deselect();
		} else {
			if (RingSize > sizeof(RING))
				ThrowBadFormat(ar);
			for (int i = 0; i < Rings; i++) {
				ar.Read(&Ring, RingSize);
				m_Ring.AddTail(Ring);
			}
			m_VideoList.Deselect();
			if (Version > 3) {	// if video overlay is supported
				CString	VideoPath;
				DWORD	VideoFrame;
				ar >> VideoPath;
				ar >> VideoFrame;
				if (!VideoPath.IsEmpty()) {	// if snapshot has video
					if (m_VideoList.CheckLink(VideoPath)
					&& !m_VideoList.Open(VideoPath, VideoFrame)) {
						CString	s;
						AfxFormatString2(s, IDS_PL_BAD_VIDEO, VideoPath, 
							m_VideoList.GetLastErrorString());
						AfxMessageBox(s);
					}
				}
			}
		}
		ar >> StateSize;
		if (StateSize > sizeof(STATE))
			ThrowBadFormat(ar);
		m_st = m_DefState;	// init to default values
		ar.Read(&m_st, StateSize);
		m_GlobRing = m_DefGlobRing;	// init to default values
		if (Version > 4) {	// if global parameters are supported
			ar >> RingSize;
			if (RingSize > sizeof(GLOBRING))
				ThrowBadFormat(ar);
			ar.Read(&m_GlobRing, RingSize);
		}
		if (Version <= 1) {	// mirror origin coordinates are unzoomed and reversed
			m_st.MirrorOrg.x *= -m_st.Zoom;	// convert to zoomed and non-reversed
			m_st.MirrorOrg.y *= -m_st.Zoom;
			POSITION	pos = m_Ring.GetHeadPosition();
			for (int i = 0; i < Rings; i++)	// per-ring origins didn't exist
				m_Ring.GetNext(pos).Origin = m_st.MirrorOrg;	// make them all the same
		}
		m_VideoList.SetROPIdx(m_st.VideoROPIdx);
		if (m_st.Mirror != PrevMirror)
			Mirror(m_st.Mirror);	// apply new mirror state 
		else {
			SetWndSize(m_Size);	// recalculate origin
			Invalidate();
		}
	}
}

bool CWhorldView::Serialize(CFile& File, bool Load)
{
	TRY {
		CArchive	a(&File, Load ? CArchive::load : CArchive::store);
		a.m_strFileName = File.GetFileName();
		Serialize(a);
	}
	CATCH(CArchiveException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}

bool CWhorldView::Serialize(LPCTSTR Path, bool Load)
{
	CFile	fp;
	CFileException	e;
	int	mode = Load ? (CFile::modeRead | CFile::shareDenyWrite) 
		: (CFile::modeCreate | CFile::modeWrite);
	if (!fp.Open(Path, mode, &e)) {
		e.ReportError();
		return(FALSE);
	}
	return(Serialize(fp, Load));
}

/////////////////////////////////////////////////////////////////////////////
// CWhorldView message handlers

BOOL CWhorldView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CView::PreCreateWindow(cs))
		return FALSE;
	cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), NULL, NULL);	// no background brush
	return TRUE;
}

int CWhorldView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_gdi.Create(this) || !m_gdi.CreateSurface(1, 1)) {
		AfxMessageBox(IDS_GDI_CANT_CREATE);
		return -1;
	}
	if (m_MainView == NULL)	// if first instance, save 'this' for frame
		m_MainView = this;

	return 0;
}

void CWhorldView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
}

void CWhorldView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	SetWndSize(CSize(cx, cy));
	if (!m_gdi.CreateSurface(cx, cy)) {
		CString	s;
		s.Format(IDS_CANT_CREATE_SURFACE, cx, cy);
		AfxMessageBox(s);
	}
}

void CWhorldView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// relay to main frame, except auto-repeats and modifier keys
	if (!(nFlags & KF_REPEAT) && nChar != VK_SHIFT && nChar != VK_CONTROL)
		AfxGetMainWnd()->SendMessage(WM_KEYDOWN, nChar, MAKELONG(nRepCnt, nFlags));
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CWhorldView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// relay to main frame
	AfxGetMainWnd()->SendMessage(WM_CHAR, nChar, MAKELONG(nRepCnt, nFlags));
	CView::OnChar(nChar, nRepCnt, nFlags);
}

#endif
