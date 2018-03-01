// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		30aug05	initial version
		01		01sep05	in Close, flush CFile before writing frame index
		02		10sep05	add save frame
		03		27sep05	add resize
		04		17oct05	in Open, store must zero frame count
		05		25oct05	in SaveFrame, recreate HLS color from RGB
		06		07jun06	add video path history
		07		28jan08	support Unicode

		movie container for snapshots

*/

#include "stdafx.h"
#include "SnapMovie.h"

CSnapMovie::CSnapMovie()
{
	m_Archive = NULL;
	m_View = NULL;
	ZeroMemory(&m_Hdr, sizeof(HEADER));
	m_CurPos = 0;
}

CSnapMovie::~CSnapMovie()
{
	Close();
}

bool CSnapMovie::Open(LPCTSTR Path, bool Load, int Mode)
{
	Close();
	CFileException	e;
	if (m_File.Open(Path, Mode, &e)) {
		TRY {
			FILEID	FileId;
			if (Load) {
				m_File.Read(&FileId, sizeof(FileId));
				if (FileId.Sig != MOVIE_SIG || FileId.Version > MOVIE_VERSION)
					AfxThrowArchiveException(CArchiveException::badIndex, Path);
				ZeroMemory(&m_Hdr, sizeof(HEADER));
				m_File.Read(&m_Hdr, FileId.HdrSize);
				if (!SetPos64(m_Hdr.FrameIndexPos))
					AfxThrowArchiveException(CArchiveException::badIndex, Path);
				m_FrameIndex.SetSize(GetFrameCount());
				m_File.Read(m_FrameIndex.GetData(), GetFrameCount() * sizeof(__int64));
				if (FileId.Version > 1) {
					CArchive	ar(&m_File, CArchive::load);
					if (!m_View->GetVideo().SerializeHistory(ar))
						return(FALSE);
				}	// archive dtor flushes
				m_File.Seek(FIRST_SNAP_OFS, CFile::begin);
			} else {
				FileId.Sig = MOVIE_SIG;
				FileId.Version = MOVIE_VERSION;
				FileId.HdrSize = sizeof(HEADER);
				m_File.Write(&FileId, sizeof(FILEID));
				m_Hdr.FrameCount = 0;
				m_File.Write(&m_Hdr, sizeof(HEADER));
				m_SnapSize.SetSize(INITIAL_INDEX_ENTRIES);
			}
			m_CurPos = 0;
		}
		CATCH(CFileException, e)
		{
			e->ReportError();
			return(FALSE);
		}
		CATCH(CArchiveException, e)
		{
			e->ReportError();
			return(FALSE);
		}
		END_CATCH
		m_Archive = new CArchive(&m_File, Load ? CArchive::load : CArchive::store);
	} else {
		e.ReportError();
		return(FALSE);
	}
	return(TRUE);
}

bool CSnapMovie::Open(LPCTSTR Path, bool Load)
{
	int	Mode = Load ? (CFile::modeRead | CFile::shareDenyWrite) 
		: (CFile::modeCreate | CFile::modeWrite);
	return(Open(Path, Load, Mode));
}

void CSnapMovie::Close()
{
	if (m_Archive != NULL) {
		if (m_Archive->IsStoring()) {
			m_Archive->Flush();
			m_File.Flush();	// flush device too, else file will be corrupt
			m_Hdr.FrameIndexPos = GetPos64();
			m_FrameIndex.SetSize(GetFrameCount());
			__int64	Pos = FIRST_SNAP_OFS;
			for (UINT i = 0; i < GetFrameCount(); i++) {
				m_FrameIndex[i] = Pos;
				Pos += m_SnapSize[i];
			}
			m_File.Write(m_FrameIndex.GetData(), GetFrameCount() * sizeof(__int64));
			{
				CArchive	ar(&m_File, CArchive::store);
				m_View->GetVideo().SerializeHistory(ar);
			}	// archive dtor flushes
			m_File.Flush();
			m_File.Seek(sizeof(FILEID), CFile::begin);
			m_File.Write(&m_Hdr, sizeof(HEADER));
		}
		delete m_Archive;
		m_Archive = NULL;
	}
	if (m_File != CFile::hFileNull)
		m_File.Close();
	m_SnapSize.SetSize(0);
	m_FrameIndex.SetSize(0);
}

void CSnapMovie::Rewind()
{
	if (!IsOpen())
		AfxThrowFileException(CFileException::invalidFile);
	m_Archive->Flush();
	m_File.Seek(FIRST_SNAP_OFS, CFile::begin);
	m_CurPos = 0;
}

void CSnapMovie::Seek(int Frame)
{
	if (!IsOpen())
		AfxThrowFileException(CFileException::invalidFile);
	if (m_Archive->IsStoring())	// seek only supports read
		AfxThrowFileException(CFileException::accessDenied);
	m_Archive->Flush();
	if (Frame < 0 || Frame >= m_FrameIndex.GetSize()
	|| !SetPos64(m_FrameIndex[Frame]))
		AfxThrowFileException(CFileException::badSeek);
	m_CurPos = Frame;
}

void CSnapMovie::SaveFrame(LPCTSTR Path)
{
	if (!IsOpen())
		AfxThrowFileException(CFileException::invalidFile);
	m_View->SetHLSFromRGB();	// movie snap's mini-rings omit HLS color
	m_View->Serialize(Path, FALSE);
}

bool CSnapMovie::Resize(LPCTSTR Path, SIZE& Size)
{
	if (Open(Path, TRUE, CFile::modeReadWrite)) {
		TRY {
			m_Hdr.FrameSize = Size;
			m_File.Seek(sizeof(FILEID), CFile::begin);
			m_File.Write(&m_Hdr, sizeof(HEADER));
		}
		CATCH(CFileException, e)
		{
			e->ReportError();
			return(FALSE);
		}
		END_CATCH
	} else
		return(FALSE);
	return(TRUE);
}
