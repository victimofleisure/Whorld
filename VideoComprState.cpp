// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      16aug06	initial version
        01      05mar07	add Read and Write

        video compressor state

*/

#include "stdafx.h"
#include "VideoComprState.h"

IMPLEMENT_SERIAL(CVideoComprState, CObject, 1);

CVideoComprState::CVideoComprState()
{
	ZeroMemory(&m_Parms, sizeof(PARMS));
}

void CVideoComprState::Copy(const CVideoComprState& st)
{
	m_Name = st.m_Name;
	m_DlgState.Copy(st.m_DlgState);
	m_Parms = st.m_Parms;
}

void CVideoComprState::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << m_Name;
		ar.Write(&m_Parms, sizeof(PARMS));
	} else {
		ar >> m_Name;
		ar.Read(&m_Parms, sizeof(PARMS));
	}
	m_DlgState.Serialize(ar);
}

void CVideoComprState::Write(CFile& fp)
{
	CArchive	ar(&fp, CArchive::store);
	Serialize(ar);
}

void CVideoComprState::Read(CFile& fp)
{
	CArchive	ar(&fp, CArchive::load);
	Serialize(ar);
}

bool CVideoComprState::Write(LPCTSTR Path)
{
	CFileException	e;
	TRY {
		CFile	fp(Path, CFile::modeCreate | CFile::modeWrite);
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

bool CVideoComprState::Read(LPCTSTR Path)
{
	CFileException	e;
	TRY {
		CFile	fp(Path, CFile::modeRead | CFile::shareDenyWrite);
		Read(fp);
	}
	CATCH(CFileException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}

