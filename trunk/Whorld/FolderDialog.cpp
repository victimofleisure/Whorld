// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28jan02 initial version
		01		09jan03	fixed cancel case
		02		23nov07	support Unicode

        Browses for a folder.
 
*/

#include "stdafx.h"
#include "FolderDialog.h"

bool CFolderDialog::GetItemIdListFromPath(LPWSTR lpszPath, LPITEMIDLIST *lpItemIdList)
{
	LPSHELLFOLDER pShellFolder = NULL;
	HRESULT	hr;
	ULONG	chUsed;
	// Get desktop IShellFolder interface
	if (SHGetDesktopFolder(&pShellFolder) != NOERROR)
		return(FALSE);     // failed
	// convert the path to an ITEMIDLIST
	hr = pShellFolder->ParseDisplayName(NULL, NULL, lpszPath, &chUsed, lpItemIdList, NULL);
	if (FAILED(hr)) {
		pShellFolder->Release();
		*lpItemIdList = NULL;
		return(FALSE);
	}
	pShellFolder->Release();
	return(TRUE);
}

bool CFolderDialog::GetItemIdListFromPath(LPCTSTR lpszPath, LPITEMIDLIST *lpItemIdList)
{
#ifdef UNICODE
	return(GetItemIdListFromPath(lpszPath, lpItemIdList));
#else
	CWordArray	w;
	w.SetSize((strlen(lpszPath) + 1) * 2);
	MultiByteToWideChar(CP_ACP, 0, lpszPath, -1, w.GetData(), w.GetSize());
	return(GetItemIdListFromPath(w.GetData(), lpItemIdList));
#endif
}

bool CFolderDialog::BrowseFolder(LPCTSTR Title, CString& Folder, LPCTSTR Root, UINT Flags)
{
	LPMALLOC lpMalloc;
	if (::SHGetMalloc(&lpMalloc) != NOERROR)
		return(FALSE);
	bool retc = FALSE;
	TCHAR	szDisplayName[_MAX_PATH];
	TCHAR	szBuffer[_MAX_PATH];
	LPITEMIDLIST pidlRoot = 0;
	if (Root)
		GetItemIdListFromPath(Root, &pidlRoot);
    BROWSEINFO	bi;
	bi.hwndOwner = AfxGetMainWnd()->m_hWnd;
    bi.pidlRoot = pidlRoot;
    bi.pszDisplayName = szDisplayName;
    bi.lpszTitle = Title;
    bi.ulFlags = Flags;
    bi.lpfn = 0;
    bi.lParam = 0;
    bi.iImage = 0;
	LPITEMIDLIST lpItemIDList = SHBrowseForFolder(&bi);
	if (lpItemIDList) {
		if (SHGetPathFromIDList(lpItemIDList, szBuffer)) {
			Folder = szBuffer;
			retc = TRUE;
		}
	}
	lpMalloc->Free(lpItemIDList);
	lpMalloc->Release();  
	if (pidlRoot)
		lpMalloc->Free(pidlRoot);
    return(retc);
}
