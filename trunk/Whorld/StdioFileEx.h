// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		06mar25	initial version

*/

#pragma once

// stream file that optionally supports UTF-8 character encoding
class CStdioFileEx : public CStdioFile {
public:
	CStdioFileEx(LPCTSTR lpszFileName, UINT nOpenFlags, bool bUTF8 = true);
};

// log file always uses UTF-8 and opens in append mode with commit
class CStdioLogFile : public CStdioFile {
public:
	CStdioLogFile(LPCTSTR lpszFileName);
};
