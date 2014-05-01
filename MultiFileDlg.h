// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01jul05	initial version
		01		27oct06	add GetFolder
		02		23nov07	support Unicode

        file dialog with enhanced multiple selection
 
*/

#if !defined(AFX_MULTIFILEDLG_H__EEE10EF0_3E57_4C15_BD7E_FA68591A7C88__INCLUDED_)
#define AFX_MULTIFILEDLG_H__EEE10EF0_3E57_4C15_BD7E_FA68591A7C88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultiFileDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMultiFileDlg dialog

class CMultiFileDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CMultiFileDlg)
// Construction
public:
	CMultiFileDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD	dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		LPCTSTR	lpszTitle = NULL,	// non-standard but useful
		CWnd*	pParentWnd = NULL);

// Constants
	enum {	// extended styles
		ES_SHOWPREVIEW	= 0x01,
	};

// Attributes
	int		GetPathCount() const;
	LPCTSTR	GetFilePath(int Idx) const;
	void	GetPathArray(CStringArray& PathArray);
	const	CStringArray *GetPathArray() const;
	void	SetExtendedStyle(int ExStyle);
	void	SetNotifyWnd(CWnd *Wnd);
	void	SetPreview(bool Enable);
	bool	GetPreview() const;
	void	GetFolder(CString& Folder);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiFileDlg)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnFileNameChange();
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog data
	//{{AFX_DATA(CMultiFileDlg)
	enum { IDD = IDD_MULTI_FILE };
	BOOL	m_Preview;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMultiFileDlg)
	afx_msg void OnMultiFilePreview();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	CStringArray	m_Path;	// array of selected file paths
	DWORD	m_ExStyle;		// extended style; see enum above
	CWnd	*m_NotifyWnd;	// notifications are sent to this window
	bool	m_OwnTemplate;	// true if we're using our own dialog template
	CString	m_Title;		// title for dialog caption bar
};

inline int CMultiFileDlg::GetPathCount() const
{
	return(m_Path.GetSize());
}

inline LPCTSTR CMultiFileDlg::GetFilePath(int Idx) const
{
	return(m_Path[Idx]);
}

inline void CMultiFileDlg::SetExtendedStyle(int ExStyle)
{
	m_ExStyle = ExStyle;
}

inline void CMultiFileDlg::SetNotifyWnd(CWnd *Wnd)
{
	m_NotifyWnd = Wnd;
}

inline bool CMultiFileDlg::GetPreview() const
{
	return(m_Preview != 0);
}

inline void CMultiFileDlg::GetPathArray(CStringArray& PathArray)
{
	PathArray.Copy(m_Path);
}

inline const CStringArray *CMultiFileDlg::GetPathArray() const
{
	return(&m_Path);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTIFILEDLG_H__EEE10EF0_3E57_4C15_BD7E_FA68591A7C88__INCLUDED_)
