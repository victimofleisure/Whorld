// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      10feb25	initial version

*/

#pragma once

#include "WhorldBase.h"
#include "RowDlg.h"
#include "EditSliderCtrl.h"
#include "NumSpin.h"

class CWhorldDoc;
class CMasterView;

class CMasterRowDlg : public CRowDlg, public CWhorldBase
{
// Construction
public:
	CMasterRowDlg();

// Types
	struct DBL_RANGE {
		double	fMinVal;
		double	fMaxVal;
	};

// Attributes
	static void	GetPropEditRange(int iProp, DBL_RANGE& range);

// Operations
	void	Update(double fVal);
	static double	Norm(int iProp, double fVal);
	static double	Denorm(int iProp, double fVal);
	static double	SliderNorm(int iProp, double fVal);
	static double	SliderDenorm(int iProp, double fVal);

// Implementation
public:
	virtual ~CMasterRowDlg();

// Constants
	static const CEditSliderCtrl::INFO m_arrSliderInfo[MASTER_COUNT];

protected:
// Types
	class CMyEditSliderCtrl : public CEditSliderCtrl {
	public:
		virtual	double	Norm(double fVal) const;
		virtual	double	Denorm(double fVal) const;
	};
	class CMyNumEdit : public CNumEdit {
	public:
		virtual	void	StrToVal(LPCTSTR Str);
		virtual	void	ValToStr(CString& Str);
	};

// Constants
	static const DBL_RANGE m_arrEditRange[MASTER_COUNT];

// Data members
	CStatic		m_staticName;
	CMyEditSliderCtrl	m_sliderVal;
	CMyNumEdit	m_editVal;

// Overrides
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

// Helpers
	CMasterView* GetView();
	CWhorldDoc* GetDocument();
	static double	Log(double Base, double x);
	static double	LogNorm(double x, double Base, double Scale);
	static double	ExpNorm(double x, double Base, double Scale);
	static int	GetRowIdx(const CWnd& wnd);

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg void OnChangedVal(NMHDR* pNMHDR, LRESULT* pResult);
};
