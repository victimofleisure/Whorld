// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      12feb25	initial version

*/

#pragma once

#include <sstream>

#if defined(_UNICODE) || defined(UNICODE)	// if Unicode build
#define STD_STRING std::wstring
#define STD_OSTREAM std::wostream
#define STD_ISTREAM std::wistream
#define STD_OSTRINGSTREAM std::wostringstream
#define STD_ISTRINGSTREAM std::wistringstream
#else	// MBCS or ANSI build
#define STD_STRING std::string
#define STD_OSTREAM std::ostream
#define STD_ISTREAM std::istream
#define STD_OSTRINGSTREAM std::ostringstream
#define STD_ISTRINGSTREAM std::istringstream
#endif

// convert value to CString
template <typename T>
CString ValToString(const T& inVal)
{
	STD_OSTRINGSTREAM oss;
	oss << inVal;	// convert value to string
	return CString(oss.str().c_str());	// convert string to CString and return it
}

// convert CString to value
template <typename T>
bool StringToVal(const CString& strIn, T& outVal)
{
	STD_STRING s(strIn);	// convert CString to string
    STD_ISTRINGSTREAM iss(s);
    iss >> outVal;	// convert string to value
    return !iss.fail();	// return true if conversion succeeded
}
