// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27jul05	initial version
        01      18apr06	add FIO_bool
		02		28jan08	support Unicode

        type-driven formatted I/O
 
*/

#ifndef CFORMATIO_INCLUDED
#define CFORMATIO_INCLUDED

enum {
	FIO_INT,
	FIO_UINT,
	FIO_FLOAT,
	FIO_DOUBLE,
	FIO_DPOINT,
	FIO_SHORT,
	FIO_USHORT,
	FIO_BYTE
};

#define FIO_bool FIO_BYTE

class CFormatIO {
public:
	static	bool	ValToStr(int Type, const PVOID Val, CString& Str);
	static	bool	ValToStr(int Type, const PVOID Val, CString& Str, int Precision);
	static	bool	StrToVal(int Type, LPCTSTR Str, void *Val);
};

#endif
