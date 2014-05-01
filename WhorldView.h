// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		03mar06	add convex
        02      04mar06	add trail, bump snapshot version to 2
		03		15mar06	add SetGlobalOrigin
		04		23mar06	move SetHue to .cpp
		05		28mar06	add zoom type
		06		29mar06	remove origin clamping
		07		13apr06	add even and odd curve, bump snapshot version to 3
		08		18apr06	add freeframe conditionals
		09		06may06	add video overlay
		10		07jun06	include video in snapshot
		11		12jun06	move video ROP into state
		12		24jun06	add Copies and Spread
		13		29jun06	add video origin
		14		15sep06	remove m_TimerDraw, add m_DelPos
		15		10dec07	add global parameters
		16		13dec07	add even and odd shear
		17		15jan08	add globals for line width and poly sides
		18		24jan08	compute curve points from real vertices
		19		27jan08	update GetMiniSnapshotSize for globals
		20		28jan08	support Unicode
		21		30jan08	add OnChar to relay char messages to main frame

        Whorld view

*/

// WhorldView.h : interface of the CWhorldView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WHORLDVIEW_H__4E7A57CC_64AF_47DE_98D1_C3DF8A34C524__INCLUDED_)
#define AFX_WHORLDVIEW_H__4E7A57CC_64AF_47DE_98D1_C3DF8A34C524__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CWhorldView window

#include <afxtempl.h>
#include "BackBufGDI.h"
#include "Oscillator.h"
#include "ParmInfo.h"
#include "VideoList.h"

#ifndef WHORLDFF
class CWhorldView : public CView
{
	DECLARE_DYNCREATE(CWhorldView)
#else
class CWhorldView
{
#endif
// Construction
public:
	CWhorldView();

// Constants
	enum {	// draw modes
		DM_FILL		= 0x01,		// fill area between rings
		DM_OUTLINE	= 0x02,		// outline edges of filled rings
		DM_XRAY		= 0x04		// draw rings with XOR pen
	};
	enum {	// options: high-order word is reserved for derived classes
		OPT_PRINT_FROM_BITMAP	= 0x01	// for color fidelity in XOR mode
	};
	enum {	// printer notifications
		PRN_PREPARE,
		PRN_BEGIN,
		PRN_PRINT,
		PRN_END,
		PRN_CANCEL
	};
	enum {	// zoom types
		ZT_WND_CENTER,	// zoom relative to center of window
		ZT_RING_ORIGIN	// zoom relative to ring origin
	};
	#undef PARMDEF
	#define PARMDEF(Name, Tag, MinVal, MaxVal, Steps, Scale) Tag,
	enum {	// define parameters
		#include "ParmDef.h"
	};

// Types
	// parameters: APPEND ONLY, else pre-existing snapshots will be unreadable
	// all members MUST be double, so that this struct can be cast to an array
	#undef PARMDEF
	#define PARMDEF(Name, Tag, MinVal, MaxVal, Steps, Scale) double Name;
	typedef struct tagPARMS {
		#include "ParmDef.h"
	} PARMS;

// Attributes
	void	GetParms(PARMS& Parms) const;
	void	SetParms(const PARMS& Parms);
	static	void	GetDefaults(PARMS& Parms);
	static	CWhorldView	*GetMainView();
	CSize	GetSize() const;
	int		GetRingCount() const;
	void	SetDrawMode(int Mode);
	void	SetDrawModeAllRings(int Mode);
	void	InvertDrawMode(int ModeMask);
	int		GetDrawMode() const;
	void	SetOrigin(const CPoint& pt);
	CPoint	GetOrigin() const;
	void	SetNormOrigin(const DPOINT& pt);
	void	GetNormOrigin(DPOINT& pt) const;
	void	SetGlobalOrigin(const CPoint& pt);
	void	SetGlobalNormOrigin(const DPOINT& pt);
	void	SetHue(double Hue);
	double	GetHue() const;
	void	SetZoom(double Zoom);
	double	GetZoom() const;
	virtual	void	Mirror(bool Enable);
	bool	IsMirrored() const;
	void	RotateHue(double Degrees);
	void	InvertColor(bool Enable);
	bool	IsColorInverted() const;
	void	GetCanvasRect(CRect& Rect);
	void	SetCanvasScale(double Scale);
	double	GetCanvasScale() const;
	virtual	void	SetOptions(int Options);
	void	LoopHue(bool Enable);
	bool	IsHueLooped() const;
	void	SetHueLoopLength(double Length);
	double	GetHueLoopLength() const;
	void	SetPhase(int Idx, double Phase);
	void	SetTimerFreq(double Freq);
	int		GetMaxRings() const;
	void	SetMaxRings(int Rings);
	bool	GetConvex() const;
	void	SetConvex(bool Enable);
	double	GetTrail() const;
	void	SetTrail(double Trail);
	int		GetZoomType() const;
	void	SetZoomType(int Type);
	bool	GetPause() const;
	void	SetPause(bool Enable);
	CVideoList&	GetVideo();
	int		GetVideoROP() const;
	void	SetVideoROP(int ROPIdx);
	int		GetCopyCount() const;
	void	SetCopyCount(int Count);
	int		GetCopySpread() const;
	void	SetCopySpread(int Spread);
	void	GetVideoOrigin(DPOINT& Origin) const;
	void	SetVideoOrigin(const DPOINT& Origin);

// Operations
	void	TimerHook(const CParmInfo& Info, const PARMS& GlobParm, double Speed);
	void	ClearScreen();
	void	Serialize(CArchive &ar);
	void	MiniSerialize(CArchive &ar);
	bool	Serialize(CFile& File, bool Load);
	bool	Serialize(LPCTSTR Path, bool Load);
	HBITMAP	MakeDIB(const CSize *DstSize = NULL, const CSize *SrcSize = NULL, bool ScaleToFit = FALSE);
	bool	ExportBitmap(CFile& File, const CSize *DstSize, const CSize *SrcSize, bool ScaleToFit, int Resolution);
	void	StepRings(bool Forward);
	void	FlushHistory();
	int		GetMiniSnapshotSize() const;
	void	SetHLSFromRGB();
	void	SetWndSize(CSize sz);
	void	Draw(HDC dc);

// Overrides
#ifndef WHORLDFF
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWhorldView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL
#endif

// Implementation
public:
	virtual ~CWhorldView();

// Generated message map functions
protected:
#ifndef WHORLDFF
	//{{AFX_MSG(CWhorldView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
#else
	void	Invalidate() {}
#endif

// Types
	// ring data: APPEND ONLY, else pre-existing snapshots will be unreadable
	typedef struct tagRING {
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
	} RING;
	// global ring data: APPEND ONLY, else pre-existing snapshots will be unreadable
	typedef struct tagGLOBRING {
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
	} GLOBRING;
	// state data: APPEND ONLY, else pre-existing snapshots will be unreadable
	typedef struct tagSTATE {
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
	} STATE;
	typedef CParmInfo::ROW ROW;
	typedef	CArray<DPOINT, DPOINT&>	DPOINT_ARRAY;

// Constants
	enum {
		ROWS = CParmInfo::ROWS,		// number of parameter rows
		MAX_SIDES = 50,				// maximum number of sides a ring can have
		MAX_VERTICES = MAX_SIDES * 2,	// rings have 2 vertices per side (even/odd)
		MAX_POINTS = MAX_VERTICES * 3 + 2,	// enough for worst-case curved ring
		SNAPSHOT_SIG = 0x4e534857,	// snapshot signature: WHSN (WHorld SNapshot)
		SNAPSHOT_VERSION = 5,		// snapshot version number
		MINI_RING_OFS = offsetof(RING, Rot),	// offset of mini-ring within ring
		MINI_RING_SIZE = sizeof(RING) - MINI_RING_OFS,	// size of mini-ring
		MINI_SNAP_INTS = 9			// number of miscellaneous integers serialized,
									// which GetMiniSnapshotSize must account for:
									// Sig, Version, ParmsSize, RingSize, RingCount, 
									// VideoIdx, VideoFrame, StateSize, GlobRingSize
	};
	static const double MIN_ASPECT_RATIO;
	static const double MIN_STAR_RATIO;
	static const PARMS	m_DefParms;	// default parameters
	static const STATE	m_DefState;	// default state
	static const GLOBRING	m_DefGlobRing;	// default global ring

// Member data
	static	CWhorldView	*m_MainView;	// static pointer to our first instance
	PARMS	m_Parms;				// current parameters
	PARMS	m_InParms;				// input parameters
	COscillator	m_Osc[ROWS];		// oscillators
	CList<RING, RING&>	m_Ring;		// array of rings
	CBackBufGDI	m_gdi;				// GDI back buffer
	POINT	m_pa[MAX_POINTS * 2];	// enough for two rings
	CParmInfo	m_PrevInfo;			// previous parameter info
	STATE	m_st;			// current state
	CPen	m_Pen;			// current pen, used for line widths > 1
	CSize	m_Size;			// size of our window in client coords
	CPoint	m_Origin;		// origin in client coords; computed from NormOrg
	CRect	m_Canvas;		// ring dies when ALL its vertices are off canvas
	double	m_NewGrowth;	// new ring growth, computed at start of TimerHook
	int		m_Options;		// see options enum
	int		m_MaxRings;		// maximum number of rings
	POSITION	m_DelPos;	// if non-null, position of ring after last deletion
	bool	m_FlushHistory;	// if true, next TimerHook won't interpolate
	bool	m_MiniRings;	// if true, Serialize reads/writes mini-rings
	bool	m_Paused;		// if true, TimerHook isn't being called
	int		m_ZoomType;		// see zoom type enum
	CVideoList	m_VideoList;	// array of video clips
	bool	m_Copying;		// if true, add a rotating skew to new ring origins
	int		m_CopyCount;	// number of copies; must be > 0
	int		m_CopySpread;	// copy rotation radius, in pixels
	double	m_CopyTheta;	// copy rotation angle, in radians
	double	m_CopyDelta;	// copy rotation angle increment, in radians per frame
	GLOBRING	m_GlobRing;	// global ring data

// Helpers
	void	MirrorToNorm(const DPOINT& Mirror, DPOINT& Norm) const;
	void	NormToMirror(const DPOINT& Norm, DPOINT& Mirror) const;
	void	ResizeCanvas();
	void	UpdateClientOrg();
	CSize	GetPaneSize() const;
	void	AddRing();
	void	SetLineWidth(const CDWordArray& LineWidth);
	void	GetLineWidth(CDWordArray& LineWidth) const;
	void	ScaleLineWidth(double Scaling, CDWordArray& PrevLineWidth, int& PrevGlobLineWidth);
	void	ScaleOrigin(double Scaling, DPOINT_ARRAY& PrevOrigin);
	void	SetOrigin(const DPOINT_ARRAY& Origin);
	static	double	Wrap(double Val, double Limit);
	static	double	Reflect(double Val, double Limit);
	void	UpdateHue(double DeltaTick);
	void	ThrowBadFormat(CArchive &ar);
};

inline CWhorldView *CWhorldView::GetMainView()
{
	return(m_MainView);
}

inline CSize CWhorldView::GetSize() const
{
	return(m_Size);
}

inline int CWhorldView::GetRingCount() const
{
	return(m_Ring.GetCount());
}

inline void CWhorldView::GetParms(PARMS& Parms) const
{
	Parms = m_Parms;
}

inline void CWhorldView::SetParms(const PARMS& Parms)
{
	m_InParms = Parms;
}

inline void CWhorldView::GetDefaults(PARMS& Parms)
{
	Parms = m_DefParms;
}

inline int CWhorldView::GetDrawMode() const
{
	return(m_st.DrawMode);
}

inline void CWhorldView::SetDrawMode(int Mode)
{
	m_st.DrawMode = Mode;
}

inline void CWhorldView::GetNormOrigin(DPOINT& pt) const
{
	pt = m_st.NormOrg;
}

inline CPoint CWhorldView::GetOrigin() const
{
	return(m_Origin);
}

inline double CWhorldView::GetHue() const
{
	return(m_st.Hue);
}

inline double CWhorldView::GetZoom() const
{
	return(m_st.Zoom);
}

inline bool CWhorldView::IsMirrored() const
{
	return(m_st.Mirror);
}

inline bool CWhorldView::IsColorInverted() const
{
	return(m_st.InvertMask != 0);
}

inline void CWhorldView::GetCanvasRect(CRect& Rect)
{
	Rect = m_Canvas;
}

inline double CWhorldView::GetCanvasScale() const
{
	return(m_st.CanvasScale);
}

inline bool CWhorldView::IsHueLooped() const
{
	return(m_st.HueLoop);
}

inline double CWhorldView::GetHueLoopLength() const
{
	return(m_st.HueLoopLength);
}

inline void CWhorldView::SetPhase(int Idx, double Phase)
{
	ASSERT(Idx >= 0 && Idx < ROWS);
	m_Osc[Idx].SetPhase(Phase);
}

inline void CWhorldView::FlushHistory()
{
	m_FlushHistory = TRUE;
}

inline void CWhorldView::MiniSerialize(CArchive &ar)
{
	m_MiniRings = TRUE;
	Serialize(ar);
	m_MiniRings = FALSE;
}

inline int CWhorldView::GetMiniSnapshotSize() const
{
	return(sizeof(PARMS) + sizeof(STATE) + sizeof(GLOBRING) 
		+ MINI_SNAP_INTS * sizeof(int)
		+ MINI_RING_SIZE * GetRingCount());
}

inline int CWhorldView::GetMaxRings() const
{
	return(m_MaxRings);
}

inline void CWhorldView::SetMaxRings(int Rings)
{
	m_MaxRings = Rings;
}

inline bool CWhorldView::GetConvex() const
{
	return(m_st.Convex);
}

inline void CWhorldView::SetTrail(double Trail)
{
	m_st.Trail = Trail;
}

inline double CWhorldView::GetTrail() const
{
	return(m_st.Trail);
}

inline void CWhorldView::SetZoomType(int Type)
{
	m_ZoomType = Type;
}

inline int CWhorldView::GetZoomType() const
{
	return(m_ZoomType);
}

inline bool CWhorldView::GetPause() const
{
	return(m_Paused);
}

inline void CWhorldView::SetPause(bool Enable)
{
	m_Paused = Enable;
}

inline CVideoList& CWhorldView::GetVideo()
{
	return(m_VideoList);
}

inline int CWhorldView::GetVideoROP() const
{
	return(m_VideoList.GetROPIdx());
}

inline int CWhorldView::GetCopyCount() const
{
	return(m_CopyCount);
}

inline int CWhorldView::GetCopySpread() const
{
	return(m_CopySpread);
}

inline void CWhorldView::GetVideoOrigin(DPOINT& Origin) const
{
	Origin = m_st.VideoOrg;
}

inline void	CWhorldView::SetVideoOrigin(const DPOINT& Origin)
{
	m_st.VideoOrg = Origin;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WHORLDVIEW_H__4E7A57CC_64AF_47DE_98D1_C3DF8A34C524__INCLUDED_)
