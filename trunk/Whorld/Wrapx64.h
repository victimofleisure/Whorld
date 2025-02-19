// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		29dec12	initial version
        01		08apr13	add static cast methods
		02		09oct13	add minimum values
		03		22nov13	add static cast to ULONG

		x64 porting types and methods

*/

#ifdef _WIN64
#define GCL_HBRBACKGROUND GCLP_HBRBACKGROUND
#endif

#ifdef _WIN64
typedef INT_PTR W64INT;
typedef UINT_PTR W64UINT;
typedef ULONG_PTR W64ULONG;
#ifdef _DEBUG
#define INT64TO32(x) W64INT_STATIC_CAST32(x)
#define UINT64TO32(x) W64UINT_STATIC_CAST32(x)
#else
#define INT64TO32(x) static_cast<int>(x)
#define UINT64TO32(x) static_cast<UINT>(x)
#endif
#define W64INT_MAX LLONG_MAX
#define W64INT_MIN LLONG_MIN
#define W64UINT_MAX ULLONG_MAX
#define W64ULONG_MAX ULLONG_MAX
#else
typedef int W64INT;
typedef UINT W64UINT;
typedef ULONG W64ULONG;
#define INT64TO32(x) x
#define UINT64TO32(x) x
#define W64INT_MAX INT_MAX
#define W64INT_MIN INT_MIN
#define W64UINT_MAX UINT_MAX
#define W64ULONG_MAX ULONG_MAX
#endif

inline int W64INT_STATIC_CAST32(W64INT Val)
{
	ASSERT(Val >= INT_MIN && Val <= INT_MAX);
	return(static_cast<int>(Val));
}

inline UINT W64UINT_STATIC_CAST32(W64UINT Val)
{
	ASSERT(Val <= UINT_MAX);
	return(static_cast<UINT>(Val));
}

inline ULONG W64ULONG_STATIC_CAST32(W64ULONG Val)
{
	ASSERT(Val <= ULONG_MAX);
	return(static_cast<ULONG>(Val));
}

inline int W64INT_CAST32(W64INT Val)
{
#ifdef _WIN64
	if (Val < INT_MIN || Val > INT_MAX)
		AfxThrowNotSupportedException();
	return(static_cast<int>(Val));
#else
	return(Val);	// already 32-bit
#endif
}

inline UINT W64UINT_CAST32(W64UINT Val)
{
#ifdef _WIN64
	if (Val > UINT_MAX)
		AfxThrowNotSupportedException();
	return(static_cast<UINT>(Val));
#else
	return(Val);	// already 32-bit
#endif
}

inline ULONG W64ULONG_CAST32(W64ULONG Val)
{
#ifdef _WIN64
	if (Val > ULONG_MAX)
		AfxThrowNotSupportedException();
	return(static_cast<ULONG>(Val));
#else
	return(Val);	// already 32-bit
#endif
}

#if _MFC_VER < 0x0700
inline __int64 _abs64(__int64 x)
{
	return(x < 0 ? -x : x);
}
#endif
