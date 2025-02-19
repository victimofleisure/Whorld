// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		15aug04	initial version
		01		09sep14	add WCopyable to allow default memberwise copy
 
        minimal base class for objects not derived from CObject
 
*/

class WObject {
public:
	WObject() {}	// default ctor required

private:
	WObject(const WObject&);	// prevent default copy
	WObject& operator=(const WObject&);	// prevent default assignment
};

class WCopyable : public WObject {
public:
	WCopyable() {}	// default ctor required
	WCopyable(const WCopyable&) {}	// allow default copy and assignment
	WCopyable& operator=(const WCopyable&) { return *this; }
};
