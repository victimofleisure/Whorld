// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		06jan10	W64: adapt for 64-bit
		02		04mar13	add int and DWORD arrays
		03		09oct13	add BYTE array with attach/detach/swap
		04		21feb14	add GetData to CIntArrayEx
		05		09sep14	add CStringArrayEx
		06		23dec15	in CStringArrayEx, add GetSize
		07		28jun17	add GetAt and SetAt to CArrayEx
		08		15feb18	optimize Swap to eliminate RemoveAll
		09		16feb18	add FastSetSize
		10		19mar18	add FastRemoveAll
		11		22mar18	add Find, BinarySearch, InsertSorted
		12		30apr18	add GetMaxSize
		13		02may18	add equality operators to non-template arrays
		14		03may18	add InsertSortedUnique and refactor
		15		18may18	move list of algorithms to header file
		16		08jun18	add more fast methods to CArrayEx; add SetGrowBy
		17		19jun18	add insert, delete, and move selection
		18		09jul18	add array find template
		19		31jul18	add boolean array
		20		29sep18	in CArrayEx, change ALGO_TYPE to TYPE
		21		10nov18	add binary search insert pos
		22		31dec18	refactor to reuse code
		23		03jan19	add CPtrArrayEx
		24		25feb19	fix missing default count in FastRemoveAt
		25		26feb19	add FastInsertSorted
		26		01apr19	add FastInsertSortedUnique
		27		26sep20	add indirect sort
		28		23feb23	add WORD array

		enhanced array with copy ctor, assignment, and fast const access
 
*/

#pragma once

#include <afxtempl.h>

// CArrayEx Fast* methods do NOT initialize elements to zero unless this is set
#define CArrayEx_FAST_INIT_ZERO 0

class CDWordArrayEx : public CDWordArray
{
public:
	#define	CArrayEx_TYPE DWORD
	#define CArrayEx_CLASS CDWordArrayEx
	#include "ArrayExAlgoDef.h"
};

class CIntArrayEx : public CDWordArray
{
public:
	#define	CArrayEx_TYPE int
	#define CArrayEx_BASE_TYPE DWORD
	#define CArrayEx_CLASS CIntArrayEx
	#include "ArrayExAlgoDef.h"
};

class CWordArrayEx : public CWordArray
{
public:
	#define	CArrayEx_TYPE WORD
	#define CArrayEx_CLASS CWordArrayEx
	#include "ArrayExAlgoDef.h"
};

class CByteArrayEx : public CByteArray
{
public:
	#define	CArrayEx_TYPE BYTE
	#define CArrayEx_CLASS CByteArrayEx
	#include "ArrayExAlgoDef.h"
};

class CBoolArrayEx : public CByteArray
{
public:
	#define	CArrayEx_TYPE bool
	#define CArrayEx_BASE_TYPE BYTE
	#define CArrayEx_CLASS CBoolArrayEx
	#include "ArrayExAlgoDef.h"
};

class CStringArrayEx : public CStringArray
{
public:
	#define	CArrayEx_TYPE CString
	#define CArrayEx_CLASS CStringArrayEx
	#include "ArrayExAlgoDef.h"
};

class CPtrArrayEx : public CPtrArray
{
public:
	#define	CArrayEx_TYPE PVOID
	#define CArrayEx_CLASS CPtrArrayEx
	#include "ArrayExAlgoDef.h"
};

template<class TYPE, class ARG_TYPE>
class CArrayEx : public CArray<TYPE, ARG_TYPE> {
public:
// Operations
	// Fast* methods do NOT call constructors or destructors, and do NOT
	// initialize elements to zero (unless CArrayEx_FAST_INIT_ZERO is set)
	void	FastSetSize(INT_PTR nNewSize, INT_PTR nGrowBy = -1);
	void	FastSetAtGrow(INT_PTR nIndex, ARG_TYPE newElement);
	INT_PTR	FastAdd(ARG_TYPE newElement);
	void	FastInsertAt(INT_PTR nIndex, ARG_TYPE newElement, INT_PTR nCount = 1);
	void	FastRemoveAt(INT_PTR nIndex, INT_PTR nCount = 1);
	void	FastInsertSorted(ARG_TYPE val);
	W64INT	FastInsertSortedUnique(ARG_TYPE val);
	#define	CArrayEx_TYPE TYPE
	#define CArrayEx_CLASS CArrayEx
	#define CArrayEx_NO_FAST_SET_SIZE
	#include "ArrayExAlgoDef.h"
	void	GetRange(int iFirstElem, int nElems, CArrayEx& arrDest) const;
	void	SetRange(int iFirstElem, const CArrayEx& arrDest);
	void	GetSelection(const CIntArrayEx& arrSelection, CArrayEx& arrDest) const;
	void	SetSelection(const CIntArrayEx& arrSelection, const CArrayEx& arrDest);
	void	InsertSelection(const CIntArrayEx& arrSelection, CArrayEx& arrInsert);
	void	DeleteSelection(const CIntArrayEx& arrSelection);
	void	MoveSelection(const CIntArrayEx& arrSelection, int iDropPos);
	void	SortIndirect(int (__cdecl *_PtFuncCompare)(const void *, const void *), CPtrArrayEx *parrSortedPtr = NULL);
};

template<class TYPE, class ARG_TYPE>
AFX_INLINE void CArrayEx<TYPE, ARG_TYPE>::FastSetSize(INT_PTR nNewSize, INT_PTR nGrowBy)
{
	ASSERT_VALID(this);
	ASSERT(nNewSize >= 0);
	if (nNewSize <= m_nMaxSize) {	// if new size fits in allocated memory
		m_nSize = nNewSize;	// set size without zeroing or freeing memory
	} else {	// new size doesn't fit
		if (nGrowBy >= 0)
			m_nGrowBy = nGrowBy;  // set new size
		if (m_pData == NULL) {	// if array is unallocated
			// create buffer big enough to hold number of requested elements or
			// m_nGrowBy elements, whichever is larger.
			size_t nAllocSize = __max(nNewSize, m_nGrowBy);
			m_pData = (TYPE*) new BYTE[(size_t)nAllocSize * sizeof(TYPE)];
#if CArrayEx_FAST_INIT_ZERO
			memset((void*)m_pData, 0, (size_t)nAllocSize * sizeof(TYPE));
#endif
			m_nSize = nNewSize;
			m_nMaxSize = nAllocSize;
		} else {	// otherwise, grow array
			nGrowBy = m_nGrowBy;
			if (nGrowBy == 0) {
				// heuristically determine growth when nGrowBy == 0
				nGrowBy = m_nSize / 8;
				nGrowBy = (nGrowBy < 4) ? 4 : ((nGrowBy > 1024) ? 1024 : nGrowBy);
			}
			INT_PTR nNewMax;
			if (nNewSize < m_nMaxSize + nGrowBy)
				nNewMax = m_nMaxSize + nGrowBy;  // granularity
			else
				nNewMax = nNewSize;  // no slush
			ASSERT(nNewMax >= m_nMaxSize);  // no wrap around		
			TYPE* pNewData = (TYPE*) new BYTE[(size_t)nNewMax * sizeof(TYPE)];
			// copy new data from old
			memcpy(pNewData, m_pData, (size_t)m_nSize * sizeof(TYPE));
			// get rid of old stuff
			delete[] (BYTE*)m_pData;
			m_pData = pNewData;
			m_nSize = nNewSize;
			m_nMaxSize = nNewMax;
		}
	}
}

template<class TYPE, class ARG_TYPE>
void CArrayEx<TYPE, ARG_TYPE>::FastSetAtGrow(INT_PTR nIndex, ARG_TYPE newElement)
{
	ASSERT_VALID(this);
	ASSERT(nIndex >= 0);
	if (nIndex >= m_nSize)
		FastSetSize(nIndex + 1, -1);
	m_pData[nIndex] = newElement;
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE INT_PTR CArrayEx<TYPE, ARG_TYPE>::FastAdd(ARG_TYPE newElement)
{
	INT_PTR nIndex = m_nSize;
	FastSetAtGrow(nIndex, newElement);
	return nIndex;
}

template<class TYPE, class ARG_TYPE>
void CArrayEx<TYPE, ARG_TYPE>::FastInsertAt(INT_PTR nIndex, ARG_TYPE newElement, INT_PTR nCount)
{
	ASSERT_VALID(this);
	ASSERT(nIndex >= 0);    // will expand to meet need
	ASSERT(nCount > 0);     // zero or negative size not allowed
	if (nIndex >= m_nSize) {
		// adding after the end of the array
		FastSetSize(nIndex + nCount, -1);   // grow so nIndex is valid
	} else {
		// inserting in the middle of the array
		INT_PTR nOldSize = m_nSize;
		FastSetSize(m_nSize + nCount, -1);  // grow it to new size
		// shift old data up to fill gap
		memmove(m_pData + nIndex + nCount, m_pData + nIndex, (nOldSize - nIndex) * sizeof(TYPE));
#if CArrayEx_FAST_INIT_ZERO
		// re-init slots we copied from
		memset((void*)(m_pData + nIndex), 0, (size_t)nCount * sizeof(TYPE));
#endif
	}
	// insert new value in the gap
	ASSERT(nIndex + nCount <= m_nSize);
	while (nCount--)
		m_pData[nIndex++] = newElement;
}

template<class TYPE, class ARG_TYPE>
void CArrayEx<TYPE, ARG_TYPE>::FastRemoveAt(INT_PTR nIndex, INT_PTR nCount)
{
	ASSERT_VALID(this);
	ASSERT(nIndex >= 0);
	ASSERT(nCount >= 0);
	INT_PTR nUpperBound = nIndex + nCount;
	ASSERT(nUpperBound <= m_nSize && nUpperBound >= nIndex && nUpperBound >= nCount);
	// just remove a range
	INT_PTR nMoveCount = m_nSize - nUpperBound;
	if (nMoveCount) {
		memmove(m_pData + nIndex, m_pData + nUpperBound, (size_t)nMoveCount * sizeof(TYPE));
	}
	m_nSize -= nCount;
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE void CArrayEx<TYPE, ARG_TYPE>::FastInsertSorted(ARG_TYPE val) 
{
	W64INT	iInsert = 0;
	W64INT	nSize = m_nSize;
	if (nSize) {	// optimize initial insertion
		W64INT	iStart = 0;
		W64INT	iEnd = nSize - 1;
		if (val <= GetAt(0))	// optimize insertion at start
			iInsert = 0;
		else if (val >= GetAt(iEnd))	// optimize insertion at end
			iInsert = nSize;
		else {	// general case
			while (iStart <= iEnd) {
				W64INT	iMid = (iStart + iEnd) / 2;
				if (GetAt(iMid) <= val)
					iStart = iMid + 1;
				else {
					iInsert = iMid;
					iEnd = iMid - 1;
				}
			}
		}
	}
	FastInsertAt(iInsert, val);	// same as InsertSorted except for this line
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE W64INT CArrayEx<TYPE, ARG_TYPE>::FastInsertSortedUnique(ARG_TYPE val) 
{
	W64INT	iInsert = m_nSize;
	W64INT	iStart = 0;
	W64INT	iEnd = iInsert - 1;
	while (iStart <= iEnd) {
		W64INT	iMid = (iStart + iEnd) / 2;
		if (GetAt(iMid) == val)	// if would be duplicate
			return iMid;	// return element index
		if (GetAt(iMid) < val)
			iStart = iMid + 1;
		else {
			iInsert = iMid;
			iEnd = iMid - 1;
		}
	}
	FastInsertAt(iInsert, val);	// same as InsertSortedUnique except for this line
	return -1;	// success
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE void CArrayEx<TYPE, ARG_TYPE>::GetRange(int iFirstElem, int nElems, CArrayEx& arrDest) const
{
	arrDest.SetSize(nElems);
	for (int iElem = 0; iElem < nElems; iElem++)	// for each element in range
		arrDest[iElem] = GetAt(iFirstElem + iElem);
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE void CArrayEx<TYPE, ARG_TYPE>::SetRange(int iFirstElem, const CArrayEx& arrDest)
{
	int	nElems = arrDest.GetSize();
	for (int iElem = 0; iElem < nElems; iElem++)	// for each element in range
		GetAt(iFirstElem + iElem) = arrDest[iElem];
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE void CArrayEx<TYPE, ARG_TYPE>::GetSelection(const CIntArrayEx& arrSelection, CArrayEx& arrDest) const
{
	int	nSels = arrSelection.GetSize();
	arrDest.SetSize(nSels);
	for (int iSel = 0; iSel < nSels; iSel++)	// for each selected item
		arrDest[iSel] = GetAt(arrSelection[iSel]);
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE void CArrayEx<TYPE, ARG_TYPE>::SetSelection(const CIntArrayEx& arrSelection, const CArrayEx& arrDest)
{
	int	nSels = arrSelection.GetSize();
	for (int iSel = 0; iSel < nSels; iSel++)	// for each selected item
		GetAt(arrSelection[iSel]) = arrDest[iSel];
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE void CArrayEx<TYPE, ARG_TYPE>::InsertSelection(const CIntArrayEx& arrSelection, CArrayEx& arrInsert)
{
	// assume selection in ascending order
	int	nSels = arrSelection.GetSize();
	for (int iSel = 0; iSel < nSels; iSel++)	// for each selected item
		InsertAt(arrSelection[iSel], arrInsert[iSel]);
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE void CArrayEx<TYPE, ARG_TYPE>::DeleteSelection(const CIntArrayEx& arrSelection)
{
	// assume selection in ascending order
	int	nSels = arrSelection.GetSize();
	for (int iSel = nSels - 1; iSel >= 0; iSel--)	// reverse iterate for deletion stability
		RemoveAt(arrSelection[iSel]);
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE void CArrayEx<TYPE, ARG_TYPE>::MoveSelection(const CIntArrayEx& arrSelection, int iDropPos)
{
	// assume drop position is already compensated for deletions
	int	nSels = arrSelection.GetSize();
	CArrayEx	arrInsert;
	arrInsert.SetSize(nSels);
	for (int iSel = nSels - 1; iSel >= 0; iSel--) {	// reverse iterate for deletion stability
		int	iItem = arrSelection[iSel];
		arrInsert[iSel] = GetAt(iItem);
		RemoveAt(iItem);
	}
	InsertAt(iDropPos, &arrInsert);
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE void CArrayEx<TYPE, ARG_TYPE>::SortIndirect(int (__cdecl *_PtFuncCompare)(const void *, const void *), CPtrArrayEx *parrSortedPtr)
{
	W64INT	nElems = m_nSize;
	CPtrArrayEx	arrObPtr;
	arrObPtr.SetSize(nElems);
	W64INT	iElem;
	for (iElem = 0; iElem < nElems; iElem++) {	// create pointers to elements
		arrObPtr[iElem] = &GetAt(iElem);
	}
	qsort(arrObPtr.GetData(), nElems, sizeof(PVOID), _PtFuncCompare);	// sort element pointers
	CArrayEx<TYPE, ARG_TYPE>	arrOb;
	arrOb.SetSize(nElems);
	for (iElem = 0; iElem < nElems; iElem++) {	// copy elements in sorted order to temporary array
		arrOb[iElem] = *static_cast<TYPE*>(arrObPtr[iElem]);
	}
	*this = arrOb;	// copy sorted temporary array to this array
	if (parrSortedPtr != NULL)	// if caller wants sorted element pointers
		parrSortedPtr->Swap(arrObPtr);	// move sorted element pointers to caller's array
}

// template to find an element in an array
template<typename T> inline int ArrayFind(const T arr[], int nItems, T target)
{
	for (int iItem = 0; iItem < nItems; iItem++) {
		if (arr[iItem] == target)
			return iItem;
	}
	return -1;
}

// array find template specialized for strings
inline int ArrayFind(const LPCTSTR arr[], int nItems, LPCTSTR target)
{
	for (int iItem = 0; iItem < nItems; iItem++) {
		if (!_tcscmp(arr[iItem], target))
			return iItem;
	}
	return -1;
}

// array find template simplified for static arrays
#define ARRAY_FIND(arr, target) ArrayFind(arr, _countof(arr), target)
