// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version
		01		07mar25	add prompt for multiple files

*/

// stdafx.cpp : source file that includes just the standard includes
// Whorld.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "resource.h"

void DDV_Fail(CDataExchange* pDX, int nIDC)
{
	ASSERT(pDX != NULL);
	ASSERT(pDX->m_pDlgWnd != NULL);
	// test for edit control via GetClassName instead of IsKindOf,
	// so test works on controls that aren't wrapped in MFC object
	HWND	hWnd = ::GetDlgItem(pDX->m_pDlgWnd->m_hWnd, nIDC);
	ASSERT(hWnd != NULL);
	TCHAR	szClassName[6];
	// if control is an edit control
	if (GetClassName(hWnd, szClassName, 6) && !_tcsicmp(szClassName, _T("Edit")))
		pDX->PrepareEditCtrl(nIDC);
	else	// not an edit control
		pDX->PrepareCtrl(nIDC);
	pDX->Fail();
}

CString FormatSystemError(DWORD ErrorCode)
{
	LPTSTR	lpszTemp;
	DWORD	bRet = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, ErrorCode, 0, (LPTSTR)&lpszTemp, 0, NULL);	// default language
	CString	sError;	
	if (bRet) {	// if format succeeded
		sError = lpszTemp;	// create string from message buffer
		LocalFree(lpszTemp);	// free message buffer
	} else	// format failed
		sError.Format(IDS_APP_UNKNOWN_SYSTEM_ERROR, ErrorCode, ErrorCode);
	return(sError);
}

CString	GetLastErrorString()
{
	return(FormatSystemError(GetLastError()));
}

void SetTimelyProgressPos(CProgressCtrl& Progress, int nPos)
{
	// workaround for Aero animated progress bar's absurd lag
	int nLower, nUpper;
	Progress.GetRange(nLower, nUpper);
	if (nPos < nUpper) {	// if not at limit
		// decrement causes immediate update, bypassing animation
		Progress.SetPos(nPos + 1);	// set one beyond desired value
		Progress.SetPos(nPos);	// set desired value
	} else {	// limit reached
		// more elaborate kludge to bypass animation for last position
		Progress.SetRange32(nLower, nUpper + 1);
		Progress.SetPos(nUpper + 1);
		Progress.SetPos(nUpper);
		Progress.SetRange32(nLower, nUpper);
	}
}

bool GetUserNameString(CString& sName)
{
	sName.Empty();
	DWORD	dwLen = UNLEN + 1;
	LPTSTR	lpszName = sName.GetBuffer(dwLen);
	bool	bRetVal = GetUserName(lpszName, &dwLen) != 0;
	sName.ReleaseBuffer();
	return bRetVal;
}

bool GetComputerNameString(CString& sName)
{
	sName.Empty();
	DWORD	dwLen = MAX_COMPUTERNAME_LENGTH + 1;
	LPTSTR	lpszName = sName.GetBuffer(dwLen);
	bool	bRetVal = GetComputerName(lpszName, &dwLen) != 0;
	sName.ReleaseBuffer();
	return bRetVal;
}

bool CopyStringToClipboard(HWND m_hWnd, const CString& strData)
{
	if (!OpenClipboard(m_hWnd))
		return false;
	EmptyClipboard();
	size_t	nLen = (strData.GetLength() + 1) * sizeof(TCHAR);
	HGLOBAL	hBuf = GlobalAlloc(GMEM_DDESHARE, nLen);
	if (hBuf == NULL)
		return false;
	TCHAR	*pBuf = (TCHAR *)GlobalLock(hBuf);
	USES_CONVERSION;
	memcpy(pBuf, strData, nLen);
	GlobalUnlock(hBuf);
#ifdef UNICODE
	UINT	nFormat = CF_UNICODETEXT;
#else
	UINT	nFormat = CF_TEXT;
#endif
	HANDLE	hData = SetClipboardData(nFormat, hBuf);
	CloseClipboard();
	return hData != NULL;
}

void EnableChildWindows(CWnd& Wnd, bool Enable, bool Deep)
{
	CWnd	*wp = Wnd.GetWindow(GW_CHILD);
	while (wp != NULL) {
		if (Deep)	// if recursively searching all children
			EnableChildWindows(*wp, Enable, Deep);
		wp->EnableWindow(Enable);
		wp = wp->GetNextWindow();
	}
}

class CUpdateMenuCmdUI : public CCmdUI {
public:
	virtual void SetRadio(BOOL bOn);
};

void CUpdateMenuCmdUI::SetRadio(BOOL bOn)
{
	// framework's implementation draws a tiny dot that looks ridiculous
	if (m_pMenu != NULL) {
		if (m_pSubMenu != NULL)
			return; // don't change popup menus indirectly
		if (bOn)
			m_pMenu->CheckMenuRadioItem(m_nID, m_nID, m_nID, MF_BYCOMMAND);
	}
}

void UpdateMenu(CWnd *pWnd, CMenu *pMenu)
{
	CUpdateMenuCmdUI	cui;
	cui.m_pMenu = pMenu;
	cui.m_nIndexMax = pMenu->GetMenuItemCount();
	for (UINT i = 0; i < cui.m_nIndexMax; i++) {
		cui.m_nID = pMenu->GetMenuItemID(i);
		if (!cui.m_nID)	// separator
			continue;
		if (cui.m_nID == -1) {	// popup submenu
			CMenu	*pSubMenu = pMenu->GetSubMenu(i);
			if (pSubMenu != NULL)
				UpdateMenu(pWnd, pSubMenu);	// recursive call
		}
		cui.m_nIndex = i;
		cui.m_pMenu = pMenu;
		cui.DoUpdate(pWnd, FALSE);
	}
}

void DoGenericContextMenu(UINT nIDResource, CPoint point, CWnd* pWnd)
{
	CMenu	menu;
	VERIFY(menu.LoadMenu(nIDResource));
	UpdateMenu(pWnd, &menu);
	menu.GetSubMenu(0)->TrackPopupMenu(0, point.x, point.y, pWnd);
}

bool FormatNumberCommas(LPCTSTR pszSrc, CString& sDst, int nPrecision)
{
	WCHAR	szDecimal[4];
	WCHAR	szThousand[4];
	if (!GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SDECIMAL, szDecimal, _countof(szDecimal)))
		return false;
	if (!GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_STHOUSAND, szThousand, _countof(szThousand)))
		return false;
	if (0) {	// GetNumberFormatEx crashes or fails in OSX via WineBottler
		USES_CONVERSION;
		NUMBERFMTW	fmt = {static_cast<DWORD>(nPrecision), 0, 3, szDecimal, szThousand};
		int	nLen = GetNumberFormatEx(LOCALE_NAME_USER_DEFAULT, 0, T2CW(pszSrc), &fmt, NULL, 0);
		CStringW	sBuf;
		LPWSTR	pBuf = sBuf.GetBuffer(nLen);
		if (!GetNumberFormatEx(LOCALE_NAME_USER_DEFAULT, 0, T2CW(pszSrc), &fmt, pBuf, nLen))
			return false;
		sBuf.ReleaseBuffer();
		sDst = sBuf;
	} else {	// emulate the basics of GetNumberFormatEx
		USES_CONVERSION;
		sDst = pszSrc;
		int	nLen = sDst.GetLength();
		if (nPrecision > 0 && nPrecision < nLen) {	// if precision specified and applicable
			nLen -= nPrecision;
			sDst.Insert(nLen, W2CT(szDecimal));	// insert decimal separator
		}
		for (int iChar = nLen - 3; iChar > 0; iChar -= 3) {	// for each thousands group
			sDst.Insert(iChar, W2CT(szThousand));	// insert thousands separator
		}
	}
	return true;
}

int StringReplaceNoCase(CString& str, LPCTSTR pszOld, LPCTSTR pszNew)
{
	CString	sDest(str);
	CString	sOld(pszOld);
	sDest.MakeLower();	// convert destination and old string to lower case
	sOld.MakeLower();
	// first pass: find instances of old string and store their positions
	int	nOldLen = sOld.GetLength();
	CDWordArray	arrSub;
	int	iPos = 0;
	while ((iPos = sDest.Find(sOld, iPos)) >= 0) {	// while old string is found
		arrSub.Add(iPos);	// add found string's position to substitution array
		iPos += nOldLen;	// advance position past found string
	}
	// second pass: do the substitions, in reverse order for stability
	int	nSubs = static_cast<int>(arrSub.GetSize());	// string indices are 32-bit
	for (int iSub = nSubs - 1; iSub >= 0; iSub--) {	// for each substitution
		str.Delete(arrSub[iSub], nOldLen);	// delete old string
		str.Insert(arrSub[iSub], pszNew);	// insert new string
	}
	return nSubs;	// return number of substitutions
}

bool ShowListColumnHeaderMenu(CWnd *pWnd, CListCtrl& list, CPoint point)
{
	if (point.x == -1 && point.y == -1)	// if menu triggered via keyboard
		return false;
	ASSERT(pWnd != NULL);
	CPoint	ptGrid(point);
	CHeaderCtrl	*pHdrCtrl = list.GetHeaderCtrl();
	pHdrCtrl->ScreenToClient(&ptGrid);
	HDHITTESTINFO	hti = {ptGrid};
	pHdrCtrl->HitTest(&hti);
	if (hti.flags & (HHT_ONHEADER | HHT_NOWHERE | HHT_ONDIVIDER)) {
		CMenu	menu;
		VERIFY(menu.LoadMenu(IDR_LIST_COL_HDR));
		return menu.GetSubMenu(0)->TrackPopupMenu(0, point.x, point.y, pWnd) != 0;
	}
	return false;
}

#define CHECK_COM(x) { HRESULT hr = x; if (FAILED(hr)) return hr; }

HRESULT PromptForFiles(CStringArrayEx& saPath, int nFilters, const COMDLG_FILTERSPEC* pFilter, int iSelFilter)
{
	CStringArrayEx	saTempPath;	// store file paths in temporary string array
	CComPtr<IFileOpenDialog> pFileOpen;
	CHECK_COM(pFileOpen.CoCreateInstance(__uuidof(FileOpenDialog)));	// create instance
	DWORD dwFlags;
	CHECK_COM(pFileOpen->GetOptions(&dwFlags));	// get existing flags
	CHECK_COM(pFileOpen->SetOptions(dwFlags | FOS_ALLOWMULTISELECT));	// set additional flags
	if (nFilters) {	// if a file type filter was specified
		CHECK_COM(pFileOpen->SetFileTypes(nFilters, pFilter));	// set file type filter
		CHECK_COM(pFileOpen->SetFileTypeIndex(iSelFilter));	// select specified file type filter
	}
	CHECK_COM(pFileOpen->Show(NULL));	// show file open dialog
	CComPtr<IShellItemArray> pItemArray;
	CHECK_COM(pFileOpen->GetResults(&pItemArray));	// get results
	DWORD	nItems;
	CHECK_COM(pItemArray->GetCount(&nItems));	// get item count
	saTempPath.SetSize(nItems);
	for (DWORD iItem = 0; iItem < nItems; iItem++) {	// for each item
		CComPtr<IShellItem> pItem;
		CHECK_COM(pItemArray->GetItemAt(iItem, &pItem));	// get item
		CComHeapPtr<WCHAR> spPath;
		CHECK_COM(pItem->GetDisplayName(SIGDN_FILESYSPATH, &spPath));	// get item path
		saTempPath[iItem] = spPath;	// store path in caller's array
	}
	saPath.Swap(saTempPath);	// swap array pointers, transferring paths to caller
	return S_OK;	// success
}
