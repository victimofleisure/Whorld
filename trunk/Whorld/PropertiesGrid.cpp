// Copyleft 2017 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      23mar17	initial version
		01		10oct17	enable spin control for integer numeric properties
		02		03nov17	add property subgroup
		03		15jan18	in InitPropList, add error checking
		04		15jan18	in InitPropList, handle multi-word subgroup names
		05		10apr18	add runtime class macros
		06		10apr18	in InitPropList's file case, handle ANSI string case
		07		16apr18	in ValidateItemData, use dynamic downcast
		08		18apr18	add UpdateOptions
		09		17may18	add enum property to allow duplicate options
		10		17may18	add spin control for floating point values
		11		04jun18	add time property type
		12		05jun18	in TrackToolTip, fix cast to derived property
		13		26sep18	in enum property, overload PushChar and OnDblClk
		14		26sep18	in PreTranslateMessage, add Alt+Down to drop list
		15		07jun21	rename rounding functions
		16		19aug21	use custom time span conversion methods
		17		26feb23	add folder property type

*/

#include "stdafx.h"
#include "resource.h"
#include "PropertiesGrid.h"
#include "Properties.h"
#include <math.h>

IMPLEMENT_DYNAMIC(CValidPropertyGridCtrl, CMFCPropertyGridCtrl)

// private constants copied from base class
#define AFX_PROP_HAS_SPIN 0x0004
#define AFX_UM_UPDATESPIN (WM_USER + 101)

CValidPropertyGridCtrl::CValidPropertyGridCtrl()
{
	m_bInPreTranslateMsg = false;
	m_bIsDataValidated = false;
}

BOOL CValidPropertyGridCtrl::ValidateItemData(CMFCPropertyGridProperty* pProp)
{
	ASSERT_VALID(pProp);
	if (pProp->IsKindOf(RUNTIME_CLASS(CMFCPropertyGridColorProperty)))	// if color property
		return true;	// special case, ignore it
	CValidPropertyGridProperty	*pMyProp = DYNAMIC_DOWNCAST(CValidPropertyGridProperty, pProp);
	if (pMyProp != NULL) {	// if property supports validation
		m_bIsDataValidated = true;
		return pMyProp->ValidateData();
	} else
		return true;
}

BOOL CValidPropertyGridCtrl::EndEditItem(BOOL bUpdateData)
{
	if (m_bInPreTranslateMsg && m_bIsDataValidated)
		return false;	// prevent duplicate message boxes
	return CMFCPropertyGridCtrl::EndEditItem(bUpdateData);
}

BOOL CValidPropertyGridCtrl::PreTranslateMessage(MSG* pMsg)
{
	m_bInPreTranslateMsg = true;
	m_bIsDataValidated = false;
	switch (pMsg->message) {
	case WM_MOUSEMOVE:
		{
			pMsg->wParam = 0x10000;	// prevent base class from calling buggy TrackToolTip
			CPoint ptCursor;
			::GetCursorPos(&ptCursor);
			ScreenToClient(&ptCursor);
			TrackToolTip(ptCursor);	// call patched TrackToolTip instead
		}
		break;
	case WM_SYSKEYDOWN:
		if (pMsg->wParam == VK_DOWN) {	// Alt+Down to drop list is standard
			CMFCPropertyGridProperty	*pProp = GetCurSel();
			if (pProp != NULL && pProp->GetOptionCount() > 0) {
				pProp->OnClickButton(NULL);	// drop options list
			}
		}
		break;
	}
	BOOL	bResult = CMFCPropertyGridCtrl::PreTranslateMessage(pMsg);
	m_bInPreTranslateMsg = false;
	return bResult;
}

// same as base class but patched to fix a bug: left-clicking a truncated property name 
// saturates the message loop with set/release capture messages, consuming all idle time
#define AFX_STRETCH_DELTA 2
void CValidPropertyGridCtrl::TrackToolTip(CPoint point)
{
	if (m_bTracking || m_bTrackingDescr)
	{
		return;
	}

	CPoint ptScreen = point;
	ClientToScreen(&ptScreen);

	CRect rectTT;
//	m_IPToolTip.GetWindowRect(&rectTT);	// root cause of bug; should be GetLastRect
	m_IPToolTip.GetLastRect(rectTT);	// tip control resets last rect on deactivation

	if (rectTT.PtInRect(ptScreen) && m_IPToolTip.IsWindowVisible())
	{
		return;
	}

/*	if (!m_IPToolTip.IsWindowVisible())
	{
		rectTT.SetRectEmpty();	// no need, tip control takes care of this
	}*/

	if (::GetCapture() == GetSafeHwnd())
	{
		ReleaseCapture();
	}

	// Because our TrackToolTip isn't a member of CMFCPropertyGridCtrl, it doesn't gain the necessary
	// access to CMFCPropertyGridProperty's protected members via friendship. The workaround is, first
	// dynamically downcast to CMFCPropertyGridProperty, and then recast to our derived property, which
	// does grant access. It's safe so long as TrackToolTip only accesses CMFCPropertyGridProperty members.
	CMFCPropertyGridProperty* pMFCProp = DYNAMIC_DOWNCAST(CMFCPropertyGridProperty, HitTest(point));
	CValidPropertyGridProperty*	pProp = static_cast<CValidPropertyGridProperty*>(pMFCProp);
	if (pProp == NULL)
	{
		m_IPToolTip.Deactivate();
		return;
	}

	if (abs(point.x -(m_rectList.left + m_nLeftColumnWidth)) <= AFX_STRETCH_DELTA)
	{
		m_IPToolTip.Deactivate();
		return;
	}

	ASSERT_VALID(pProp);

	if (pProp->IsInPlaceEditing())
	{
		return;
	}

	CString strTipText;
	CRect rectToolTip = pProp->m_Rect;

	BOOL bIsValueTT = FALSE;

	if (point.x < m_rectList.left + m_nLeftColumnWidth)
	{
		if (pProp->IsGroup())
		{
			rectToolTip.left += m_nRowHeight;

			if (point.x <= rectToolTip.left)
			{
				m_IPToolTip.Deactivate();
				return;
			}
		}

		if (pProp->m_bNameIsTruncated)
		{
			if (!m_bAlwaysShowUserTT || pProp->GetNameTooltip().IsEmpty())
			{
				strTipText = pProp->m_strName;
			}
		}
	}
	else
	{
		if (pProp->m_bValueIsTruncated)
		{
			if (!m_bAlwaysShowUserTT || pProp->GetValueTooltip().IsEmpty())
			{
				strTipText = pProp->FormatProperty();
			}
		}

		rectToolTip.left = m_rectList.left + m_nLeftColumnWidth + 1;
		bIsValueTT = TRUE;
	}

	if (!strTipText.IsEmpty())
	{
		ClientToScreen(&rectToolTip);

		if (rectTT.TopLeft() == rectToolTip.TopLeft())
		{
			// Tooltip on the same place, don't show it to prevent flashing
			return;
		}

		m_IPToolTip.SetTextMargin(AFX_TEXT_MARGIN);

		m_IPToolTip.SetFont(bIsValueTT && pProp->IsModified() && m_bMarkModifiedProperties ? &m_fontBold : GetFont());

		m_IPToolTip.Track(rectToolTip, strTipText);
		SetCapture();
	}
	else
	{
		m_IPToolTip.Deactivate();
	}
}

void CValidPropertyGridCtrl::SaveSubitemExpansion(CString sRegKey, const CMFCPropertyGridProperty *pProp) const
{
	CWinApp	*pApp = AfxGetApp();
	int	nSubitems = pProp->GetSubItemsCount();
	for (int iSubitem = 0; iSubitem < nSubitems; iSubitem++) {	// for each subitem
		const CMFCPropertyGridProperty	*pSubitem = pProp->GetSubItem(iSubitem);
		if (pSubitem->IsGroup()) {	// if subitem is a group
			CString	sKey(CString(sRegKey) + '\\' + pProp->GetName());
			VERIFY(pApp->WriteProfileInt(sKey, pSubitem->GetName(), pSubitem->IsExpanded()));
			SaveSubitemExpansion(sKey, pSubitem);	// recurse into subitem
		}
	}
}

void CValidPropertyGridCtrl::RestoreSubitemExpansion(CString sRegKey, CMFCPropertyGridProperty *pProp) const
{
	CWinApp	*pApp = AfxGetApp();
	int	nSubitems = pProp->GetSubItemsCount();
	for (int iSubitem = 0; iSubitem < nSubitems; iSubitem++) {	// for each subitem
		CMFCPropertyGridProperty	*pSubitem = pProp->GetSubItem(iSubitem);
		if (pSubitem->IsGroup()) {	// if subitem is a group
			CString	sKey(CString(sRegKey) + '\\' + pProp->GetName());
			pSubitem->Expand(pApp->GetProfileInt(sKey, pSubitem->GetName(), TRUE));
			RestoreSubitemExpansion(sKey, pSubitem);	// recurse into subitem
		}
	}
}

void CValidPropertyGridCtrl::SaveGroupExpansion(LPCTSTR pszRegKey) const
{
	CWinApp	*pApp = AfxGetApp();
	POSITION	pos = m_lstProps.GetHeadPosition();
	while (pos != NULL) {	// for each top-level property
		const CMFCPropertyGridProperty	*pProp = m_lstProps.GetNext(pos);
		if (pProp->IsGroup()) {	// if property is a group
			VERIFY(pApp->WriteProfileInt(pszRegKey, pProp->GetName(), pProp->IsExpanded()));
			SaveSubitemExpansion(pszRegKey, pProp);	// recursively save subitem expansion
		}
	}
}

void CValidPropertyGridCtrl::RestoreGroupExpansion(LPCTSTR pszRegKey) const
{
	CWinApp	*pApp = AfxGetApp();
	POSITION	pos = m_lstProps.GetHeadPosition();
	while (pos != NULL) {	// for each top-level property
		CMFCPropertyGridProperty	*pProp = m_lstProps.GetNext(pos);
		if (pProp->IsGroup()) {	// if property is a group
			pProp->Expand(pApp->GetProfileInt(pszRegKey, pProp->GetName(), TRUE));
			RestoreSubitemExpansion(pszRegKey, pProp);	// recursively restore subitem expansion
		}
	}
}

int CValidPropertyGridCtrl::GetActualDescriptionRows() const
{
	if (m_nDescrHeight >= 0 && m_nRowHeight > 0)	// if valid heights
		return m_nDescrHeight / m_nRowHeight;
	return m_nDescrRows;
}

BEGIN_MESSAGE_MAP(CValidPropertyGridCtrl, CMFCPropertyGridCtrl)
	ON_NOTIFY(UDN_DELTAPOS, AFX_PROPLIST_ID_INPLACE, OnSpinDeltaPos)
END_MESSAGE_MAP()

void CValidPropertyGridCtrl::OnSpinDeltaPos(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN	pUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	CMFCPropertyGridProperty* pProp = GetCurSel();	// get selected property
	ASSERT(pProp != NULL);	// a property should be selected
	if (pProp != NULL) {	// but check pointer anyway just in case
		CComVariant	var = pProp->GetValue();	// get property's value
		if (var.vt == VT_R4 || var.vt == VT_R8) {	// if value type is float or double
			// The spin button has UDS_SETBUDDYINT style, and if the buddy window's string
			// value has a decimal point, this message sets the value to one regardless of
			// its current value. So we detect this case, round the value up or down to an
			// integer, and set pResult non-zero to prevent the default position change.
			double	fVal;
			if (var.vt == VT_R4)	// if float
				fVal = var.fltVal;
			else	// double
				fVal = var.dblVal;
			CString	sVal;
			sVal.Format(_T("%g"), fVal);	// same format our FormatProperty uses
			if (sVal.Find('.') >= 0) {	// if string value has a decimal point
				if (pUpDown->iDelta > 0)	// if incrementing
					fVal = ceil(fVal);	// round up to integer
				else	// decrementing
					fVal = floor(fVal);	// round down to integer
				if (var.vt == VT_R4)	// if float
					var.fltVal = static_cast<float>(fVal);	// don't change type
				else	// double
					var.dblVal = fVal;
				pProp->SetValue(var);	// update property's value
				*pResult = 1;	// prevent default position change
				return;	// don't post update spin message either
			}
		}
	}
	// default behavior, copied from base class
	*pResult = 0;
	PostMessage(AFX_UM_UPDATESPIN);
}

IMPLEMENT_DYNAMIC(CValidPropertyGridProperty, CMFCPropertyGridProperty)

BOOL CValidPropertyGridProperty::ValidateData()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT_VALID(m_pWndList);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));
	CString strText;
	m_pWndInPlace->GetWindowText(strText);
	CWnd	*pParent = m_pWndInPlace->GetParent();
	ASSERT_VALID(pParent);
	int	nID = m_pWndInPlace->GetDlgCtrlID();
	COleVariant	dst;
	BOOL	bResult = FALSE;
	bool	bRange = m_vMinVal.vt && m_vMaxVal.vt && m_vMinVal != m_vMaxVal;
	TRY {
		CDataExchange	dx(pParent, TRUE);
		switch (m_varValue.vt) {
		case VT_BSTR:
			break;
		case VT_UI1:
			DDX_Text(&dx, nID, dst.bVal);
			if (bRange)
				DDV_MinMaxByte(&dx, dst.bVal, m_vMinVal.bVal, m_vMaxVal.bVal);
			break;
		case VT_I2:
			DDX_Text(&dx, nID, dst.iVal);
			if (bRange)
				DDV_MinMaxShort(&dx, dst.iVal, m_vMinVal.iVal, m_vMaxVal.iVal);
			break;
		case VT_I4:
		case VT_INT:
			DDX_Text(&dx, nID, dst.intVal);
			if (bRange)
				DDV_MinMaxInt(&dx, dst.intVal, m_vMinVal.intVal, m_vMaxVal.intVal);
			break;
		case VT_UI4:
		case VT_UINT:
			DDX_Text(&dx, nID, dst.uintVal);
			if (bRange)
				DDV_MinMaxUInt(&dx, dst.uintVal, m_vMinVal.uintVal, m_vMaxVal.uintVal);
			break;
		case VT_R4:
			DDX_Text(&dx, nID, dst.fltVal);
			if (bRange)
				DDV_MinMaxFloat(&dx, dst.fltVal, m_vMinVal.fltVal, m_vMaxVal.fltVal);
			break;
		case VT_R8:
			DDX_Text(&dx, nID, dst.dblVal);
			if (bRange)
				DDV_MinMaxDouble(&dx, dst.dblVal, m_vMinVal.dblVal, m_vMaxVal.dblVal);
			break;
		case VT_BOOL:
			break;
		default:
			ASSERT(0);
		}
		bResult = TRUE;
	}
	CATCH (CUserException, e) {
	}
	END_CATCH;
	return bResult;
}

CString CValidPropertyGridProperty::FormatProperty()
{
	COleVariant& var = m_varValue;
	CString strVal;
	if (!m_bIsValueList) {
		switch (var.vt) {
		case VT_R4:
			strVal.Format(_T("%g"), (float)var.fltVal);
			return strVal;
		case VT_R8:
			strVal.Format(_T("%g"), (double)var.dblVal);
			return strVal;
		}
	}
	return CMFCPropertyGridProperty::FormatProperty();
}

void CValidPropertyGridProperty::EnableSpinControl(BOOL bEnable, int nMin, int nMax)
{
	// copied from base class
	ASSERT_VALID(this);

	switch (m_varValue.vt)
	{
	case VT_INT:
	case VT_UINT:
	case VT_I2:
	case VT_I4:
	case VT_UI2:
	case VT_UI4:
	case VT_R4:	// allow reals
	case VT_R8:
		break;

	default:
		ASSERT(FALSE);
		return;
	}

	m_nMinValue = nMin;
	m_nMaxValue = nMax;

	if (bEnable)
	{
		m_dwFlags |= AFX_PROP_HAS_SPIN;
	}
	else
	{
		m_dwFlags &= ~AFX_PROP_HAS_SPIN;
	}
}

void CValidPropertyGridProperty::GetValueRange(int& nMinVal, int& nMaxVal) const
{
	switch (m_vMinVal.vt) {
	case VT_INT:
	case VT_UINT:
	case VT_I4:
	case VT_UI4:
		nMinVal = m_vMinVal.intVal;
		nMaxVal = m_vMaxVal.intVal;
		break;
	case VT_I2:
	case VT_UI2:
		nMinVal = m_vMinVal.iVal;
		nMaxVal = m_vMaxVal.iVal;
		break;
	case VT_R4:
		nMinVal = Round(m_vMinVal.fltVal);
		nMaxVal = Round(m_vMaxVal.fltVal);
		break;
	case VT_R8:
		nMinVal = Round(m_vMinVal.dblVal);
		nMaxVal = Round(m_vMaxVal.dblVal);
		break;
	default:
		nMinVal = 0;
		nMaxVal = 0;
	}
}

IMPLEMENT_DYNAMIC(CEnumPropertyGridProperty, CMFCPropertyGridProperty)

void CEnumPropertyGridProperty::OnSelectCombo()
{
	// copied from base class
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndCombo);
	ASSERT_VALID(m_pWndInPlace);

	int iSelIndex = m_pWndCombo->GetCurSel();
	if (iSelIndex >= 0)
	{
		m_iCurSel = iSelIndex;	// save selection index before updating value
		CString str;
		m_pWndCombo->GetLBText(iSelIndex, str);
		m_pWndInPlace->SetWindowText(str);
		OnUpdateValue();
	}
}

void CEnumPropertyGridProperty::OnClickButton(CPoint point)
{
	UNREFERENCED_PARAMETER(point);
	// copied from base class
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);

	if (m_pWndCombo != NULL)
	{
		m_bButtonIsDown = TRUE;
		Redraw();

		CString str;
		m_pWndInPlace->GetWindowText(str);

		m_pWndCombo->SetCurSel(m_iCurSel);	// use selection index

		m_pWndCombo->SetFocus();
		m_pWndCombo->ShowDropDown();
	}
}

BOOL CEnumPropertyGridProperty::OnRotateListValue(BOOL bForward)
{
	// emulate base class but using current selection index
	if (m_pWndInPlace == NULL)
		return FALSE;
	INT_PTR	nOptions = m_lstOptions.GetCount();
	if (bForward) {	// if incrementing
		m_iCurSel++;
		if (m_iCurSel >= nOptions)
			m_iCurSel = 0;
	} else {	// decrementing
		m_iCurSel--;
		if (m_iCurSel < 0)
			m_iCurSel = INT64TO32(nOptions - 1);
	}
	CString	strText = GetOption(m_iCurSel);
	m_pWndInPlace->SetWindowText(strText);
	OnUpdateValue();
	CEdit* pEdit = DYNAMIC_DOWNCAST(CEdit, m_pWndInPlace);
	if (::IsWindow(pEdit->GetSafeHwnd()))
		pEdit->SetSel(0, -1);
	return TRUE;
}

BOOL CEnumPropertyGridProperty::PushChar(UINT nChar)
{
	// adapted from base class
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT(!m_bAllowEdit);	// only drop list is supported
	if (!m_bAllowEdit)
	{
		if (nChar == VK_SPACE)
		{
			OnRotateListValue(true);
		}
		else if (m_lstOptions.GetCount() > 1)
		{
			CString strText;
			m_pWndInPlace->GetWindowText(strText);
			POSITION pos = m_lstOptions.Find(strText);
			if (pos == NULL)
			{
				return FALSE;
			}
			POSITION posSave = pos;
			CString strChar((TCHAR) nChar);
			strChar.MakeUpper();
			m_lstOptions.GetNext(pos);
			while (pos != posSave)
			{
				if (pos == NULL)
				{
					pos = m_lstOptions.GetHeadPosition();
				}
				if (pos == posSave)
				{
					break;
				}
				strText = m_lstOptions.GetAt(pos);
				CString strUpper = strText;
				strUpper.MakeUpper();
				if (strUpper.Left(1) == strChar)
				{
					INT_PTR	nOptions = m_lstOptions.GetCount();
					for (int iOpt = 0; iOpt < nOptions; iOpt++) {
						if (strText == GetOption(iOpt)) {
							m_iCurSel = iOpt;
							break;
						}
					}
					m_pWndInPlace->SetWindowText(strText);
					OnUpdateValue();
					break;
				}
				m_lstOptions.GetNext(pos);
			}
		}
	}
	OnEndEdit();
	if (::GetCapture() == m_pWndList->GetSafeHwnd())
	{
		ReleaseCapture();
	}
	return FALSE;
}

BOOL CEnumPropertyGridProperty::OnDblClk(CPoint point)
{
	UNREFERENCED_PARAMETER(point);
	return OnRotateListValue(true);
}

IMPLEMENT_DYNAMIC(CPropertiesGridCtrl, CValidPropertyGridCtrl)

CPropertiesGridCtrl::CPropertiesGridCtrl()
{
}

void CPropertiesGridCtrl::GetProperties(CProperties& Props) const
{
	int	nProps = Props.GetPropertyCount();
	CVariantArray	arrVar;
	arrVar.SetSize(nProps);
	for (int iProp = 0; iProp < nProps; iProp++) {	// for each property
		GetProperty(Props, iProp, m_arrProp[iProp], arrVar[iProp]);
	}
	Props.SetVariants(arrVar);
}

void CPropertiesGridCtrl::SetProperties(const CProperties& Props)
{
	CVariantArray	arrVar;
	Props.GetVariants(arrVar);
	int	nProps = Props.GetPropertyCount();
	for (int iProp = 0; iProp < nProps; iProp++) {	// for each property
		SetProperty(Props, iProp, m_arrProp[iProp], arrVar[iProp]);
	}
}

void CPropertiesGridCtrl::GetProperty(CProperties& Props, int iProp) const
{
	CComVariant	var;
	GetProperty(Props, iProp, m_arrProp[iProp], var);
	Props.SetProperty(iProp, var);
}

void CPropertiesGridCtrl::SetProperty(const CProperties& Props, int iProp)
{
	CComVariant	var;
	Props.GetProperty(iProp, var);
	SetProperty(Props, iProp, m_arrProp[iProp], var);
}

void CPropertiesGridCtrl::GetCustomValue(int iProp, CComVariant& varProp, CMFCPropertyGridProperty *pProp) const
{
	UNREFERENCED_PARAMETER(iProp);
	UNREFERENCED_PARAMETER(varProp);
	UNREFERENCED_PARAMETER(pProp);
}

void CPropertiesGridCtrl::SetCustomValue(int iProp, const CComVariant& varProp, CMFCPropertyGridProperty *pProp)
{
	UNREFERENCED_PARAMETER(iProp);
	UNREFERENCED_PARAMETER(varProp);
	UNREFERENCED_PARAMETER(pProp);
}

void CPropertiesGridCtrl::GetProperty(CProperties& Props, int iProp, CMFCPropertyGridProperty *pProp, CComVariant& var) const
{
	switch (Props.GetPropertyType(iProp)) {
	case CProperties::PT_ENUM:
		{
			int	iOption = static_cast<const CEnumPropertyGridProperty *>(pProp)->m_iCurSel;
			var = max(iOption, 0);	// avoid negative enum
		}
		break;
	case CProperties::PT_COLOR:
		var.uintVal = static_cast<const CMFCPropertyGridColorProperty *>(pProp)->GetColor();
		break;
	case CProperties::PT_TIME:
		{
			CString	sTime(pProp->GetValue());
			LONGLONG	nTime;
			if (!CProperties::StringToTimeSpan(sTime, nTime))
				AfxMessageBox(IDS_GRID_ERR_BAD_TIME_FORMAT);
			sTime = CProperties::TimeSpanToString(nTime);
			pProp->SetValue(sTime);	// display reformatted time
			var.intVal = static_cast<int>(nTime);
		}
		break;
	case CProperties::PT_CUSTOM:
		GetCustomValue(iProp, var, pProp);
		break;
	default:
		var = pProp->GetValue();
	}
}

void CPropertiesGridCtrl::SetProperty(const CProperties& Props, int iProp, CMFCPropertyGridProperty *pProp, CComVariant& var)
{
	switch (Props.GetPropertyType(iProp)) {
	case CProperties::PT_ENUM:
		{
			int	iOption = var.intVal;
			LPCTSTR	szOption;
			if (iOption >= 0)
				szOption = pProp->GetOption(iOption);
			else
				szOption = NULL;
			pProp->SetValue(szOption);
			static_cast<CEnumPropertyGridProperty *>(pProp)->m_iCurSel = iOption;
		}
		break;
	case CProperties::PT_COLOR:
		static_cast<CMFCPropertyGridColorProperty *>(pProp)->SetColor(var.uintVal);
		break;
	case CProperties::PT_TIME:
		{
			CString	sTime(CProperties::TimeSpanToString(var.intVal));
			pProp->SetValue(sTime);
		}
		break;
	case CProperties::PT_CUSTOM:
		SetCustomValue(iProp, var, pProp);
		break;
	default:
		pProp->SetValue(var);
	}
}

void CPropertiesGridCtrl::InitPropList(const CProperties& Props)
{
	CMFCPropertyGridProperty	*pGroup = NULL;
	CMFCPropertyGridProperty	*pSubgroup = NULL;
	int	iPrevGroup = -1;
	int	iPrevSubgroup = -1;
	CVariantArray	arrVar;
	Props.GetVariants(arrVar);
	int	nProps = arrVar.GetSize();
	m_arrProp.SetSize(nProps);
	for (int iProp = 0; iProp < nProps; iProp++) {	// for each property
		CString	sPropName(Props.GetPropertyName(iProp));
		CString	sPropDescrip(Props.GetPropertyDescription(iProp));
		CMFCPropertyGridProperty	*pParentProp;
		int	iGroup = Props.GetGroup(iProp);
		if (iGroup != iPrevGroup) {	// if group changed
			CString	sGroupName(Props.GetGroupName(iGroup));
			pGroup = new CMFCPropertyGridProperty(sGroupName, DWORD_PTR(-1));
			VERIFY(AddProperty(pGroup) >= 0);	// add group to grid control
			iPrevGroup = iGroup;
		}
		int	iSubgroup = Props.GetSubgroup(iProp);
		if (iSubgroup >= 0) {	// if property belongs to a subgroup
			CString	sSubgroupName(Props.GetSubgroupName(iGroup, iSubgroup));
			if (iSubgroup != iPrevSubgroup) {	// if subgroup changed
				pSubgroup = new CMFCPropertyGridProperty(sSubgroupName, DWORD_PTR(-1));
				VERIFY(pGroup->AddSubItem(pSubgroup));	// add subgroup to its parent group
				iPrevSubgroup = iSubgroup;
			}
			int	iPos = sPropName.Find(sSubgroupName);
			if (!iPos) {	// if property name starts with subgroup name
				int	nLen = sSubgroupName.GetLength();	// if property name's next char is space
				if (sPropName.GetLength() > nLen && sPropName[nLen] == ' ')
					sPropName.Delete(0, nLen + 1);	// delete subgroup name from property name
			}
			pParentProp = pSubgroup;	// parent is subgroup
		} else {	// property doesn't belong to a subgroup
			iPrevSubgroup = -1;	// reset subgroup index
			pParentProp = pGroup;	// parent is group
		}
		CMFCPropertyGridProperty*	pProp;
		switch (Props.GetPropertyType(iProp)) {
		case CProperties::PT_ENUM:
			{
				CEnumPropertyGridProperty	*pEnumProp = 
				pEnumProp = new CEnumPropertyGridProperty(sPropName, _T(""), sPropDescrip, iProp);
				int	nOptions = Props.GetOptionCount(iProp);
				for (int iOption = 0; iOption < nOptions; iOption++)	// for each option
					VERIFY(pEnumProp->AddOption(Props.GetOptionName(iProp, iOption), false));	// allow dups
				pEnumProp->AllowEdit(false);
				pProp = pEnumProp;
			}
			break;
		case CProperties::PT_COLOR:
			{
				CMFCPropertyGridColorProperty	*pColorProp = 
					new CMFCPropertyGridColorProperty(sPropName, arrVar[iProp].uintVal, NULL, sPropDescrip, iProp);
				pColorProp->EnableOtherButton(LDS(IDS_PROPS_MORE_COLORS));
				pProp = pColorProp;
			}
			break;
		case CProperties::PT_FILE:
			{
				CMFCPropertyGridFileProperty	*pFileProp = 
					new CMFCPropertyGridFileProperty(sPropName, TRUE, CString(arrVar[iProp].bstrVal), NULL, 0, NULL, sPropDescrip, iProp);
				pProp = pFileProp;
			}
			break;
		case CProperties::PT_FOLDER:
			{
				CMFCPropertyGridFileProperty	*pFolderProp = 
					new CMFCPropertyGridFileProperty(sPropName, CString(arrVar[iProp].bstrVal), iProp, sPropDescrip);
				pProp = pFolderProp;
			}
			break;
		case CProperties::PT_TIME:
		case CProperties::PT_CUSTOM:
			{
				CValidPropertyGridProperty	*pNumProp = 
					new CValidPropertyGridProperty(sPropName, _T(""), sPropDescrip, iProp);
				pProp = pNumProp;
			}
			break;
		default:
			CValidPropertyGridProperty	*pNumProp;
			pNumProp = new CValidPropertyGridProperty(sPropName, arrVar[iProp], sPropDescrip, iProp);
			Props.GetRange(iProp, pNumProp->m_vMinVal, pNumProp->m_vMaxVal);
			int	nMinVal, nMaxVal;
			pNumProp->GetValueRange(nMinVal, nMaxVal);
			if (nMinVal != nMaxVal)
				pNumProp->EnableSpinControl(TRUE, nMinVal, nMaxVal);
			pProp = pNumProp;
		}
		VERIFY(pParentProp->AddSubItem(pProp));	// add property to its parent property
		m_arrProp[iProp] = pProp;	// store pointer in value property array
	}
}

int CPropertiesGridCtrl::GetCurSelIdx() const
{
	if (m_pSel != NULL)
		return INT64TO32(m_pSel->GetData());
	return -1;
}

void CPropertiesGridCtrl::SetCurSelIdx(int iProp)
{
	CMFCPropertyGridProperty	*pProp;
	if (iProp >= 0)
		pProp = m_arrProp[iProp];
	else
		pProp = NULL;
	SetCurSel(pProp);
}

void CPropertiesGridCtrl::UpdateOptions(const CProperties& Props, int iProp)
{
	EndEditItem();
	CMFCPropertyGridProperty	*pProp = m_arrProp[iProp];
	pProp->RemoveAllOptions();
	int	nOptions = Props.GetOptionCount(iProp);
	for (int iOption = 0; iOption < nOptions; iOption++)	// for each option
		VERIFY(pProp->AddOption(Props.GetOptionName(iProp, iOption), false));	// allow dups
	if (Props.GetPropertyType(iProp) == CProperties::PT_ENUM) {
		CComVariant	var;
		Props.GetProperty(iProp, var);
		int	iOption = var.intVal;
		static_cast<CEnumPropertyGridProperty *>(pProp)->m_iCurSel = iOption;
		CString	sOption(Props.GetOptionName(iProp, iOption));
		pProp->SetValue(sOption);
	}
}
