// Copyleft 2017 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      23mar17	initial version
		01		03nov17	remove group array; add subitem expansion
		02		26mar18	add Enable and IsEnabled
		03		10apr18	add runtime class macros
		04		18apr18	add GetValueProperty and UpdateOptions
		05		17may18	add enum property to allow duplicate options
		06		17may18	add spin control for floating point values
		07		04jun18	add time property type
		08		26sep18	in enum property, overload PushChar and OnDblClk

*/

#pragma once

#include "afxpropertygridctrl.h"
#include "ArrayEx.h"

class CProperties;

class CValidPropertyGridCtrl : public CMFCPropertyGridCtrl {
public:
	DECLARE_DYNAMIC(CValidPropertyGridCtrl)

// Construction
	CValidPropertyGridCtrl();

// Attributes
	int		GetActualDescriptionRows() const;

// Operations
	void	SaveGroupExpansion(LPCTSTR pszRegKey) const;
	void	RestoreGroupExpansion(LPCTSTR pszRegKey) const;

protected:
// Overrides
	virtual BOOL ValidateItemData(CMFCPropertyGridProperty* pProp);
	virtual	BOOL EndEditItem(BOOL bUpdateData = TRUE);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

// Data members
	bool	m_bInPreTranslateMsg;
	bool	m_bIsDataValidated;

// Helpers
	void	TrackToolTip(CPoint point);
	void	SaveSubitemExpansion(CString sRegKey, const CMFCPropertyGridProperty *pProp) const;
	void	RestoreSubitemExpansion(CString sRegKey, CMFCPropertyGridProperty *pProp) const;

// Message handlers
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSpinDeltaPos(NMHDR* pNMHDR, LRESULT* pResult);	
};

class CValidPropertyGridProperty : public CMFCPropertyGridProperty {
public:
	DECLARE_DYNAMIC(CValidPropertyGridProperty)

// Construction
	CValidPropertyGridProperty(const CString& strGroupName, DWORD_PTR dwData = 0, BOOL bIsValueList = FALSE) 
		: CMFCPropertyGridProperty(strGroupName, dwData, bIsValueList) {}
	CValidPropertyGridProperty(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0,
		LPCTSTR lpszEditMask = NULL, LPCTSTR lpszEditTemplate = NULL, LPCTSTR lpszValidChars = NULL) 
		: CMFCPropertyGridProperty(strName, varValue, lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars) {}

// Attributes
	void	GetValueRange(int& nMinVal, int& nMaxVal) const;

// Data members
	CComVariant	m_vMinVal;
	CComVariant	m_vMaxVal;

// Overrides
	virtual CString FormatProperty();
	void	EnableSpinControl(BOOL bEnable, int nMin, int nMax);	// not virtual

// Helpers
	BOOL	ValidateData();
	friend class CValidPropertyGridCtrl;	// need this for TrackToolTip
};

class CEnumPropertyGridProperty : public CMFCPropertyGridProperty {
public:
	DECLARE_DYNAMIC(CEnumPropertyGridProperty)

// Construction
	CEnumPropertyGridProperty(const CString& strGroupName, DWORD_PTR dwData = 0, BOOL bIsValueList = FALSE) 
		: CMFCPropertyGridProperty(strGroupName, dwData, bIsValueList) { m_iCurSel = -1; }
	CEnumPropertyGridProperty(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0,
		LPCTSTR lpszEditMask = NULL, LPCTSTR lpszEditTemplate = NULL, LPCTSTR lpszValidChars = NULL) 
		: CMFCPropertyGridProperty(strName, varValue, lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars) { m_iCurSel = -1; }

// Data members
	int		m_iCurSel;	// index of current selection, or -1 if none; allows duplicate options

// Overrides
	virtual	void OnSelectCombo();
	virtual void OnClickButton(CPoint point);
	virtual BOOL OnRotateListValue(BOOL bForward);
	virtual BOOL PushChar(UINT nChar);
	virtual BOOL OnDblClk(CPoint point);
};

class CPropertiesGridCtrl : public CValidPropertyGridCtrl {
public:
	DECLARE_DYNAMIC(CPropertiesGridCtrl)

// Construction
	CPropertiesGridCtrl();

// Attributes
	void	GetProperties(CProperties& Props) const;
	void	SetProperties(const CProperties& Props);
	void	GetProperty(CProperties& Props, int iProp) const;
	void	SetProperty(const CProperties& Props, int iProp);
	int		GetCurSelIdx() const;
	void	SetCurSelIdx(int iProp);
	bool	IsEnabled(int iProp) const;
	void	Enable(int iProp, bool bEnable);
	CMFCPropertyGridProperty*	GetValueProperty(int iProp);

// Operations
	void	InitPropList(const CProperties& Props);
	void	UpdateOptions(const CProperties& Props, int iProp);

// Overrideables
	virtual	void	GetCustomValue(int iProp, CComVariant& varProp, CMFCPropertyGridProperty *pProp) const;
	virtual	void	SetCustomValue(int iProp, const CComVariant& varProp, CMFCPropertyGridProperty *pProp);

protected:
// Types
	typedef CArrayEx<CMFCPropertyGridProperty *, CMFCPropertyGridProperty *> CPropertyPtrArray;

// Data members
	CPropertyPtrArray	m_arrProp;	// pointers to value properties

// Helpers
	void	GetProperty(CProperties& Props, int iProp, CMFCPropertyGridProperty *pProp, CComVariant& var) const;
	void	SetProperty(const CProperties& Props, int iProp, CMFCPropertyGridProperty *pProp, CComVariant& var);
};

inline bool CPropertiesGridCtrl::IsEnabled(int iProp) const
{
	return m_arrProp[iProp]->IsEnabled() != 0;
}

inline void CPropertiesGridCtrl::Enable(int iProp, bool bEnable)
{
	m_arrProp[iProp]->Enable(bEnable);
}

inline CMFCPropertyGridProperty* CPropertiesGridCtrl::GetValueProperty(int iProp)
{
	return m_arrProp[iProp];
}
