// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        visualcpp.net
 
		revision history:
		rev		date	comments
        00      07may03	replaced leading underscores with m_ convention
		01		02oct04	add optional origin shift to initial parent rect
		02		30jul05	make style compatible
 
		resize a windows control

*/

#ifndef CCTRLRESIZE_INCLUDED
#define CCTRLRESIZE_INCLUDED

#include <afxtempl.h>

enum {
	BIND_TOP		= 0x01,
	BIND_LEFT		= 0x02,
	BIND_RIGHT		= 0x04,
	BIND_BOTTOM		= 0x08,
	BIND_ALL		= 0x0F,
	BIND_UNKNOWN	= 0x00
};

class CCtrlResize : public WObject {
public:
	typedef struct tagCTRL_LIST {
		int		CtrlID;
		int		BindType;
	} CTRL_LIST;
	CCtrlResize();
	void	OnSize();
	void	SetParentWnd(CWnd *pWnd);
	bool	FixControls();
	void	AddControl(int CtrlID, int BindType, const CRect *Rect = NULL);
	void	AddControlList(CWnd *pWnd, const CTRL_LIST *List);
	void	SetOriginShift(SIZE Shift);

private:
	class CtrlInfo {
	public:
		CtrlInfo();
		CtrlInfo(int CtrlID, int BindType, const CRect *RectInit, CWnd* pCtrlWnd = NULL);
		int		m_CtrlID;
		int		m_BindType;
		CRect	m_RectInit;
		CWnd	*m_pCtrlWnd;
	};
	CWnd	*m_pWnd;
	CArray	<CtrlInfo, CtrlInfo&> m_Info;
	CRect	m_RectInit;
	CSize	m_OrgShift;
};

#endif
