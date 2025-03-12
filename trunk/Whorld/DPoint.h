// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		28jan03	initial version
		01		17apr06	add DPOINT ctor and assignment
		02		14feb08	add SIZE ctor
		03		09feb25	rename round function
		04		12feb25	add POINTFLOAT ctor and conversion
		05		14feb25	simplify base struct def
		06		03mar25	modernize style
		07		12mar25	add Direct2D types

		double-precision 2D coordinate

*/

#pragma once

struct DPOINT {
	double	x;
	double	y;
};

class DPoint : public DPOINT {
public:
	DPoint();
	DPoint(double x, double y);
	DPoint(const DPoint& p);
	DPoint(const DPOINT& p);
	DPoint(const POINT& p);
	DPoint(const SIZE& p);
	DPoint(const POINTFLOAT& p);
	DPoint(const D2D1_POINT_2F& p);
	DPoint(const D2D1_SIZE_F& p);
	DPoint& operator=(const DPoint& p);
	DPoint& operator=(const DPOINT& p);
	const DPoint operator+(const DPoint& p) const;
	const DPoint operator-(const DPoint& p) const;
	const DPoint operator*(const DPoint& p) const;
	const DPoint operator/(const DPoint& p) const;
	const DPoint operator+(double d) const;
	const DPoint operator-(double d) const;
	const DPoint operator*(double d) const;
	const DPoint operator/(double d) const;
	DPoint& operator+=(const DPoint& p);
	DPoint& operator-=(const DPoint& p);
	DPoint& operator*=(const DPoint& p);
	DPoint& operator/=(const DPoint& p);
	DPoint& operator+=(double d);
	DPoint& operator-=(double d);
	DPoint& operator*=(double d);
	DPoint& operator/=(double d);
	bool operator==(const DPoint& p) const;
	bool operator!=(const DPoint& p) const;
	const double operator[](int i) const;
	double& operator[](int i);
  	operator POINT() const;
	operator POINTFLOAT() const;
	operator D2D1_POINT_2F() const;
	static bool Equal(double a, double b);
	static const double m_fEpsilon;
};

inline DPoint::DPoint()
{
}

inline DPoint::DPoint(double x, double y)
{
	this->x = x;
	this->y = y;
}

inline DPoint::DPoint(const DPoint& p)
{
	x = p.x;
	y = p.y;
}

inline DPoint::DPoint(const DPOINT& p)
{
	x = p.x;
	y = p.y;
}

inline	DPoint::DPoint(const POINT& p)
{
	x = p.x;
	y = p.y;
}

inline	DPoint::DPoint(const SIZE& p)
{
	x = p.cx;
	y = p.cy;
}

inline	DPoint::DPoint(const POINTFLOAT& p)
{
	x = p.x;
	y = p.y;
}

inline	DPoint::DPoint(const D2D1_POINT_2F& p)
{
	x = p.x;
	y = p.y;
}

inline	DPoint::DPoint(const D2D1_SIZE_F& p)
{
	x = p.width;
	y = p.height;
}

inline DPoint& DPoint::operator=(const DPoint& p)
{
	if (this == &p)
		return *this;	// self-assignment
	x = p.x;
	y = p.y;
	return *this;
}

inline DPoint& DPoint::operator=(const DPOINT& p)
{
	if (this == &p)
		return *this;	// self-assignment
	x = p.x;
	y = p.y;
	return *this;
}

const inline DPoint DPoint::operator+(const DPoint& p) const
{
	return DPoint(x + p.x, y + p.y);
}

const inline DPoint DPoint::operator-(const DPoint& p) const
{
	return DPoint(x - p.x, y - p.y);
}

const inline DPoint DPoint::operator*(const DPoint& p) const
{
	return DPoint(x * p.x, y * p.y);
}

const inline DPoint DPoint::operator/(const DPoint& p) const
{
	return DPoint(x / p.x, y / p.y);
}

const inline DPoint DPoint::operator+(double d) const
{
	return *this + DPoint(d, d);
}

const inline DPoint DPoint::operator-(double d) const
{
	return *this - DPoint(d, d);
}

const inline DPoint DPoint::operator*(double d) const
{
	return *this * DPoint(d, d);
}

const inline DPoint DPoint::operator/(double d) const
{
	return *this / DPoint(d, d);
}

inline DPoint& DPoint::operator+=(const DPoint& p)
{
	return *this = *this + p;
}

inline DPoint& DPoint::operator-=(const DPoint& p)
{
	return *this = *this - p;
}

inline DPoint& DPoint::operator*=(const DPoint& p)
{
	return *this = *this * p;
}

inline DPoint& DPoint::operator/=(const DPoint& p)
{
	return *this = *this / p;
}

inline DPoint& DPoint::operator+=(double d)
{
	return *this = *this + d;
}

inline DPoint& DPoint::operator-=(double d)
{
	return *this = *this - d;
}

inline DPoint& DPoint::operator*=(double d)
{
	return *this = *this * d;
}

inline DPoint& DPoint::operator/=(double d)
{
	return *this = *this / d;
}

inline bool DPoint::operator==(const DPoint& p) const
{
	return Equal(p.x, x) && Equal(p.y, y);
}

inline bool DPoint::operator!=(const DPoint& p) const
{
	return !Equal(p.x, x) || !Equal(p.y, y);
}

inline const double DPoint::operator[](int i) const
{
	return ((double *)this)[i];
}

inline double& DPoint::operator[](int i)
{
	return ((double *)this)[i];
}

inline DPoint::operator POINT() const
{
	POINT	p;
	p.x = Round(x);
	p.y = Round(y);
	return p;
}

inline DPoint::operator POINTFLOAT() const
{
	POINTFLOAT	pt;
	pt.x = static_cast<float>(x);
	pt.y = static_cast<float>(y);
	return pt;
}

inline DPoint::operator D2D1_POINT_2F() const
{
	D2D1_POINT_2F	pt;
	pt.x = static_cast<float>(x);
	pt.y = static_cast<float>(y);
	return pt;
}
