// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      05mar25	initial version

*/

#pragma once

// EditSliderCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCenterSliderCtrl window

#include "EditSliderCtrl.h"

class CCenterSliderCtrl : public CEditSliderCtrl
{
	DECLARE_DYNAMIC(CCenterSliderCtrl);
public:
	virtual ~CCenterSliderCtrl();

// Generated message map functions
	afx_msg void HScroll(UINT nSBCode, UINT nPos);
	DECLARE_MESSAGE_MAP()

protected:
};
