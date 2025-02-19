// Copyleft 2018 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      18may18	initial version
		01		29sep18	for Insert methods, remove const from val
		02		10nov18	add binary search insert pos
		03		31dec18	refactor to reuse code
		04		02dec19	add sort method
		05		16mar20	add insert sorted descending
		06		26sep20	add find min/max element
		07		18feb22	add index validator

*/

// array classes defined in CArrayEx.h include these methods

CArrayEx_CLASS() 
{
}

CArrayEx_CLASS(const CArrayEx_CLASS& arr)
{
	*this = arr;
}

CArrayEx_CLASS& operator=(const CArrayEx_CLASS& arr)
{
	if (this != &arr)
		Copy(arr);
	return *this;
}

int GetSize() const
{
	return INT64TO32(m_nSize);	// W64: force to 32-bit
}

W64INT GetSize64() const
{
	return m_nSize;
}

W64INT GetMaxSize() const
{
	return m_nMaxSize;
}

W64INT GetGrowBy() const
{
	return m_nGrowBy;
}

void SetGrowBy(INT_PTR nGrowBy)
{
	ASSERT(nGrowBy >= 0);
	m_nGrowBy = nGrowBy;
}

bool IsIndex(int iElem) const
{
	return iElem >= 0 && iElem < m_nSize;
}

void FastRemoveAll()
{
	m_nSize = 0;	// set size without freeing memory
}

CArrayEx_TYPE& GetAt(W64INT nIndex)
{
	ASSERT(nIndex >= 0 && nIndex < m_nSize);
#ifdef CArrayEx_BASE_TYPE
	return reinterpret_cast<CArrayEx_TYPE *>(m_pData)[nIndex];
#else
	return m_pData[nIndex];
#endif
}

const CArrayEx_TYPE& GetAt(W64INT nIndex) const
{
	ASSERT(nIndex >= 0 && nIndex < m_nSize);
#ifdef CArrayEx_BASE_TYPE
	return reinterpret_cast<CArrayEx_TYPE *>(m_pData)[nIndex];
#else
	return m_pData[nIndex];
#endif
}

void SetAt(W64INT nIndex, CArrayEx_TYPE newElement)
{
	ASSERT(nIndex >= 0 && nIndex < m_nSize);
	m_pData[nIndex] = newElement; 
}

CArrayEx_TYPE& ElementAt(W64INT nIndex)
{
	ASSERT(nIndex >= 0 && nIndex < m_nSize);
#ifdef CArrayEx_BASE_TYPE
	return reinterpret_cast<CArrayEx_TYPE *>(m_pData)[nIndex];
#else
	return m_pData[nIndex];
#endif
}

const CArrayEx_TYPE& ElementAt(W64INT nIndex) const
{
	ASSERT(nIndex >= 0 && nIndex < m_nSize);
#ifdef CArrayEx_BASE_TYPE
	return reinterpret_cast<CArrayEx_TYPE *>(m_pData)[nIndex];
#else
	return m_pData[nIndex];
#endif
}

CArrayEx_TYPE& operator[](W64INT nIndex)
{
	return ElementAt(nIndex);
}

const CArrayEx_TYPE& operator[](W64INT nIndex) const
{
	return ElementAt(nIndex);	// base class uses GetAt which is too slow
}

#ifdef CArrayEx_BASE_TYPE
const CArrayEx_TYPE *GetData() const
{
	return(reinterpret_cast<CArrayEx_TYPE *>(m_pData));
}

CArrayEx_TYPE  *GetData()
{
	return(reinterpret_cast<CArrayEx_TYPE *>(m_pData));
}
#endif	// CArrayEx_BASE_TYPE

bool operator==(const CArrayEx_CLASS& arr) const 
{
	if (arr.m_nSize != m_nSize)
		return(FALSE);
	W64INT nElems = m_nSize;
	for (int iElem = 0; iElem < nElems; iElem++) {
		if (arr[iElem] != GetAt(iElem))
			return(FALSE);
	}
	return(TRUE);
}

bool operator!=(const CArrayEx_CLASS& arr) const 
{
	return !operator==(arr); 
}

template<typename T> void Swap(T& a, T& b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

void Swap(CArrayEx_CLASS& src)
{
	Swap(m_pData, src.m_pData);
	Swap(m_nSize, src.m_nSize);
	Swap(m_nMaxSize, src.m_nMaxSize);
}

void Detach(CArrayEx_TYPE*& pData, W64INT& nSize)
{
	ASSERT_VALID(this);
#ifdef CArrayEx_BASE_TYPE
	pData = reinterpret_cast<CArrayEx_TYPE*>(m_pData);
#else
	pData = m_pData;
#endif
	nSize = m_nSize;
	m_pData = NULL;
	m_nSize = 0;
	m_nMaxSize = 0;
	m_nGrowBy = -1;
}

void Attach(CArrayEx_TYPE *pData, W64INT nSize)
{
	ASSERT_VALID(this);
	RemoveAll();
#ifdef CArrayEx_BASE_TYPE
	m_pData = reinterpret_cast<CArrayEx_BASE_TYPE*>(pData);
#else
	m_pData = pData;
#endif
	m_nSize = nSize;
	m_nMaxSize = nSize;
}

#ifndef CArrayEx_NO_FAST_SET_SIZE
void FastSetSize(INT_PTR nNewSize, INT_PTR nGrowBy = -1)
{
	if (nNewSize <= m_nMaxSize)	// if new size fits in allocated memory
		m_nSize = nNewSize;	// set size without zeroing or freeing memory
	else	// set size the usual way
		SetSize(nNewSize, nGrowBy);
}
#endif	// CArrayEx_NO_FAST_SET_SIZE

W64INT Find(const CArrayEx_TYPE& val) const 
{
	W64INT nElems = m_nSize;
	for (W64INT iElem = 0; iElem < nElems; iElem++) {
		if (GetAt(iElem) == val)
			return iElem;
	}
	return -1;
}

W64INT BinarySearch(const CArrayEx_TYPE& val) const 
{
	W64INT iStart = 0;
	W64INT iEnd = m_nSize - 1;
	while (iStart <= iEnd) {
		W64INT iMid = (iStart + iEnd) / 2;
		if (val == GetAt(iMid))
			return iMid;
		else if (val < GetAt(iMid))
			iEnd = iMid - 1;
		else
			iStart = iMid + 1;
	}
	return -1;
}

W64INT BinarySearchAbove(const CArrayEx_TYPE& val) const 
{
	W64INT iStart = 0;
	W64INT iEnd = m_nSize - 1;
	W64INT iResult = -1;
	while (iStart <= iEnd) {
		W64INT iMid = (iStart + iEnd) / 2;
		if (GetAt(iMid) <= val)
			iStart = iMid + 1;
		else {
			iResult = iMid;
			iEnd = iMid - 1;
		}
	}
	return iResult;
}

void InsertSorted(CArrayEx_TYPE& val) 
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
	InsertAt(iInsert, val);
}

void InsertSortedDescending(CArrayEx_TYPE& val) 
{
	W64INT	iInsert = 0;
	W64INT	nSize = m_nSize;
	if (nSize) {	// optimize initial insertion
		W64INT	iStart = 0;
		W64INT	iEnd = nSize - 1;
		if (val >= GetAt(0))	// optimize insertion at start
			iInsert = 0;
		else if (val <= GetAt(iEnd))	// optimize insertion at end
			iInsert = nSize;
		else {	// general case
			while (iStart <= iEnd) {
				W64INT	iMid = (iStart + iEnd) / 2;
				if (GetAt(iMid) >= val)
					iStart = iMid + 1;
				else {
					iInsert = iMid;
					iEnd = iMid - 1;
				}
			}
		}
	}
	InsertAt(iInsert, val);
}

W64INT InsertSortedUnique(CArrayEx_TYPE& val) 
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
	InsertAt(iInsert, val);
	return -1;	// success
}

void Reverse(W64INT iStart, W64INT nElems) 
{
	W64INT	nMid = iStart + nElems / 2;
	W64INT	iLast = iStart * 2 + nElems - 1;
	for (W64INT iElem = iStart; iElem < nMid; iElem++)
		Swap(GetAt(iElem), GetAt(iLast - iElem));
}

void Reverse() 
{
	Reverse(0, m_nSize);
}

void Rotate(W64INT iStart, W64INT nElems, W64INT nOffset) 
{
	CArrayEx_CLASS	arrSrc(*this);
	for (W64INT iElem = 0; iElem < nElems; iElem++) {
		W64INT	iRot = (iElem + nOffset) % nElems;
		if (iRot < 0)
			iRot += nElems;
		GetAt(iStart + iRot) = arrSrc[iStart + iElem];
	}
}

void Rotate(W64INT nOffset) 
{
	Rotate(0, m_nSize, nOffset); 
}

void Shift(W64INT iStart, W64INT nElems, W64INT nOffset, CArrayEx_TYPE& val) 
{
	W64INT	iEnd = iStart + nElems;
	if (nOffset < 0) {	// if shifting down
		W64INT	iSrc = iStart - nOffset;
		for (W64INT iElem = iStart; iElem < iEnd; iElem++) {
			if (iSrc < iEnd)
				GetAt(iElem) = GetAt(iSrc);
			else
				GetAt(iElem) = val;
			iSrc++;
		}
	} else {	// shifting up
		W64INT	iSrc = iEnd - nOffset;
		for (W64INT iElem = iEnd - 1; iElem >= iStart; iElem--) {
			iSrc--;
			if (iSrc >= iStart)
				GetAt(iElem) = GetAt(iSrc);
			else
				GetAt(iElem) = val;
		}
	}
}

void Shift(W64INT nOffset, CArrayEx_TYPE& val) 
{
	Shift(0, m_nSize, nOffset, val); 
}

static int SortCompareAscending(const void *elem1, const void *elem2)
{
	const CArrayEx_TYPE	*a = (const CArrayEx_TYPE *)elem1;
	const CArrayEx_TYPE	*b = (const CArrayEx_TYPE *)elem2;
	return(*a < *b ? -1 : (*a > *b ? 1 : 0));
}

static int SortCompareDescending(const void *elem1, const void *elem2)
{
	const CArrayEx_TYPE	*a = (const CArrayEx_TYPE *)elem1;
	const CArrayEx_TYPE	*b = (const CArrayEx_TYPE *)elem2;
	return(*a < *b ? 1 : (*a > *b ? -1 : 0));
}

void Sort(bool bDescending = FALSE)
{
	qsort(m_pData, m_nSize, sizeof(CArrayEx_TYPE), 
		bDescending ? SortCompareDescending : SortCompareAscending);
}

INT_PTR FindMin() const
{
	if (!m_nSize)
		return -1;
	INT_PTR	iMin = 0;
	for (INT_PTR iElem = 1; iElem < m_nSize; iElem++) {
		if (GetAt(iElem) < GetAt(iMin))
			iMin = iElem;
	}
	return iMin;
}

INT_PTR FindMax() const
{
	if (!m_nSize)
		return -1;
	INT_PTR	iMax = 0;
	for (INT_PTR iElem = 1; iElem < m_nSize; iElem++) {
		if (GetAt(iElem) > GetAt(iMax))
			iMax = iElem;
	}
	return iMax;
}

bool IsAscending() const
{
	for (INT_PTR iElem = 0; iElem < m_nSize - 1; iElem++) {
		if (GetAt(iElem + 1) < GetAt(iElem))	// if next element is less
			return false;
	}
	return true;
}

bool IsDescending() const
{
	for (INT_PTR iElem = 0; iElem < m_nSize - 1; iElem++) {
		if (GetAt(iElem + 1) > GetAt(iElem))	// if next element is greater
			return false;
	}
	return true;
}

#undef CArrayEx_TYPE
#undef CArrayEx_BASE_TYPE
#undef CArrayEx_CLASS
#undef CArrayEx_NO_FAST_SET_SIZE
