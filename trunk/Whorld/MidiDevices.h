// Copyleft 2018 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17apr18	initial version
		01		27mar20	add device accessors
		02		18feb22	move index validator to array base class

*/

#pragma once

#include "ArrayEx.h"

class CMidiDevices {
public:
// Construction
	CMidiDevices();

// Constants
	enum {	// device types
		INPUT,
		OUTPUT,
		DEVICE_TYPES
	};
	enum {	// change mask bits
		CM_INPUT	= (1 << INPUT),			// input device index changed
		CM_OUTPUT	= (1 << OUTPUT),		// output device index changed
		CM_CHANGE	= (1 << DEVICE_TYPES),	// device enumeration changed
	};

// Types
	class CDevice {
	public:
	// Construction
		CDevice();
		CDevice(CString sName, CString sID);

	// Member data
		CString	m_sName;	// device name
		CString	m_sID;		// device identifier

	// Operations
		bool	operator==(const CDevice& dev) const;
		bool	operator!=(const CDevice& dev) const;
		void	Empty();
	};
	class CSelection {
	public:
		bool	operator==(const CSelection& dev) const;
		bool	operator!=(const CSelection& dev) const;
		CDevice	m_arrDev[DEVICE_TYPES];		// array of selected device info
	};

// Attributes
	bool	IsEmpty(int iType) const;
	int		GetIdx(int iType) const;
	void	SetIdx(int iType, int iDev);
	int		GetCount(int iType) const;
	CString	GetName(int iType) const;
	CString	GetID(int iType) const;
	void	GetDevice(int iType, CDevice& dev) const;
	bool	IsDeviceIdx(int iType, int iDev) const;
	CString	GetName(int iType, int iDev) const;
	CString	GetID(int iType, int iDev) const;
	void	GetDevice(int iType, int iDev, CDevice& dev) const;
	void	GetSelection(CSelection& sel) const;
	static	CString	GetTypeCaption(int iType);

// Operations
	bool	operator==(const CMidiDevices& devs) const;
	bool	operator!=(const CMidiDevices& devs) const;
	void	Update();
	void	Read();
	void	Write();
	void	Dump() const;
	void	Dump(CString& str, bool bShowIDs = false) const;
	void	DumpSystemState(CString& str) const;
	bool	OnDeviceChange(UINT& nChangeMask);

protected:
// Types
	class CDeviceArray : public CArrayEx<CDevice, CDevice&> {
	public:
	// Construction
		CDeviceArray();

	// Attributes
		int		GetIdx() const;
		void	SetIdx(int iDev);
		CString	GetName(int iDev) const;
		CString	GetID(int iDev) const;
		void	GetDevice(int iDev, CDevice& dev) const;

	// Operations
		bool	operator==(const CDeviceArray& arrDev) const;
		bool	operator!=(const CDeviceArray& arrDev) const;
		int		Find(CString sName, CString sID) const;
		int		Find(CString sName) const;
		int		Find(const CDeviceArray& arrDev) const;
		int		GetNameCount(CString sName) const;

	protected:
	// Member data
		int		m_iDev;		// device index
	};

protected:
// Member data
	CDeviceArray	m_arrDev[DEVICE_TYPES];		// array of device arrays

// Constants
	static const LPCTSTR	m_rkDevName[DEVICE_TYPES];		// device name registry keys
	static const LPCTSTR	m_rkDevID[DEVICE_TYPES];		// device ID registry keys
	static const int		m_nDevCaption[DEVICE_TYPES];	// device type captions

// Helpers
	bool	OnDeviceChange(const CMidiDevices& devsPrev, UINT& nChangeMask);
};

inline CMidiDevices::CDevice::CDevice()
{
}

inline CMidiDevices::CDevice::CDevice(CString sName, CString sID) : m_sName(sName), m_sID(sID)
{
}

inline bool CMidiDevices::CDevice::operator!=(const CDevice& dev) const
{
	return !operator==(dev);
}

inline void CMidiDevices::CDevice::Empty()
{
	m_sID.Empty();
	m_sName.Empty();
}

inline bool CMidiDevices::CSelection::operator!=(const CSelection& sel) const
{
	return !operator==(sel);
}

inline CMidiDevices::CDeviceArray::CDeviceArray()
{
	m_iDev = -1;
}

inline int CMidiDevices::CDeviceArray::GetIdx() const
{
	return m_iDev;
}

inline bool CMidiDevices::CDeviceArray::operator!=(const CDeviceArray& arrDev) const
{
	return !operator==(arrDev);
}

inline CMidiDevices::CMidiDevices()
{
}

inline bool CMidiDevices::IsEmpty(int iType) const
{
	return GetIdx(iType) < 0;
}

inline int CMidiDevices::GetIdx(int iType) const
{
	return m_arrDev[iType].GetIdx();
}

inline void CMidiDevices::SetIdx(int iType, int iDev)
{
	m_arrDev[iType].SetIdx(iDev);
}

inline int CMidiDevices::GetCount(int iType) const
{
	return m_arrDev[iType].GetSize();
}

inline CString CMidiDevices::GetName(int iType) const
{
	return GetName(iType, GetIdx(iType));
}

inline CString CMidiDevices::GetID(int iType) const
{
	return GetID(iType, GetIdx(iType));
}

inline void CMidiDevices::GetDevice(int iType, CDevice& dev) const
{
	GetDevice(iType, GetIdx(iType), dev);
}

inline bool CMidiDevices::IsDeviceIdx(int iType, int iDev) const
{
	return m_arrDev[iType].IsIndex(iDev);
}

inline CString CMidiDevices::GetName(int iType, int iDev) const
{
	return m_arrDev[iType].GetName(iDev);
}

inline CString CMidiDevices::GetID(int iType, int iDev) const
{
	return m_arrDev[iType].GetID(iDev);
}

inline void CMidiDevices::GetDevice(int iType, int iDev, CDevice& dev) const
{
	m_arrDev[iType].GetDevice(iDev, dev);
}

inline CString CMidiDevices::GetTypeCaption(int iType)
{
	return LDS(m_nDevCaption[iType]);
}

inline bool CMidiDevices::operator!=(const CMidiDevices& devs) const
{
	return !operator==(devs);
}
