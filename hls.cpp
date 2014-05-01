// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version

        convert between RGB and HLS color space
 
*/

#include "stdafx.h"
#include "hls.h"

double CHLS::mkrgb(double H, double M1, double M2)
{
	if (H < 0)
		H += 360;
	else if (H > 360)
		H -= 360;
	if (H < 60)
		return(M1 + (M2 - M1) * H / 60);
	else if (H >= 60 && H < 180)
		return(M2);
	else if (H >= 180 && H < 240)
		return(M1 + (M2 - M1) * (240 - H) / 60);
	else
		return(M1);
}

void CHLS::hls2rgb(double H, double L, double S, 
				  double& R, double &G, double &B)
{
	double  M1, M2;
	if (L <= 0.5)
		M2 = L + L * S;
	else
		M2 = L + S - L * S;
	M1 = 2 * L - M2;
	R = mkrgb(H + 120, M1, M2);
	G = mkrgb(H, M1, M2);
	B = mkrgb(H - 120, M1, M2);
}

void CHLS::rgb2hls(double R, double G, double B,
				  double& H, double& L, double& S)
						 
{
	double	M1 = max(max(R, G), B);
	double	M2 = min(min(R, G), B);
	L = (M1 + M2) / 2;
	if (M1 == M2) {
		S = 0;
		H = 0;
	} else {
		double	delta = M1 - M2;
		if (L <= 0.5)
			S = delta / (M1 + M2);
		else
			S = delta / (2 - M1 - M2);
		if (M1 == R)
			H = (G - B) / delta;
		else if (M1 == G)
			H = 2 + (B - R) / delta;
		else
			H = 4 + (R - G) / delta;
		H *= 60;
		if (H < 0)
			H += 360;
	}
}
