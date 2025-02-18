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

		slider with jump to position and default
 
*/

#if !defined(AFX_CLICKSLIDERCTRL_H__810797E4_C040_4E3D_98F7_C75B7C55507D__INCLUDED_)
#define AFX_CLICKSLIDERCTRL_H__810797E4_C040_4E3D_98F7_C75B7C55507D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
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
	void	SetDefaultPos(int Pos);
	void	SetTicCount(int Count);

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
	int		m_DefPos;	// default position, in slider coordinates

// Helpers
	int		PointToPos(CPoint point);
	void	PostPos();
	void	PostNotification(int Code);
};

inline int CClickSliderCtrl::GetDefaultPos() const
{
	return(m_DefPos);
}

inline void CClickSliderCtrl::SetDefaultPos(int Pos)
{
	m_DefPos = Pos;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLICKSLIDERCTRL_H__810797E4_C040_4E3D_98F7_C75B7C55507D__INCLUDED_)
