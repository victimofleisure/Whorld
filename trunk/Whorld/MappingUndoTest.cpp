// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08oct13	initial version
        01      07may14	move generic functionality to base class
		02		09sep14	move enable flag to Globals.h
		03		15mar15	remove 16-bit assert in rename case
		04		06apr15	in DoPageEdit, skip disabled controls
		05		02jan19	add case for range type property
		06		26feb19	change master property default to fail gracefully
		07		02dec19	remove sort function, array now provides it
		08		20mar20	add mapping
		09		29mar20	add sort and learn mapping
		10		07apr20	add move steps
		11		19nov20	use set channel property methods
		12		19nov20	add randomized docking bar visibility
		13		21jun21	route track editing commands directly to document
		14		22jan22	limit channel tests to event properties
		15		10sep24	add method to randomize channel property
		16		02mar25	adapt for Whorld
		17		19mar25	make mapping range real instead of integer
		18		29mar25	add playlist edits

		automated undo test for Whorld mapping
 
*/

#include "stdafx.h"

// to enable this test, edit Globals.h and set UNDO_TEST to 1

#if UNDO_TEST == 2	// do not change this unique identifier

#include "Whorld.h"
#include "MappingUndoTest.h"
#include "MainFrm.h"
#include "UndoState.h"
#include "RandList.h"
#include "MasterRowDlg.h"

#define TIMER_PERIOD 1	// timer period, in milliseconds

// pause only matters if receiving MIDI input during test,
// in which case, also back off the timer period to 10 ms
#define PAUSE_VIEW_DURING_TEST false	// unpaused is a tougher test

#define midiMaps theApp.m_midiMgr.m_midiMaps
#define mappingPane theApp.GetMainFrame()->m_wndMappingBar
#define patchArray theApp.m_pPlaylist->m_arrPatch
#define playlistPane theApp.GetMainFrame()->m_wndPlaylistBar

static CMappingUndoTest gUndoTest(true);	// one and only instance, initially running

const CMappingUndoTest::EDIT_INFO CMappingUndoTest::m_arrEditInfo[] = {
	{UCODE_PROPERTY,			1},
	{UCODE_MULTI_PROPERTY,		1},
	{UCODE_INSERT,				0.5f},
	{UCODE_DELETE,				0.2f},
	{UCODE_CUT,					0.2f},
	{UCODE_PASTE,				0.5f},
	{UCODE_MOVE,				0.3f},
	{UCODE_SORT,				0.2f},
	{UCODE_LEARN,				1},
	{UCODE_LEARN_MULTI,			0.5f},
	{UCODE_PLAYLIST_INSERT,		0.1f},
	{UCODE_PLAYLIST_DELETE,		0.1f},
	{UCODE_PLAYLIST_MOVE,		0.1f},
};

CMappingUndoTest::CMappingUndoTest(bool bInitRunning) :
	CUndoTest(bInitRunning, TIMER_PERIOD, m_arrEditInfo, _countof(m_arrEditInfo))
{
#if 0
	m_nCycles = 1;
	m_nPasses = 2;
	m_nPassEdits = 10;
	m_nPassUndos = 5;
	m_nMaxEdits = INT_MAX;
	m_nRandSeed = 666;
	m_bMakeSnapshots = true;
#else
	m_nCycles = 1;
	m_nPasses = 10;
	m_nPassEdits = 250;
	m_nPassUndos = 100;
	m_nMaxEdits = INT_MAX;
	m_nRandSeed = 666;
	m_bMakeSnapshots = true;
#endif
}

CMappingUndoTest::~CMappingUndoTest()
{
}

LONGLONG CMappingUndoTest::GetSnapshot() const
{
	LONGLONG	nSum = 0;
	const CMappingArray&	aMapping = midiMaps.GetArray();
	nSum += Fletcher64(aMapping.GetData(), aMapping.GetSize() * sizeof(CMapping));
	const CPlaylist::CPatchLinkArray&	aPatch = patchArray;
	int	nPatches = patchArray.GetSize(); 
	for (int iPatch = 0; iPatch < nPatches; iPatch++) {	// for each patch link
		const CString&	sPath = aPatch[iPatch].m_sPath;
		nSum += Fletcher64(sPath.GetString(), sPath.GetLength() * sizeof(TCHAR));
	}
//	_tprintf(_T("%I64x\n"), nSum);
	return nSum;
}

CMappingBase::VARIANT_PROP CMappingUndoTest::MakeRandomMappingProperty(int iProp)
{
	VARIANT_PROP	prop = {0};	// clear entire property
	switch (iProp) {
	case PROP_EVENT:
		prop.intVal = Random(EVENTS);
		break;
	case PROP_CHANNEL:
		prop.intVal = Random(MIDI_CHANNELS);
		break;
	case PROP_CONTROL:
		prop.intVal = Random(MIDI_NOTE_MAX);
		break;
	case PROP_TARGET:
		prop.intVal = Random(TARGETS);
		break;
	case PROP_PROPERTY:
		prop.intVal = Random(PARAM_PROP_COUNT);
		break;
	case PROP_START:
	case PROP_END:
		prop.dblVal = RandomFloat(MIDI_NOTE_MAX);
		break;
	default:
		ASSERT(0);	// missing case
	}
	return prop;
}

bool CMappingUndoTest::MakeRandomSelection(int nItems, CIntArrayEx& arrSelection) const
{
	if (nItems <= 0)
		return false;
	int	nSels = Random(nItems) + 1;	// select at least one item
	CRandList	list(nItems);
	arrSelection.SetSize(nSels);
	for (int iSel = 0; iSel < nSels; iSel++)	// for each selection
		arrSelection[iSel] = list.GetNext();	// select random track
	arrSelection.Sort();
	return true;
}

CString	CMappingUndoTest::PrintSelection(CIntArrayEx& arrSelection) const
{
	CString	str;
	str = '[';
	int	nSels = arrSelection.GetSize();
	for (int iSel = 0; iSel < nSels; iSel++) {	// for each selection
		if (iSel)
			str += ',';
		CString	s;
		s.Format(_T("%d"), arrSelection[iSel]);
		str += s;
	}
	str += ']';
	return str;
}

int CMappingUndoTest::ApplyEdit(int nUndoCode)
{
	CUndoState	state(0, nUndoCode);
	CString	sUndoTitle(mappingPane.GetUndoTitle(state));
	switch (nUndoCode) {
	case UCODE_PROPERTY:
		{
			int	iMapping = Random(midiMaps.GetCount());
			if (iMapping < 0)
				return DISABLED;
			int	iProp = Random(PROPERTIES);
			VARIANT_PROP	prop = MakeRandomMappingProperty(iProp);
			mappingPane.SetProperty(iMapping, iProp, prop);
		}
		break;
	case UCODE_MULTI_PROPERTY:
		{
			int	nMappings = midiMaps.GetCount(); 
			CIntArrayEx	arrSelection;
			if (!MakeRandomSelection(nMappings, arrSelection))
				return DISABLED;
			int	iProp = Random(PROPERTIES);
			VARIANT_PROP	prop = MakeRandomMappingProperty(iProp);
			mappingPane.SetProperty(arrSelection, iProp, prop);
		}
		break;
	case UCODE_INSERT:
		{
			int iInsPos = max(Random(midiMaps.GetCount()), 0);
			mappingPane.Insert(iInsPos);
			PRINTF(_T("%s %d\n"), sUndoTitle, iInsPos);
		}
		break;
	case UCODE_CUT:
	case UCODE_DELETE:
		{
			int	nMappings = midiMaps.GetCount(); 
			CIntArrayEx	arrSelection;
			if (!MakeRandomSelection(nMappings, arrSelection))
				return DISABLED;
			mappingPane.GetListCtrl().SetSelection(arrSelection);
			if (nUndoCode == UCODE_CUT) {
				mappingPane.Cut(arrSelection);
			} else {
				mappingPane.Delete(arrSelection);
			}
			PRINTF(_T("%s %s\n"), sUndoTitle, PrintSelection(arrSelection));
		}
		break;
	case UCODE_PASTE:
		{
			if (!mappingPane.CanPaste())
				return DISABLED;
			int iInsPos = max(Random(midiMaps.GetCount()), 0);
			mappingPane.Paste(iInsPos);
			PRINTF(_T("%s %d\n"), sUndoTitle, iInsPos);
		}
		break;
	case UCODE_MOVE:
		{
			int	nMappings = midiMaps.GetCount(); 
			if (nMappings < 2)
				return DISABLED;
			CIntArrayEx	arrSelection;
			if (!MakeRandomSelection(nMappings, arrSelection))
				return DISABLED;
			int	iDropPos = Random(nMappings + 1);
			if (!CDragVirtualListCtrl::CompensateDropPos(arrSelection, iDropPos))
				return DISABLED;
			mappingPane.Move(arrSelection, iDropPos);
			PRINTF(_T("%s %s %d\n"), sUndoTitle, PrintSelection(arrSelection), iDropPos);
		}
		break;
	case UCODE_SORT:
		{
			int	nMappings = midiMaps.GetCount(); 
			if (nMappings < 2)
				return DISABLED;
			int	iProp = Random(PROPERTIES);
			bool	bDescending = Random(2) != 0;
			mappingPane.Sort(iProp, bDescending);
			PRINTF(_T("%s %d\n"), sUndoTitle, iProp);
		}
		break;
	case UCODE_LEARN:
		{
			int	iMapping = Random(midiMaps.GetCount());
			if (iMapping < 0)
				return DISABLED;
			int	nInMidiMsg = MakeMidiMsg(MIDI_IDX_CMD(Random(MIDI_CHANNEL_VOICE_MESSAGES)),
				Random(MIDI_CHANNELS), Random(MIDI_NOTES), Random(MIDI_NOTES));
			mappingPane.LearnMapping(iMapping, nInMidiMsg);
			PRINTF(_T("%s %d %x\n"), sUndoTitle, iMapping, nInMidiMsg);
		}
		break;
	case UCODE_LEARN_MULTI:
		{
			CIntArrayEx	arrSelection;
			if (!MakeRandomSelection(midiMaps.GetCount(), arrSelection))
				return DISABLED;
			int	nInMidiMsg = MakeMidiMsg(MIDI_IDX_CMD(Random(MIDI_CHANNEL_VOICE_MESSAGES)),
				Random(MIDI_CHANNELS), Random(MIDI_NOTES), Random(MIDI_NOTES));
			mappingPane.LearnMappings(arrSelection, nInMidiMsg);
			PRINTF(_T("%s %s %x\n"), sUndoTitle, PrintSelection(arrSelection), nInMidiMsg);
		}
		break;
	case UCODE_PLAYLIST_INSERT:
		{
			const int	MAX_PATCHES = 10;
			const int	TEST_CHARS = 26;
			int	nInsPatches = Random(MAX_PATCHES - 1) + 1;
			CPlaylist::CPatchLinkArray	arrPatch;
			arrPatch.SetSize(nInsPatches);
			for (int iPatch = 0; iPatch < nInsPatches; iPatch++) {	// for each patch link
				int	nPathLen = Random(MAX_PATH - 1) + 1;
				CString	sPath;
				LPTSTR	pszPath = sPath.GetBuffer(nPathLen);
				int	iStartChar = Random(TEST_CHARS);
				for (int iChar = 0; iChar < nPathLen; iChar++) {
					pszPath[iChar] = static_cast<TCHAR>(((iStartChar + iChar) % TEST_CHARS) + 'A');
				}
				sPath.ReleaseBuffer(nPathLen);
				arrPatch[iPatch].m_sPath = sPath;
			}
			int iInsPos = max(Random(patchArray.GetSize()), 0);
			playlistPane.Insert(iInsPos, arrPatch);
			PRINTF(_T("%s %d\n"), sUndoTitle, iInsPos);
		}
		break;
	case UCODE_PLAYLIST_DELETE:
		{
			int	nPatches = patchArray.GetSize(); 
			CIntArrayEx	arrSelection;
			if (!MakeRandomSelection(nPatches, arrSelection))
				return DISABLED;
			playlistPane.GetListCtrl().SetSelection(arrSelection);
			playlistPane.Delete(arrSelection);
			PRINTF(_T("%s %s\n"), sUndoTitle, PrintSelection(arrSelection));
		}
		break;
	case UCODE_PLAYLIST_MOVE:
		{
			int	nPatches = patchArray.GetSize(); 
			if (nPatches < 2)
				return DISABLED;
			CIntArrayEx	arrSelection;
			if (!MakeRandomSelection(nPatches, arrSelection))
				return DISABLED;
			int	iDropPos = Random(nPatches + 1);
			if (!CDragVirtualListCtrl::CompensateDropPos(arrSelection, iDropPos))
				return DISABLED;
			playlistPane.Move(arrSelection, iDropPos);
			PRINTF(_T("%s %s %d\n"), sUndoTitle, PrintSelection(arrSelection), iDropPos);
		}
		break;
	default:
		NODEFAULTCASE;
		return ABORT;
	}
	return SUCCESS;
}

bool CMappingUndoTest::Create()
{
	theApp.SetPause(PAUSE_VIEW_DURING_TEST);
	m_pUndoMgr = mappingPane.GetUndoManager();
	m_pUndoMgr->SetLevels(-1);	// unlimited undo
	mappingPane.ShowPane(true, 0, true);
	playlistPane.ShowPane(true, 0, true);
	if (!CUndoTest::Create())
		return false;
	return true;
}

void CMappingUndoTest::Destroy()
{
	CUndoTest::Destroy();
	mappingPane.SetModifiedFlag(false);
}

#endif
