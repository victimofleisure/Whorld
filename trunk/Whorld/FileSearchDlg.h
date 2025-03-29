// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09jan04	initial version
		01		23nov07	support Unicode
		02		29mar25	modernize style

        abstract class to search for files recursively
 
*/

#pragma once

#include <afxmt.h>

// FileSearchDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFileSearchDlg dialog

class CFileSearchDlg : public CDialog
{
	DECLARE_DYNAMIC(CFileSearchDlg);
public:
// Construction
	CFileSearchDlg(LPCTSTR pszFolder, CWnd* pParentWnd = NULL);
	virtual ~CFileSearchDlg();

// Constants
	enum DRIVE_TYPE_MASK {
		DTM_UNKNOWN		= (1 << DRIVE_UNKNOWN),		// The drive type cannot be determined. 
		DTM_NO_ROOT_DIR	= (1 << DRIVE_NO_ROOT_DIR),	// The root directory does not exist. 
		DTM_REMOVABLE	= (1 << DRIVE_REMOVABLE),	// The disk can be removed from the drive. 
		DTM_FIXED		= (1 << DRIVE_FIXED),		// The disk cannot be removed from the drive. 
		DTM_REMOTE		= (1 << DRIVE_REMOTE),		// The drive is a remote (network) drive. 
		DTM_CDROM		= (1 << DRIVE_CDROM),		// The drive is a CD-ROM drive. 
		DTM_RAMDISK		= (1 << DRIVE_RAMDISK),		// The drive is a RAM disk. 
		DTM_ALL			= -1
	};

// Attributes
	void	SetDriveTypeMask(int nMask);

// Overrides
	// ClassWizard generated virtual function overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
// Dialog data
	enum { IDD = IDD_FILE_SEARCH };

// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg LRESULT OnUser(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Member data
	CString	m_sFolder;
	volatile bool m_bCancel;
	int		m_nDriveTypeMask;

// Helpers
	bool	SearchFolder(LPCTSTR pszFolder);
	bool	SearchAllDrives();
	void	Search();
	static	UINT	SearchThread(LPVOID pParam);
	virtual	bool	OnFile(LPCTSTR pszPath) = 0;
};
