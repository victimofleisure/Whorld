// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		28jan03	initial version

		double-precision 2D coordinate

*/

#include "stdafx.h"
#include "DPoint.h"
#include "math.h"

const double DPoint::Epsilon = 1e-10;

bool DPoint::Equal(double a, double b)
{
	return(fabs(a - b) < Epsilon);	// less than this and they're equal
}
