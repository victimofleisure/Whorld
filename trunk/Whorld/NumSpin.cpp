// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28sep04	initial version

        spin buddy for numeric edit control
 
*/

// NumSpin.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "NumSpin.h"
#include "NumEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNumSpin

IMPLEMENT_DYNAMIC(CNumSpin, CSpinButtonCtrl);

CNumSpin::CNumSpin()
{
	m_Delta = 1;
}

CNumSpin::~CNumSpin()
{
}

BEGIN_MESSAGE_MAP(CNumSpin, CSpinButtonCtrl)
	//{{AFX_MSG_MAP(CNumSpin)
	ON_NOTIFY_REFLECT(UDN_DELTAPOS, OnDeltapos)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNumSpin message handlers

void CNumSpin::OnDeltapos(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	CNumEdit	*edit = DYNAMIC_DOWNCAST(CNumEdit, GetWindow(GW_HWNDPREV));
	if (edit != NULL)
		edit->AddSpin(-pNMUpDown->iDelta * m_Delta);
	*pResult = 0;
}
