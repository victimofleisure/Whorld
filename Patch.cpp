// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17apr06	initial version
		01		18apr06	add HueLoopLength and CanvasScale
		02		21jun06	add tag to MasterDef macro
		03		24jun06	add Copies and Spread
		04		28jan08	support Unicode

		patch container
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "Patch.h"
#include "FormatIO.h"
#include "WhorldView.h"

const LPCTSTR CPatch::FILE_ID = _T("WHORLD%d");	// placeholder for version number

const CPatch::LINE_INFO CPatch::m_LineInfo[] = {
	#undef MASTERDEF
	#define MASTERDEF(tag, name, type) {_T(#name), FIO_##type, offsetof(CPatch, m_Master.name)},
	#include "MasterDef.h"	// build I/O list for master members
	#undef MAINDEF
	#define MAINDEF(name, type) {_T(#name), FIO_##type, offsetof(CPatch, m_Main.name)},
	#include "MainDef.h"	// build I/O list for main members
	{NULL}	// terminator, don't delete
};

const CPatch::MASTER CPatch::m_MasterDefaults = {
	1,			// Speed
	1,			// Zoom
	.186261,	// Damping
	0,			// Trail
	MAX_RINGS,	// Rings
	100,		// Tempo
	30,			// HueLoopLength
	1.4,		// CanvasScale
	1,			// Copies
	100			// Spread
};

const CPatch::MAIN CPatch::m_MainDefaults = {
	{.5, .5},	// Origin
	0,			// DrawMode
	0,			// OrgMotion
	0,			// Hue
	FALSE,		// Mirror
	FALSE,		// Reverse
	FALSE,		// Convex
	FALSE,		// InvertColor
	FALSE,		// LoopHue
	FALSE,		// Pause
	FALSE		// ZoomCenter
};

void CPatch::SetDefaults()
{
	CParmInfo::SetDefaults();
	m_Master = m_MasterDefaults;
	m_Main = m_MainDefaults;
}

bool CPatch::Read(CStdioFile& fp)
{
	CString	s;
	int	Version;
	if (!fp.ReadString(s) || _stscanf(s, FILE_ID, &Version) != 1)
		return(FALSE);
	SetDefaults();	// in case some lines are missing
	while (fp.ReadString(s)) {
		s.TrimLeft();
		CString	Name = s.SpanExcluding(_T(" \t"));
		CString	Arg = s.Mid(Name.GetLength());
		int	i;
		for (i = 0; i < ROWS; i++) {
			if (Name == m_RowData[i].Name) {
				ROW	*r = &m_Row[i];
				_stscanf(Arg, _T("%lf %d %lf %lf %lf"),
					&r->Val, &r->Wave, &r->Amp, &r->Freq, &r->PW);
				break;
			}
		}
		if (i >= ROWS) {	// not a parm row, assume it's a state member
			for (i = 0; m_LineInfo[i].Name != NULL; i++) {
				if (Name == m_LineInfo[i].Name) {
					CFormatIO::StrToVal(m_LineInfo[i].Type, Arg,
						((char *)this) + m_LineInfo[i].Offset);
					break;
				}
			}
		}
	}
	return(TRUE);
}

bool CPatch::Write(CStdioFile& fp) const
{
	CString	s;
	s.Format(FILE_ID, FILE_VERSION);
	fp.WriteString(s + "\n");
	int	i;
	for (i = 0; i < ROWS; i++) {
		const ROW	*r = &m_Row[i];
		s.Format(_T("%s\t%g\t%d\t%g\t%g\t%g\n"), m_RowData[i].Name,
			r->Val, r->Wave, r->Amp, r->Freq, r->PW);
		fp.WriteString(s);
	}
	for (i = 0; m_LineInfo[i].Name != NULL; i++) {
		CFormatIO::ValToStr(m_LineInfo[i].Type, 
			((char *)this) + m_LineInfo[i].Offset, s);
		fp.WriteString(CString(m_LineInfo[i].Name) + "\t" + s + "\n");
	}
	return(TRUE);
}

bool CPatch::Read(LPCTSTR Path)
{
	CStdioFile	fp;
	CFileException	e;
	if (!fp.Open(Path, CFile::modeRead | CFile::shareDenyWrite, &e)) {
		e.ReportError();
		return(FALSE);
	}
	TRY {
		if (!Read(fp)) {
			AfxMessageBox(IDS_DOC_BAD_FORMAT);
			return(FALSE);
		}
	}
	CATCH(CFileException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}

bool CPatch::Write(LPCTSTR Path) const
{
	CStdioFile	fp;
	CFileException	e;
	if (!fp.Open(Path, CFile::modeCreate | CFile::modeWrite, &e)) {
		e.ReportError();
		return(FALSE);
	}
	TRY {
		Write(fp);
	}
	CATCH(CFileException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}
