// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28sep04	initial version
		01		24apr18	standardize names

        spin buddy for numeric edit control
 
*/

// NumSpin.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "NumSpin.h"
#include "NumEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CNumSpin

IMPLEMENT_DYNAMIC(CNumSpin, CSpinButtonCtrl);

CNumSpin::CNumSpin()
{
	m_fDelta = 1;
}

CNumSpin::~CNumSpin()
{
}

BEGIN_MESSAGE_MAP(CNumSpin, CSpinButtonCtrl)
	ON_NOTIFY_REFLECT(UDN_DELTAPOS, OnDeltapos)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNumSpin message handlers

void CNumSpin::OnDeltapos(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	CNumEdit	*edit = DYNAMIC_DOWNCAST(CNumEdit, GetWindow(GW_HWNDPREV));
	if (edit != NULL)
		edit->AddSpin(-pNMUpDown->iDelta * m_fDelta);
	*pResult = 0;
}
