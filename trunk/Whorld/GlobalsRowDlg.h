// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02mar25	initial version
		01		05mar25	use center slider to fix unreachable center value

*/

#pragma once

#include "WhorldBase.h"
#include "RowDlg.h"
#include "CenterSliderCtrl.h"
#include "NumEdit.h"
#include "NumSpin.h"

class CWhorldDoc;

class CGlobalsRowDlg : public CRowDlg, public CWhorldBase
{
// Construction
public:
	CGlobalsRowDlg();

// Operations
	void	Update(double fVal);

// Implementation
public:
	virtual ~CGlobalsRowDlg();

protected:
// Types
	class CMyEditSliderCtrl : public CCenterSliderCtrl, public CWhorldBase {
	public:
		virtual	double	Norm(double x) const;
		virtual	double	Denorm(double x) const;
	};
	class CMyComboBox : public CComboBox {
	public:
		// define accessor aliases needed to generate code via preprocessor
		int		GetVal() const { GetCurSel(); }
		void	SetVal(int nSelect) { SetCurSel(nSelect); }
	};

// Data members
	CStatic		m_staticName;
	CMyEditSliderCtrl	m_sliderVal;
	CNumEdit	m_editVal;
	CNumSpin	m_spinVal;

// Overrides
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

// Helpers
	CGlobalsView* GetView();
	void	SetParam(double fVal);

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg void OnChangedVal(NMHDR* pNMHDR, LRESULT* pResult);
};
