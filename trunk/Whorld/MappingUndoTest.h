// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08oct13	initial version
        01      07may14	move generic functionality to base class
		02		09sep14	add CPatchObject
		03		02dec19	remove sort function, array now provides it
		04		19nov20	add randomized docking bar visibility
		05		10sep24	add method to randomize channel property
		06		02mar25	adapt for Whorld

		automated undo test for Whorld mapping
 
*/

#pragma once

#include "UndoTest.h"
#include "Mapping.h"

class CWhorldDoc;

class CMappingUndoTest : public CUndoTest, public CMappingBase {
public:
// Construction
	CMappingUndoTest(bool bInitRunning);
	virtual ~CMappingUndoTest();

protected:
// Types

// Constants
	static const EDIT_INFO	m_arrEditInfo[];	// array of edit properties

// Data members

// Overrides
	virtual	bool	Create();
	virtual	void	Destroy();
	virtual	int		ApplyEdit(int nUndoCode);
	virtual	LONGLONG	GetSnapshot() const;

// Helpers
	int		MakeRandomMappingProperty(int iProp);
	bool	MakeRandomSelection(int nItems, CIntArrayEx& arrSelection) const;
	CString	PrintSelection(CIntArrayEx& arrSelection) const;
};
