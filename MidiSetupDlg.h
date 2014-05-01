// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      13aug05	initial version
		01		17feb06	add oscillator properties
		02		10dec07	add global parameters
		03		22jan08	main now calls OnMidiIn directly
		04		23jan08	replace MIDI range scaler with start/end
		05		28jan08	support Unicode
		06		29jan08	in SetValue, add static cast to fix warning

		MIDI setup dialog
 
*/

#if !defined(AFX_MIDISETUPDLG_H__88433D48_4449_41E5_8534_6B2DFE3FAA5B__INCLUDED_)
#define AFX_MIDISETUPDLG_H__88433D48_4449_41E5_8534_6B2DFE3FAA5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MidiSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupDlg dialog

#include "RowDialog.h"
#include "ParmInfo.h"
#include "MidiSetupRow.h"
#include "MidiAutoAssignDlg.h"
#include "MidiIO.h"

class CMidiSetupDlg : public CRowDialog
{
	DECLARE_DYNAMIC(CMidiSetupDlg);
// Construction
public:
	CMidiSetupDlg(CWnd* pParent = NULL);   // standard constructor

// Constants
	enum {	// columns
		COL_TITLE,
		COL_RANGE_START,
		COL_RANGE_END,
		COL_EVENT,
		COL_CHAN,
		COL_CTRL,
		COL_VALUE,
		COLS
	};
	#define MIDI_PROP(name, tag, start, end, ctrl) MP_##tag,
	enum {	// miscellaneous properties
		#include "MidiProps.h"
		MISC_PROPS
	};
	enum {
		PARMS			= CParmInfo::ROWS,
		GLOBAL_PARMS	= CParmInfo::GLOBAL_PARMS,
		OSC_PROPS		= CParmInfo::OSC_PROPS,
		OSC_ROWS		= CParmInfo::ROWS * OSC_PROPS,
		GLOB_OSC_ROWS	= GLOBAL_PARMS * OSC_PROPS,
		FIRST_PARM_ROW	= 0,
		FIRST_MISC_ROW	= FIRST_PARM_ROW + PARMS,
		FIRST_GLOB_ROW	= FIRST_MISC_ROW + MISC_PROPS,
		FIRST_OSC_ROW	= FIRST_GLOB_ROW + GLOBAL_PARMS,
		FIRST_GLOB_OSC_ROW	= FIRST_OSC_ROW + OSC_ROWS,
		ROWS			= FIRST_GLOB_OSC_ROW + GLOB_OSC_ROWS,
		MIDI_CHANS		= 16,
		MIDI_PARMS		= 128,
		CENTER_POS		= 64
	};
	enum {	// event types
		ET_OFF		= CMidiSetupRow::ET_OFF,
		ET_CTRL		= CMidiSetupRow::ET_CTRL,
		ET_NOTE		= CMidiSetupRow::ET_NOTE,
		ET_PITCH	= CMidiSetupRow::ET_PITCH
	};

// Types
	typedef CMidiSetupRow::INFO ROWINFO;
	typedef CMidiSetupRow::DRANGE DRANGE;
	typedef struct tagINFO {
		ROWINFO	Row[ROWS];
	} INFO;

// Attributes
	void	GetDefaults(INFO& Info) const;
	void	GetInfo(INFO& Info) const;
	void	SetInfo(const INFO& Info);
	void	GetRange(int Idx, DRANGE& Range) const;
	int		GetMapping(int Event, int Chan, int Ctrl) const;
	int		GetCtrlMapping(int Chan, int Ctrl) const;
	int		GetNoteMapping(int Chan, int Note) const;
	int		GetPitchMapping(int Chan) const;
	void	SetValue(int Idx, int Value);
	int		GetValue(int Idx) const;
	LPCTSTR	GetRowName(int Idx) const;
	LPCTSTR	GetRowCaption(int Idx) const;
	int		GetRowIdx(int Pos) const;
	int		GetCount() const;
	bool	IsDefault(int Idx) const;
	void	SetLearn(bool Enable);
	bool	IsLearning() const;

// Operations
	void	Assign(int Idx, int Event, int Chan, int Ctrl);
	void	RestoreDefaults();
	void	ZeroControllers();
	bool	AutoAssign();
	void	OnMidiIn(CMidiIO::MSG msg);
	static	void	FixMidiRange(int RowIdx, CMidiSetupRow::INFO& Info);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMidiSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMidiSetupDlg)
	enum { IDD = IDD_MIDI_SETUP };
	CButton	m_AdvancedChk;
	CButton	m_LearnChk;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMidiSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnRestoreDefaults();
	afx_msg void OnZeroControls();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnAutoAssign();
	afx_msg void OnAdvanced();
	afx_msg void OnLearn();
	//}}AFX_MSG
	afx_msg LRESULT OnMidiRowEdit(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMidiRowSel(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Types
	typedef struct tagMISCPROPDATA {	// data about each miscellaneous property
		LPCTSTR	Name;		// name for tagged I/O
		int		TitleID;	// title string resource ID
		DRANGE	DefRange;	// default range
		int		DefCtrl;	// default controller number
	} MISCPROPDATA;
	typedef struct tagOSCPROPDATA {		// data about each oscillator property
		LPCTSTR	Suffix;		// suffix for tagged I/O
		int		TitleID;	// title string resource ID
	} OSCPROPDATA;

// Constants
	#define MIDI_CTRLR_DEF(name) name,
	enum {	// generate MIDI enum from header file via macro
		#include "MidiCtrlrDef.h"
	};
	enum {
		TOP_MARGIN		= 40,
		DEFAULT_CHAN	= 0,
		DEFAULT_CTRL	= 7
	};
	static const COLINFO		m_ColInfo[COLS];
	static const MISCPROPDATA	m_MiscPropData[MISC_PROPS];
	static const OSCPROPDATA	m_OscPropData[OSC_PROPS];

// Member data
	CMidiAutoAssignDlg	m_AutoDlg;
	int		m_RowPos[ROWS];				// display position of each row
	int		m_RowIdx[ROWS];				// storage position of each row
	BYTE	m_CtrlMap[MIDI_CHANS][MIDI_PARMS];	// map of controller assignments
	BYTE	m_NoteMap[MIDI_CHANS][MIDI_PARMS];	// map of note assignments
	BYTE	m_PitchMap[MIDI_CHANS];		// map of pitch bend assignments
	BYTE	m_Value[ROWS];				// shadow of MIDI value
	INFO	m_Info;						// information about each property
	CStringArray	m_RowName;			// name of each row for tagged I/O
	CStringArray	m_RowCaption;		// title of each row for display
	INFO	m_DefInfo;		// default row info
	int		m_RowSel;		// current row selection, or -1 if none
	bool	m_ShowRowSel;	// true if row selection is visible
	bool	m_Learn;		// true if we're in learn mode

// Overrides
	CWnd	*CreateRow(int Idx, int& Pos);

// Helpers
	CMidiSetupRow	*GetRow(int Idx) const;
	void	MakeRowTables();
	void	MakeDefaults(INFO& Info) const;
	void	MakeMidiMap();
	void	SelectRow(int RowIdx);
	void	ShowRowSel(bool Enable);
	static	DRANGE	MakeRange(double Start, double End);
};

inline CMidiSetupRow *CMidiSetupDlg::GetRow(int Idx) const
{
	return((CMidiSetupRow *)m_Row[Idx]);
}

inline int CMidiSetupDlg::GetCount() const
{
	return(m_Row.GetSize());
}

inline void CMidiSetupDlg::GetRange(int Idx, DRANGE& Range) const
{
	Range = m_Info.Row[Idx].Range;
}

inline int CMidiSetupDlg::GetCtrlMapping(int Chan, int Ctrl) const
{
	ASSERT(Chan >= 0 && Chan < MIDI_CHANS);
	ASSERT(Ctrl >= 0 && Ctrl < MIDI_PARMS);
	return(int(m_CtrlMap[Chan][Ctrl]) - 1);
}

inline int CMidiSetupDlg::GetNoteMapping(int Chan, int Note) const
{
	ASSERT(Chan >= 0 && Chan < MIDI_CHANS);
	ASSERT(Note >= 0 && Note < MIDI_PARMS);
	return(int(m_NoteMap[Chan][Note]) - 1);
}

inline int CMidiSetupDlg::GetPitchMapping(int Chan) const
{
	ASSERT(Chan >= 0 && Chan < MIDI_CHANS);
	return(int(m_PitchMap[Chan]) - 1);
}

inline void CMidiSetupDlg::SetValue(int Idx, int Value)
{
	ASSERT(Idx >= 0 && Idx <= ROWS);
	m_Value[Idx] = static_cast<BYTE>(Value);
}

inline int CMidiSetupDlg::GetValue(int Idx) const
{
	ASSERT(Idx >= 0 && Idx <= ROWS);
	return(m_Value[Idx]);
}

inline int CMidiSetupDlg::GetRowIdx(int Pos) const
{
	ASSERT(Pos >= 0 && Pos <= ROWS);
	return(m_RowIdx[Pos]);
}

inline LPCTSTR CMidiSetupDlg::GetRowName(int Idx) const
{
	ASSERT(Idx >= 0 && Idx <= ROWS);
	return(m_RowName[Idx]);
}

inline LPCTSTR CMidiSetupDlg::GetRowCaption(int Idx) const
{
	ASSERT(Idx >= 0 && Idx < ROWS);
	return(m_RowCaption[Idx]);
}

inline bool CMidiSetupDlg::IsLearning() const
{
	return(m_Learn);
}

inline CMidiSetupDlg::DRANGE CMidiSetupDlg::MakeRange(double Start, double End)
{
	DRANGE	Range;
	Range.Start = Start;
	Range.End = End;
	return(Range);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIDISETUPDLG_H__88433D48_4449_41E5_8534_6B2DFE3FAA5B__INCLUDED_)
