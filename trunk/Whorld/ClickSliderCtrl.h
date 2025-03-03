// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28sep04	initial version
		01		22apr05	remove undo handling
        02      22feb25	replace header guard with pragma
		03		03mar25	modernize style

		slider with jump to position and default
 
*/

#pragma once

// ClickSliderCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CClickSliderCtrl window

class CClickSliderCtrl : public CSliderCtrl
{
	DECLARE_DYNAMIC(CClickSliderCtrl);
// Construction
public:
	CClickSliderCtrl();

// Attributes
public:
	int		GetDefaultPos() const;
	void	SetDefaultPos(int nPos);
	void	SetTicCount(int nCount);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClickSliderCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CClickSliderCtrl();

protected:
// Generated message map functions
	//{{AFX_MSG(CClickSliderCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		MKU_DEFAULT_POS = MK_SHIFT
	};

// Member data
	int		m_nDefPos;	// default position, in slider coordinates

// Helpers
	int		PointToPos(CPoint point);
	void	PostPos();
	void	PostNotification(int nCode);
};

inline int CClickSliderCtrl::GetDefaultPos() const
{
	return m_nDefPos;
}

inline void CClickSliderCtrl::SetDefaultPos(int nPos)
{
	m_nDefPos = nPos;
}
