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

#include "stdafx.h"
#include "Resource.h"
#include "CenterSliderCtrl.h"
#include <math.h>

/////////////////////////////////////////////////////////////////////////////
// CCenterSliderCtrl

IMPLEMENT_DYNAMIC(CCenterSliderCtrl, CEditSliderCtrl);

CCenterSliderCtrl::~CCenterSliderCtrl()
{
}

BEGIN_MESSAGE_MAP(CCenterSliderCtrl, CEditSliderCtrl)
	ON_WM_HSCROLL_REFLECT()
END_MESSAGE_MAP()

void CCenterSliderCtrl::HScroll(UINT nSBCode, UINT nPos)
{
	UNREFERENCED_PARAMETER(nSBCode);
	UNREFERENCED_PARAMETER(nPos);
	const double fCenterEpsilon = 0.0075;	// determined empirically
	int	nMin, nMax;
	GetRange(nMin, nMax);
	// normalize slider position
	double	fNormPos = static_cast<double>(nPos - nMin) / (nMax - nMin);
	// if slider close enough to center position
	if (fabs(fNormPos - 0.5) < fCenterEpsilon) {
		SetValNorm(0.5);	// call it center
	} else {	// not near center
		CEditSliderCtrl::HScroll(nSBCode, nPos);	// defer to base
	}
}
