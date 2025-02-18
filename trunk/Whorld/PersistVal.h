// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      31jul05	initial version
		01		28jan08	support Unicode

		template for persistent values
 
*/

#ifndef CPERSISTVAL_INCLUDED
#define	CPERSISTVAL_INCLUDED

#include "Persist.h"

template<class T> class PersistVal;

#pragma warning(disable : 4800)	// forcing value to bool 'true' or 'false'

#define PERSIST_VAL(ctype, pftype) \
template<> class PersistVal<ctype> { \
public: \
	PersistVal(LPCTSTR Name, ctype& Val, ctype Default) : m_Name(Name), m_Val(Val) \
		{ Val = CPersist::Get##pftype(REG_SETTINGS, Name, Default); } \
	~PersistVal() \
		{ CPersist::Write##pftype(REG_SETTINGS, m_Name, m_Val); } \
private: \
	LPCTSTR	m_Name; \
	ctype&	m_Val; \
};

// specialize template for common types
PERSIST_VAL(int, Int)
PERSIST_VAL(UINT, Int)
PERSIST_VAL(float, Float)
PERSIST_VAL(double, Double)
PERSIST_VAL(bool, Int)

#pragma warning(default : 4800)

#endif
