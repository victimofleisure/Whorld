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

#ifndef CHLS_INCLUDED
#define	CHLS_INCLUDED

class CHLS {
public:
	static	void	hls2rgb(double H, double L, double S,
							double& R, double &G, double &B);
	static	void	rgb2hls(double R, double G, double B, 
							double& H, double& L, double& S);

private:
	static	double	mkrgb(double H, double M1, double M2);
};

#endif
