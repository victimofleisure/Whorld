// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08feb25	initial version

*/

#pragma once

#include "WhorldBase.h"
#include "RowDlg.h"
#include "EditSliderCtrl.h"
#include "NumEdit.h"
#include "NumSpin.h"

class CWhorldDoc;

class CParamsRowDlg : public CRowDlg, public CWhorldBase
{
// Construction
public:
	CParamsRowDlg();

// Operations
	void	Update(const PARAM_ROW& row);
	void	Update(const PARAM_ROW& row, int iProp);

// Implementation
public:
	virtual ~CParamsRowDlg();

protected:
// Types
	class CMyEditSliderCtrl : public CEditSliderCtrl, public CWhorldBase {
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
	CMyComboBox	m_comboWave;
	CNumEdit	m_editAmp;
	CNumSpin	m_spinAmp;
	CNumEdit	m_editFreq;
	CNumSpin	m_spinFreq;
	CNumEdit	m_editPW;
	CNumSpin	m_spinPW;

// Overrides
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

// Helpers
	CParamsView* GetView();
	void	SetParam(int iProp, const CComVariant& prop);

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg void OnChangedVal(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChangeWave();
	afx_msg void OnChangedAmp(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangedFreq(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangedPW(NMHDR* pNMHDR, LRESULT* pResult);
};
