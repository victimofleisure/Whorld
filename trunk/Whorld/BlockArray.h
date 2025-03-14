// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      24feb14	initial version
		01		26feb14	in Serialize, use WriteCount and ReadCount
		02		16aug16	use CPtrArrayEx for faster element access in Release mode
		03		17dec18	add inline prefix, fix RemoveAll
		04		14mar25	add block accessors

		dynamic array that allocates in blocks of a given size
 
*/

#pragma once

#include "ArrayEx.h"

template<class TYPE, int BLOCK_SIZE>
class CBlockArray : public WObject
{
public:
// Construction
	CBlockArray();
	~CBlockArray();

// Attributes
	INT_PTR	GetSize() const;
	void	SetSize(INT_PTR nSize);
	TYPE&	operator[](INT_PTR iIndex) const;
	INT_PTR	GetBlockCount() const;
	TYPE*	GetBlock(INT_PTR Block) const;
	INT_PTR	GetBlockSize(INT_PTR iBlock) const;

// Operations
	INT_PTR	Add(const TYPE &newElement);
	void	RemoveAll();
	void	Serialize(CArchive& ar);

protected:
// Types
	class CBlock : public WObject {
	public:
		TYPE	m_arrData[BLOCK_SIZE];	// block of elements
	};

// Data members
	CTypedPtrArray<CPtrArrayEx, CBlock*> m_arrBlock;	// array of pointers to blocks
	INT_PTR	m_nSize;	// total number of elements in array
};

template<class TYPE, int BLOCK_SIZE>
AFX_INLINE CBlockArray<TYPE, BLOCK_SIZE>::CBlockArray()
{
	m_nSize = 0;
}

template<class TYPE, int BLOCK_SIZE>
AFX_INLINE CBlockArray<TYPE, BLOCK_SIZE>::~CBlockArray()
{
	RemoveAll();
}

template<class TYPE, int BLOCK_SIZE>
AFX_INLINE INT_PTR CBlockArray<TYPE, BLOCK_SIZE>::GetSize() const
{
	return(m_nSize);
}

template<class TYPE, int BLOCK_SIZE>
AFX_INLINE void CBlockArray<TYPE, BLOCK_SIZE>::SetSize(INT_PTR nSize)
{
	INT_PTR	nNewBlocks = (nSize - 1) / BLOCK_SIZE + 1;
	INT_PTR	nBlocks = m_arrBlock.GetSize();
	if (nNewBlocks > nBlocks) {	// if growing block array
		m_arrBlock.SetSize(nNewBlocks);	// increase block array size
		for (INT_PTR iBlock = nBlocks; iBlock < nNewBlocks; iBlock++)	// for each new block
			m_arrBlock[iBlock] = new CBlock;	// allocate block
	} else if (nNewBlocks < nBlocks) {// if shrinking block array
		for (INT_PTR iBlock = nNewBlocks; iBlock < nBlocks; iBlock++)	// for each surplus block
			delete m_arrBlock[iBlock];	// delete block
		m_arrBlock.SetSize(nNewBlocks);	// reduce block array size
	}
	m_nSize = nSize;
}

template<class TYPE, int BLOCK_SIZE>
AFX_INLINE TYPE& CBlockArray<TYPE, BLOCK_SIZE>::operator[](INT_PTR iIndex) const
{
	ASSERT(iIndex >= 0 && iIndex < m_nSize);	// bounds check
	UINT_PTR	uIndex = static_cast<UINT_PTR>(iIndex); // unsigned optimizes better if block size is power of two
	UINT_PTR	iBlock = uIndex / BLOCK_SIZE;	// index of block
	UINT_PTR	iElement = uIndex % BLOCK_SIZE;	// index of element within block
	return m_arrBlock[iBlock]->m_arrData[iElement];
}

template<class TYPE, int BLOCK_SIZE>
AFX_INLINE INT_PTR CBlockArray<TYPE, BLOCK_SIZE>::Add(const TYPE& newElement)
{
	if (!(m_nSize % BLOCK_SIZE))		// if last block is full
		m_arrBlock.Add(new CBlock);			// allocate and add another block
	INT_PTR	iIndex = m_nSize++;
	(*this)[iIndex] = newElement;	// add new element to last block
	return iIndex;
}

template<class TYPE, int BLOCK_SIZE>
AFX_INLINE void CBlockArray<TYPE, BLOCK_SIZE>::RemoveAll()
{
	INT_PTR	nBlocks = m_arrBlock.GetSize();
	for (INT_PTR iBlock = 0; iBlock < nBlocks; iBlock++)	// for each block
		delete m_arrBlock[iBlock];	// delete block
	m_arrBlock.RemoveAll();
	m_nSize = 0;
}

template<class TYPE, int BLOCK_SIZE>
AFX_INLINE void CBlockArray<TYPE, BLOCK_SIZE>::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {	// if storing
		ar.WriteCount(m_nSize);	// handles both 32-bit and 64-bit sizes
		INT_PTR	nBlocks = m_arrBlock.GetSize();
		for (INT_PTR iBlock = 0; iBlock < nBlocks; iBlock++)	// for each block
			ar.Write(m_arrBlock[iBlock], sizeof(CBlock));	// store block
	} else {	// loading
		SetSize(ar.ReadCount());	// allocate blocks
		INT_PTR	nBlocks = m_arrBlock.GetSize();
		for (INT_PTR iBlock = 0; iBlock < nBlocks; iBlock++)	// for each block
			ar.Read(m_arrBlock[iBlock], sizeof(CBlock));	// load block
	}
}

template<class TYPE, int BLOCK_SIZE>
AFX_INLINE INT_PTR CBlockArray<TYPE, BLOCK_SIZE>::GetBlockCount() const
{
	return m_arrBlock.GetSize();
}

template<class TYPE, int BLOCK_SIZE>
AFX_INLINE TYPE* CBlockArray<TYPE, BLOCK_SIZE>::GetBlock(INT_PTR iBlock) const
{
	return reinterpret_cast<TYPE*>(m_arrBlock[iBlock]);
}

template<class TYPE, int BLOCK_SIZE>
AFX_INLINE INT_PTR CBlockArray<TYPE, BLOCK_SIZE>::GetBlockSize(INT_PTR iBlock) const
{
	if (iBlock < m_arrBlock.GetSize() - 1) {	// if not last block
		return BLOCK_SIZE;	// block is full by definition
	} else {	// last block
		return m_nSize % BLOCK_SIZE;	// block may not be full
	}
}
