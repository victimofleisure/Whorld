// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      24mar05	initial version
		01		05apr17	add inline attribute to fix linker errors

        define registry templates
 
*/

#pragma once

#include "Persist.h"

template<class T>
inline void RdReg(LPCTSTR Key, T& Value)
{
	DWORD	Size = sizeof(T);
	CPersist::GetBinary(REG_SETTINGS, Key, &Value, &Size);
}
template<class T>	
inline void WrReg(LPCTSTR Key, const T& Value)
{
	CPersist::WriteBinary(REG_SETTINGS, Key, &Value, sizeof(T));
}
#define	WINAPPCK_REG_TYPE_DEF(T, prefix) \
	inline void RdReg(LPCTSTR Key, T& Value)	\
	{	\
		Value = static_cast<T>(prefix CPersist::GetInt(REG_SETTINGS, Key, Value));	\
	}
#include "RegTemplTypes.h"	// specialize RdReg for numeric types
#define	WINAPPCK_REG_TYPE_DEF(T, prefix) \
	inline void WrReg(LPCTSTR Key, const T& Value)	\
	{	\
		CPersist::WriteInt(REG_SETTINGS, Key, Value);	\
	}
#include "RegTemplTypes.h"	// specialize WrReg for numeric types
inline void RdReg(LPCTSTR Key, CString& Value)
{
	Value = CPersist::GetString(REG_SETTINGS, Key, Value);
}
inline void WrReg(LPCTSTR Key, const CString& Value)
{
	CPersist::WriteString(REG_SETTINGS, Key, Value);
}

// Persistence in specified section
template<class T>
inline void RdReg(LPCTSTR Section, LPCTSTR Key, T& Value)
{
	DWORD	Size = sizeof(T);
	CPersist::GetBinary(Section, Key, &Value, &Size);
}
template<class T>	
inline void WrReg(LPCTSTR Section, LPCTSTR Key, const T& Value)
{
	CPersist::WriteBinary(Section, Key, &Value, sizeof(T));
}
#define	WINAPPCK_REG_TYPE_DEF(T, prefix) \
	inline void RdReg(LPCTSTR Section, LPCTSTR Key, T& Value)	\
	{	\
		Value = static_cast<T>(prefix CPersist::GetInt(Section, Key, Value));	\
	}
#include "RegTemplTypes.h"	// specialize RdReg for numeric types
#define	WINAPPCK_REG_TYPE_DEF(T, prefix) \
	inline void WrReg(LPCTSTR Section, LPCTSTR Key, const T& Value)	\
	{	\
		CPersist::WriteInt(Section, Key, Value);	\
	}
#include "RegTemplTypes.h"	// specialize WrReg for numeric types
inline void RdReg(LPCTSTR Section, LPCTSTR Key, CString& Value)
{
	Value = CPersist::GetString(Section, Key, Value);
}
inline void WrReg(LPCTSTR Section, LPCTSTR Key, const CString& Value)
{
	CPersist::WriteString(Section, Key, Value);
}
