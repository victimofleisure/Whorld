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
		02		18apr06	bump file version to 2
		03		21jun06	add tag to MasterDef macro
		04		28jan08	support Unicode

		patch container
 
*/

#ifndef CPATCH_INCLUDED
#define CPATCH_INCLUDED

#include "ParmInfo.h"

class CPatch : public CParmInfo {
public:
// Construction
	CPatch() {}	// use SetDefaults to initialize this object
	~CPatch() {}

// Types
	#undef MASTERDEF
	#define MASTERDEF(tag, name, type) type name;
	typedef struct tagMASTER {
		#include "MasterDef.h"
	} MASTER;
	#undef MAINDEF
	#define MAINDEF(name, type) type name;
	typedef struct tagMAIN {
		#include "MainDef.h"
	} MAIN;
	typedef struct tagLINE_INFO {
		LPCTSTR	Name;		// field's unique identifier
		int		Type;		// field's data type; see enum in FormatIO.h
		int		Offset;		// field's byte offset within this class
	} LINE_INFO;

// Constants
	enum {
		MAX_RINGS = 1000
	};
	static const MASTER	m_MasterDefaults;	// default values for master members
	static const MAIN	m_MainDefaults;		// default values for main members

// Public data
	MASTER	m_Master;
	MAIN	m_Main;

// Operations
	void	SetDefaults();
	bool	Read(CStdioFile& fp);
	bool	Read(LPCTSTR Path);
	bool	Write(CStdioFile& fp) const;
	bool	Write(LPCTSTR Path) const;
	void	operator=(const CParmInfo& Info);

private:
// Constants
	enum {
		FILE_VERSION = 2	// file format version number
	};
	static const LPCTSTR FILE_ID;		// file format signature
	static const LINE_INFO	m_LineInfo[];	// data for I/O of lines
};

inline void CPatch::operator=(const CParmInfo& Info)
{
	CParmInfo *p = this;	// upcast to base class pointer
	*p = Info;	// assign to base class
}

#endif
