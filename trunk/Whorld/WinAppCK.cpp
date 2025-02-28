// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        01      06jan10	W64: in DeleteDirectory, use size_t
        02      13jan12	add GetTempPath and GetAppPath
		03		21nov12	add DockControlBarLeftOf
		04		30nov12	add UpdateMenu
        05      17apr13	add temporary files folder
		06		21may13	add GetSpecialFolderPath
		07		10jul13	add GetLastErrorString
		08		19nov13	in EnableChildWindows, add Deep argument
		09		07may15	SHGetSpecialFolderPath return value is BOOL, not HRESULT
		10		03may18	remove VC6 cruft and methods that moved to file scope
		11		28jan19	add GetLogicalDrives
		12		10feb19	add temp file path wrapper
		13		13jun20	add conditional wait cursor wrapper
		14		11nov21	add FindMenuItem and InsertNumericMenuItems
		15		16feb25	add restart app method
		16		19feb25	add goto URL
		17		28feb25	add methods to set size of recent file list

        enhanced application
 
*/

// WinAppCK.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Resource.h"
#include "WinAppCK.h"
#include "PathStr.h"
#include "VersionInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWinAppCK

BEGIN_MESSAGE_MAP(CWinAppCK, CWinApp)
	//{{AFX_MSG_MAP(CWinAppCK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

bool CWinAppCK::GetTempPath(CString& Path)
{
	LPTSTR	pBuf = Path.GetBuffer(MAX_PATH);
	DWORD	retc = ::GetTempPath(MAX_PATH, pBuf);
	Path.ReleaseBuffer();
	return(retc != 0);
}

bool CWinAppCK::GetTempFileName(CString& Path, LPCTSTR Prefix, UINT Unique)
{
	CString	TempPath;
	if (!GetTempPath(TempPath))
		return(FALSE);
	if (Prefix == NULL)
		Prefix = m_pszAppName;
	LPTSTR	pBuf = Path.GetBuffer(MAX_PATH);
	DWORD	retc = ::GetTempFileName(TempPath, Prefix, Unique, pBuf);
	Path.ReleaseBuffer();
	return(retc != 0);
}

void CWinAppCK::GetCurrentDirectory(CString& Path)
{
	LPTSTR	pBuf = Path.GetBuffer(MAX_PATH);
	::GetCurrentDirectory(MAX_PATH, pBuf);
	Path.ReleaseBuffer();
}

bool CWinAppCK::GetSpecialFolderPath(int FolderID, CString& Path)
{
	LPTSTR	p = Path.GetBuffer(MAX_PATH);
	bool	retc = SUCCEEDED(SHGetSpecialFolderPath(NULL, p, FolderID, 0));
	Path.ReleaseBuffer();
	return(retc);
}

bool CWinAppCK::GetAppDataFolder(CString& Folder) const
{
	CPathStr	path;
	if (!GetSpecialFolderPath(CSIDL_APPDATA, path))
		return(FALSE);
	path.Append(m_pszAppName);
	Folder = path;
	return(TRUE);
}

CString CWinAppCK::GetAppPath()
{
	CString	s = GetCommandLine();
	s.TrimLeft();	// trim leading whitespace just in case
	if (s[0] == '"')	// if first char is a quote
		s = s.Mid(1).SpanExcluding(_T("\""));	// span to next quote
	else
		s = s.SpanExcluding(_T(" \t"));	// span to next whitespace
	return(s);
}

CString CWinAppCK::GetAppFolder()
{
	CPathStr	path(GetAppPath());
	path.RemoveFileSpec();
	return(path);
}

bool CWinAppCK::GetJobLogFolder(CString& Folder) const
{
	UNREFERENCED_PARAMETER(Folder);
	return(false);
}

CString CWinAppCK::GetVersionString()
{
	VS_FIXEDFILEINFO	AppInfo;
	CString	sVersion;
	CVersionInfo::GetFileInfo(AppInfo, NULL);
	sVersion.Format(_T("%d.%d.%d.%d"), 
		HIWORD(AppInfo.dwFileVersionMS), LOWORD(AppInfo.dwFileVersionMS),
		HIWORD(AppInfo.dwFileVersionLS), LOWORD(AppInfo.dwFileVersionLS));
#ifdef _WIN64
	sVersion += _T(" x64");
#else
	sVersion += _T(" x86");
#endif
#ifdef _DEBUG
	sVersion += _T(" Debug");
#else
	sVersion += _T(" Release");
#endif
	return sVersion;
}

#if _MFC_VER >= 0x0700	// this method is a problem in VC6
bool CWinAppCK::CreateFolder(LPCTSTR Path)
{
	int	retc = SHCreateDirectoryEx(NULL, Path, NULL);	// create folder
	switch (retc) {
	case ERROR_SUCCESS:
	case ERROR_FILE_EXISTS:
	case ERROR_ALREADY_EXISTS:
		break;
	default:
		return false;
	}
	return true;
}
#endif

bool CWinAppCK::DeleteFolder(LPCTSTR Path, FILEOP_FLAGS nFlags)
{
	SHFILEOPSTRUCT fop = {NULL, FO_DELETE, Path, _T(""), nFlags};
	return !SHFileOperation(&fop);
}

CString CWinAppCK::GetTitleFromPath(LPCTSTR Path)
{
	CPathStr	s(PathFindFileName(Path));
	s.RemoveExtension();
	return(s);
}

bool CWinAppCK::GetLogicalDriveStringArray(CStringArrayEx& arrDrive)
{
	DWORD	dwBufferSize = GetLogicalDriveStrings(0, NULL);
	if (!dwBufferSize)
		return false;
	CString	sDrive;
	TCHAR	*pDrive = sDrive.GetBuffer(dwBufferSize + 1);
    if (pDrive == NULL)
		return false;
	GetLogicalDriveStrings(dwBufferSize, pDrive);
	while (*pDrive) {
		arrDrive.Add(pDrive);
		pDrive += _tcslen(pDrive) + 1;
	}
	return true;
}

int	CWinAppCK::FindMenuItem(const CMenu *pMenu, UINT nItemID)
{
	int	nItems = pMenu->GetMenuItemCount();
	for (int iItem = 0; iItem < nItems; iItem++) {
		if (pMenu->GetMenuItemID(iItem) == nItemID)
			return(iItem);	// return item's position
	}
	return(-1);
}

bool CWinAppCK::InsertNumericMenuItems(CMenu *pMenu, UINT nPlaceholderID, UINT nStartID, int nStartVal, int nItems, bool bInsertAfter)
{
	int	iStartItem = FindMenuItem(pMenu, nPlaceholderID);
	if (iStartItem < 0)
		return false;
	CString	sItem;
	for (int iItem = 0; iItem < nItems; iItem++) {	// for each item in range
		sItem.Format(_T("%d"), nStartVal + iItem);
		int	iAmpersand = iItem >= nItems - nStartVal;
		sItem.Insert(iAmpersand, '&');
		if (!pMenu->InsertMenu(iItem + bInsertAfter, MF_STRING | MF_BYPOSITION, nStartID + iItem, sItem))
			return false;
	}
	return true;
}

bool CWinAppCK::RestartApp()
{
	TCHAR	szAppPath[MAX_PATH] = {0};
	::GetModuleFileName(NULL, szAppPath, MAX_PATH);
	STARTUPINFO	si = {0};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_NORMAL;
	PROCESS_INFORMATION	pi;
	if (!CreateProcess(NULL, szAppPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		AfxMessageBox(GetLastErrorString()); // report the error
		return false;
	}
	return true;
}

bool CWinAppCK::GotoUrl(LPCTSTR Url)
{
	INT_PTR	retc = (INT_PTR)ShellExecute(NULL, NULL, Url, NULL, NULL, SW_SHOWNORMAL);
	return(retc > HINSTANCE_ERROR);
}

void CWinAppCK::SetRecentFileListSize(int nSize)
{
	if (m_pRecentFileList != NULL) {
		SetRecentFileListSize(*m_pRecentFileList, nSize);
	}
}

void CWinAppCK::SetRecentFileListSize(CRecentFileList& list, int nSize)
{
	ASSERT(nSize >= 0);
	if (nSize == list.m_nSize)	// if size is unchanged
		return;	// nothing to do
	CStringArray	arrName;
	int	nSavedNames = min(nSize, list.m_nSize);
	arrName.SetSize(nSavedNames);
	// save names that can be saved
	for (int iName = 0; iName < nSavedNames; iName++) {
		arrName[iName] = list.m_arrNames[iName];
	}
	delete [] list.m_arrNames;	// delete old list
	list.m_arrNames = new CString[nSize];	// create new list
	// restore saved names
	for (int iName = 0; iName < nSavedNames; iName++) {
		list.m_arrNames[iName] = arrName[iName];
	}
	list.m_nSize = nSize;	// update size
}

CTempFilePath::~CTempFilePath()
{
	Empty();
}

void CTempFilePath::Empty()
{
	if (!m_sPath.IsEmpty()) {
		DeleteFile(m_sPath);
		m_sPath.Empty();
	}
}

void CTempFilePath::SetPath(const CString& sPath)
{
	Empty();
	m_sPath = sPath;
}

void CWaitCursorEx::Show(bool bShow)
{
	if (bShow != m_bShow) {
		m_bShow = bShow;
		if (bShow)
			AfxGetApp()->BeginWaitCursor();
		else
			AfxGetApp()->EndWaitCursor();
	}
}
