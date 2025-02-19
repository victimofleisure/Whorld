// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		21nov12	initial version
        01		10jun17	overload multiply and divide
        02		10jun17	remove self-assignment check
        03		13feb18	add IsNull and Include
        04		05may18	overload assignment operator for single value

		range template

*/

#pragma once

template<class T>
class CRange {
public:
// Construction
	CRange();
	CRange(const CRange& Range);
	CRange(T Start, T End);
	CRange& operator=(const CRange& Range);
	CRange& operator=(T Val);

// Attributes
	T		Length() const;
	void	SetEmpty();
	bool	IsEmpty() const;
	bool	IsNull() const;
	bool	IsNormalized() const;

// Operations
	void	Normalize();
	bool	InRange(T Val) const;
	bool	Intersect(const CRange& Range1, const CRange& Range2);
	bool	Union(const CRange& Range1, const CRange& Range2);
	void	Include(const CRange& Range);

// Operators
	bool	operator==(const CRange& Range) const;
	bool	operator!=(const CRange& Range) const;
	const CRange operator+(const CRange& Range) const;
	const CRange operator-(const CRange& Range) const;
	const CRange operator*(const CRange& Range) const;
	const CRange operator/(const CRange& Range) const;
	const CRange operator+(T Val) const;
	const CRange operator-(T Val) const;
	const CRange operator*(T Val) const;
	const CRange operator/(T Val) const;
	CRange& operator+=(const CRange& Range);
	CRange& operator-=(const CRange& Range);
	CRange& operator*=(const CRange& Range);
	CRange& operator/=(const CRange& Range);
	CRange& operator+=(T Val);
	CRange& operator-=(T Val);
	CRange& operator*=(T Val);
	CRange& operator/=(T Val);
	CRange	operator&(const CRange& Range2) const;
	CRange	operator|(const CRange& Range2) const;

// Data members
	T		Start;
	T		End;
};

template<class T>
inline CRange<T>::CRange()
{
}

template<class T>
inline CRange<T>::CRange(const CRange& Range)
{
	Start = Range.Start;
	End = Range.End;
}

template<class T>
inline CRange<T>::CRange(T Start, T End)
{
	this->Start = Start;
	this->End = End;
}

template<class T>
inline CRange<T>& CRange<T>::operator=(const CRange& Range)
{
	Start = Range.Start;
	End = Range.End;
	return(*this);
}

template<class T>
inline CRange<T>& CRange<T>::operator=(T Val)
{
	Start = Val;
	End = Val;
	return(*this);
}

template<class T>
inline T CRange<T>::Length() const
{
	return(End - Start);
}

template<class T>
inline void CRange<T>::SetEmpty()
{
	Start = 0;
	End = 0;
}

template<class T>
inline bool CRange<T>::IsEmpty() const
{
	return(Start == End);
}

template<class T>
inline bool CRange<T>::IsNull() const
{
	return(!Start && !End);
}

template<class T>
inline bool CRange<T>::IsNormalized() const
{
	return(Start <= End);
}

template<class T>
inline void CRange<T>::Normalize()
{
	if (!IsNormalized()) {
		T	tmp = Start;
		Start = End;
		End = tmp;
	}
}

template<class T>
inline bool CRange<T>::InRange(T Val) const
{
	return(Val >= Start && Val < End);
}

template<class T>
inline bool CRange<T>::operator==(const CRange& Range) const
{
	return(Range.Start == Start && Range.End == End);
}

template<class T>
inline bool CRange<T>::operator!=(const CRange& Range) const
{
	return(Range.Start != Start || Range.End != End);
}

template<class T>
const inline CRange<T> CRange<T>::operator+(const CRange& Range) const
{
	return(CRange(Start + Range.Start, End + Range.End));
}

template<class T>
const inline CRange<T> CRange<T>::operator-(const CRange& Range) const
{
	return(CRange(Start - Range.Start, End - Range.End));
}

template<class T>
const inline CRange<T> CRange<T>::operator*(const CRange& Range) const
{
	return(CRange(Start * Range.Start, End * Range.End));
}

template<class T>
const inline CRange<T> CRange<T>::operator/(const CRange& Range) const
{
	return(CRange(Start / Range.Start, End / Range.End));
}

template<class T>
const inline CRange<T> CRange<T>::operator+(T Val) const
{
	return(*this + CRange(Val, Val));
}

template<class T>
const inline CRange<T> CRange<T>::operator-(T Val) const
{
	return(*this - CRange(Val, Val));
}

template<class T>
const inline CRange<T> CRange<T>::operator*(T Val) const
{
	return(*this * CRange(Val, Val));
}

template<class T>
const inline CRange<T> CRange<T>::operator/(T Val) const
{
	return(*this / CRange(Val, Val));
}

template<class T>
inline CRange<T>& CRange<T>::operator+=(const CRange& Range)
{
	return(*this = *this + Range);
}

template<class T>
inline CRange<T>& CRange<T>::operator-=(const CRange& Range)
{
	return(*this = *this - Range);
}

template<class T>
inline CRange<T>& CRange<T>::operator*=(const CRange& Range)
{
	return(*this = *this * Range);
}

template<class T>
inline CRange<T>& CRange<T>::operator/=(const CRange& Range)
{
	return(*this = *this / Range);
}

template<class T>
inline CRange<T>& CRange<T>::operator+=(T Val)
{
	return(*this = *this + Val);
}

template<class T>
inline CRange<T>& CRange<T>::operator-=(T Val)
{
	return(*this = *this - Val);
}

template<class T>
inline CRange<T>& CRange<T>::operator*=(T Val)
{
	return(*this = *this * Val);
}

template<class T>
inline CRange<T>& CRange<T>::operator/=(T Val)
{
	return(*this = *this / Val);
}

template<class T>
inline bool CRange<T>::Intersect(const CRange& Range1, const CRange& Range2)
{
	if (!Range1.IsEmpty() && !Range2.IsEmpty()) {	// if both ranges non-empty
		Start = max(Range1.Start, Range2.Start);	// calculate intersection
		End = min(Range1.End, Range2.End);
		if (Start < End)	// if valid intersection
			return(TRUE);	// success
	}
	SetEmpty();	// one or both ranges empty, or no intersection: empty ourself
	return(FALSE);	// fail
}

template<class T>
inline bool CRange<T>::Union(const CRange& Range1, const CRange& Range2)
{
	if (Range1.IsEmpty()) {	// if Range1 empty
		if (Range2.IsEmpty()) {	// if Range2 empty
			SetEmpty();	// both ranges empty: empty ourself
			return(FALSE);	// fail
		} else	// Range2 non-empty
			*this = Range2;	// copy Range2
	} else {	// Range1 non-empty
		if (Range2.IsEmpty())	// if Range2 empty
			*this = Range1;	// copy Range1
		else {	// both ranges non-empty
			Start = min(Range1.Start, Range2.Start);	// calculate union
			End = max(Range1.End, Range2.End);
		}
	}
	return(TRUE);	// success
}

template<class T>
inline void CRange<T>::Include(const CRange& Range)
{
	if (Range.Start < Start)
		Start = Range.Start;
	if (Range.End > End)
		End = Range.End;
}

template<class T>
inline CRange<T> CRange<T>::operator&(const CRange& Range2) const
{
	CRange	range;
	range.Intersect(*this, Range2);
	return(range);
}

template<class T>
inline CRange<T> CRange<T>::operator|(const CRange& Range2) const
{
	CRange	range;
	range.Union(*this, Range2);
	return(range);
}
