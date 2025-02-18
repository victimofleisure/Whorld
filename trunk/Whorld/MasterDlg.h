// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23jun05	initial version
        01      04mar06	add trail
        02      17apr06	move INFO struct into CPatch
		03		18apr06	add HueLoopLength and CanvasScale
		04		21jun06	make dialog resizable
		05		24jun06	add Copies and Spread
		06		28jan08	support Unicode

        master dialog
 
*/

#if !defined(AFX_MASTERDLG_H__5D7BA160_510D_49A7_B7FB_AD99BD48D9DC__INCLUDED_)
#define AFX_MASTERDLG_H__5D7BA160_510D_49A7_B7FB_AD99BD48D9DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MasterDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMasterDlg dialog

#include "ToolDlg.h"
#include "EditSliderCtrl.h"
#include "NumSpin.h"
#include "Patch.h"
#include "CtrlResize.h"

class CMasterDlg : public CToolDlg
{
	DECLARE_DYNAMIC(CMasterDlg);
// Construction
public:
	CMasterDlg(CWnd* pParent = NULL);   // standard constructor

// Constants
	enum {	// sliders
		#undef MASTERDEF
		#define MASTERDEF(tag, name, type) tag,
		#include "MasterDef.h"
		SLIDERS
	};

// Types
	typedef CPatch::MASTER INFO;

// Attributes
	#undef MASTERDEF
	#define MASTERDEF(tag, name, type) \
		const CEditSliderCtrl&	Get##name() const { return(m_##name##Slider); } \
		CEditSliderCtrl&		Get##name() { return(m_##name##Slider); }
	#include "MasterDef.h"	// emit const and non-const accessor for each slider
	void	GetDefaults(INFO& Info) const;
	void	GetInfo(INFO& Info) const;
	void	SetInfo(const INFO& Info);

// Operations
	void	RestoreDefaults();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMasterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
// Types
	class CDampingSlider : public CEditSliderCtrl {
	protected:
		static const float	SCALE;
		static const float	BASE;
		double	Norm(double x) const;
		double	Denorm(double x) const;
	};
	class CDampingEdit : public CNumEdit {
	protected:
		static const float	SCALE;
		void	ValToStr(CString& Str);
		void	StrToVal(LPCTSTR Str);
	};
	class CRingsSlider : public CEditSliderCtrl {
	protected:
		static const float	SCALE;
		static const float	BASE;
		double	Norm(double x) const;
		double	Denorm(double x) const;
	};
	class CRingsEdit : public CNumEdit {
	protected:
		void	ValToStr(CString& Str);
	};
	class CTrailSlider : public CEditSliderCtrl {
	protected:
		static const float	SCALE;
		static const float	BASE;
		double	Norm(double x) const;
		double	Denorm(double x) const;
	};

// Dialog Data
	//{{AFX_DATA(CMasterDlg)
	enum { IDD = IDD_MASTER };
	CEditSliderCtrl	m_SpreadSlider;
	CNumEdit	m_SpreadEdit;
	CEditSliderCtrl	m_CopiesSlider;
	CNumEdit	m_CopiesEdit;
	CEditSliderCtrl	m_CanvasScaleSlider;
	CNumEdit	m_CanvasScaleEdit;
	CEditSliderCtrl	m_HueLoopLengthSlider;
	CNumEdit	m_HueLoopLengthEdit;
	CTrailSlider	m_TrailSlider;
	CNumEdit	m_TrailEdit;
	CRingsSlider	m_RingsSlider;
	CRingsEdit	m_RingsEdit;
	CNumEdit	m_TempoEdit;
	CDampingEdit	m_DampingEdit;
	CNumEdit	m_ZoomEdit;
	CNumEdit	m_SpeedEdit;
	CEditSliderCtrl	m_TempoSlider;
	CDampingSlider	m_DampingSlider;
	CEditSliderCtrl	m_ZoomSlider;
	CEditSliderCtrl	m_SpeedSlider;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMasterDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	static	const CEditSliderCtrl::INFO m_SliderInfo[SLIDERS];

// Member data
	double	m_Damping;
	CEditSliderCtrl	*m_Slider[SLIDERS];
	CCtrlResize	m_Resize;
	CRect	m_InitRect;

// Helpers
	static	double	Log(double Base, double x);
	static	double	LogNorm(double x, double Base, double Scale);
	static	double	ExpNorm(double x, double Base, double Scale);

// Friends
	friend	CDampingSlider;
	friend	CDampingEdit;
	friend	CRingsSlider;
	friend	CTrailSlider;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MASTERDLG_H__5D7BA160_510D_49A7_B7FB_AD99BD48D9DC__INCLUDED_)
