// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27jul05	initial version
		01		28jan08	support Unicode
		02		29jan08	in StrToVal, add static cast to fix warning

        type-driven formatted I/O
 
*/

#include "stdafx.h"
#include "FormatIO.h"

#define VALTOSTR(type, fmt) case FIO_##type: Str.Format(fmt, *((type *)Val)); break;
#define VALTOSTR_PREC(type, fmt, prec) case FIO_##type: Str.Format(fmt, prec, *((type *)Val)); break;
#define STRTOVAL(type, fmt) case FIO_##type: _stscanf(Str, fmt, Val); break;

bool CFormatIO::ValToStr(int Type, const PVOID Val, CString& Str)
{
	switch (Type) {
	VALTOSTR(INT,		_T("%d"));
	VALTOSTR(UINT,		_T("%u"));
	VALTOSTR(FLOAT,		_T("%g"));
	VALTOSTR(DOUBLE,	_T("%g"));
	case FIO_DPOINT:
		Str.Format(_T("%g %g"), ((DPOINT *)Val)->x, ((DPOINT *)Val)->y);
		break;
	VALTOSTR(SHORT,		_T("%hd"));
	VALTOSTR(USHORT,	_T("%hu"));
	VALTOSTR(BYTE,		_T("%u"));
	default:
		return(FALSE);
	}
	return(TRUE);
}

bool CFormatIO::ValToStr(int Type, const PVOID Val, CString& Str, int Precision)
{
	switch (Type) {
	VALTOSTR_PREC(FLOAT,	_T("%.*f"), Precision);
	VALTOSTR_PREC(DOUBLE,	_T("%.*f"), Precision);
	default:
		return(ValToStr(Type, Val, Str));
	}
	return(TRUE);
}

bool CFormatIO::StrToVal(int Type, LPCTSTR Str, void *Val)
{
	switch (Type) {
	STRTOVAL(INT,		_T("%d"));
	STRTOVAL(UINT,		_T("%u"));
	STRTOVAL(FLOAT,		_T("%f"));
	STRTOVAL(DOUBLE,	_T("%lf"));
	case FIO_DPOINT:
		_stscanf(Str, _T("%lf %lf"), &((DPOINT *)Val)->x, &((DPOINT *)Val)->y);
		break;
	STRTOVAL(SHORT,		_T("%hd"));
	STRTOVAL(USHORT,	_T("%hu"));
	case FIO_BYTE:
		{
			int	i;
			_stscanf(Str, _T("%d"), &i);
			*((BYTE *)Val) = static_cast<BYTE>(i);
		}
	default:
		return(FALSE);
	}
	return(TRUE);
}

