# Microsoft Developer Studio Project File - Name="Whorld" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Whorld - Win32 Unicode Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Whorld.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Whorld.mak" CFG="Whorld - Win32 Unicode Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Whorld - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Whorld - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Whorld - Win32 Unicode Release" (based on "Win32 (x86) Application")
!MESSAGE "Whorld - Win32 Unicode Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Whorld - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GX /O2 /Oy- /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 shlwapi.lib ddraw.lib dxguid.lib version.lib dinput.lib winmm.lib htmlhelp.lib strmbase.lib quartz.lib vfw32.lib imagehlp.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /profile /debug

!ELSEIF  "$(CFG)" == "Whorld - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 shlwapi.lib ddraw.lib dxguid.lib version.lib dinput.lib winmm.lib htmlhelp.lib strmbase.lib quartz.lib vfw32.lib imagehlp.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "Whorld - Win32 Unicode Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "UnicodeRelease"
# PROP BASE Intermediate_Dir "UnicodeRelease"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "UnicodeRelease"
# PROP Intermediate_Dir "UnicodeRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /Oy- /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GX /O2 /Oy- /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "UNICODE" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 shlwapi.lib ddraw.lib dxguid.lib version.lib dinput.lib winmm.lib htmlhelp.lib strmbase.lib quartz.lib vfw32.lib imagehlp.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT BASE LINK32 /profile /debug
# ADD LINK32 shlwapi.lib ddraw.lib dxguid.lib version.lib dinput.lib winmm.lib htmlhelp.lib strmbase.lib quartz.lib vfw32.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386
# SUBTRACT LINK32 /profile /debug

!ELSEIF  "$(CFG)" == "Whorld - Win32 Unicode Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "UnicodeDebug"
# PROP BASE Intermediate_Dir "UnicodeDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "UnicodeDebug"
# PROP Intermediate_Dir "UnicodeDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "UNICODE" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 shlwapi.lib ddraw.lib dxguid.lib version.lib dinput.lib winmm.lib htmlhelp.lib strmbase.lib quartz.lib vfw32.lib imagehlp.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 shlwapi.lib ddraw.lib dxguid.lib version.lib dinput.lib winmm.lib htmlhelp.lib strmbase.lib quartz.lib vfw32.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Whorld - Win32 Release"
# Name "Whorld - Win32 Debug"
# Name "Whorld - Win32 Unicode Release"
# Name "Whorld - Win32 Unicode Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AboutDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AboutDlg.h
# End Source File
# Begin Source File

SOURCE=.\AuxFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\AuxFrame.h
# End Source File
# Begin Source File

SOURCE=.\AuxView.cpp
# End Source File
# Begin Source File

SOURCE=.\AuxView.h
# End Source File
# Begin Source File

SOURCE=.\AviFileErrs.h
# End Source File
# Begin Source File

SOURCE=.\AviToBmp.cpp
# End Source File
# Begin Source File

SOURCE=.\AviToBmp.h
# End Source File
# Begin Source File

SOURCE=.\BackBufDD.cpp
# End Source File
# Begin Source File

SOURCE=.\BackBufDD.h
# End Source File
# Begin Source File

SOURCE=.\BackBufGDI.cpp
# End Source File
# Begin Source File

SOURCE=.\BackBufGDI.h
# End Source File
# Begin Source File

SOURCE=.\Benchmark.cpp
# End Source File
# Begin Source File

SOURCE=.\Benchmark.h
# End Source File
# Begin Source File

SOURCE=.\BmpToAvi.cpp
# End Source File
# Begin Source File

SOURCE=.\BmpToAvi.h
# End Source File
# Begin Source File

SOURCE=.\BmpToAviCo.h
# End Source File
# Begin Source File

SOURCE=.\BmpToAviErrs.h
# End Source File
# Begin Source File

SOURCE=.\ClickSliderCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ClickSliderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\Clipboard.cpp
# End Source File
# Begin Source File

SOURCE=.\Clipboard.h
# End Source File
# Begin Source File

SOURCE=.\CrossDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CrossDlg.h
# End Source File
# Begin Source File

SOURCE=.\CtrlResize.cpp
# End Source File
# Begin Source File

SOURCE=.\CtrlResize.h
# End Source File
# Begin Source File

SOURCE=.\DirDrawErrs.h
# End Source File
# Begin Source File

SOURCE=.\DirInput.cpp
# End Source File
# Begin Source File

SOURCE=.\DirInput.h
# End Source File
# Begin Source File

SOURCE=.\DirInputErrs.h
# End Source File
# Begin Source File

SOURCE=.\DirShowU.cpp
# End Source File
# Begin Source File

SOURCE=.\DirShowU.h
# End Source File
# Begin Source File

SOURCE=.\DPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\DPoint.h
# End Source File
# Begin Source File

SOURCE=.\DragListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\DragListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\EditListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\EditListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\EditSliderCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\EditSliderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\ExportDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportDlg.h
# End Source File
# Begin Source File

SOURCE=.\FileSearchDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FileSearchDlg.h
# End Source File
# Begin Source File

SOURCE=.\FolderDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderDialog.h
# End Source File
# Begin Source File

SOURCE=.\FormatIO.cpp
# End Source File
# Begin Source File

SOURCE=.\FormatIO.h
# End Source File
# Begin Source File

SOURCE=.\hls.cpp
# End Source File
# Begin Source File

SOURCE=.\hls.h
# End Source File
# Begin Source File

SOURCE=.\HotKeyCtrlEx.cpp
# End Source File
# Begin Source File

SOURCE=.\HotKeyCtrlEx.h
# End Source File
# Begin Source File

SOURCE=.\Hyperlink.cpp
# End Source File
# Begin Source File

SOURCE=.\Hyperlink.h
# End Source File
# Begin Source File

SOURCE=.\KeyboardLeds.cpp
# End Source File
# Begin Source File

SOURCE=.\KeyboardLeds.h
# End Source File
# Begin Source File

SOURCE=.\MainDef.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MasterDef.h
# End Source File
# Begin Source File

SOURCE=.\MasterDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MasterDlg.h
# End Source File
# Begin Source File

SOURCE=.\MessageBoxCheck.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageBoxCheck.h
# End Source File
# Begin Source File

SOURCE=.\MidiAutoAssignDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MidiAutoAssignDlg.h
# End Source File
# Begin Source File

SOURCE=.\MidiCtrlrDef.h
# End Source File
# Begin Source File

SOURCE=.\MidiIO.cpp
# End Source File
# Begin Source File

SOURCE=.\MidiIO.h
# End Source File
# Begin Source File

SOURCE=.\MidiProps.h
# End Source File
# Begin Source File

SOURCE=.\MidiSetupDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MidiSetupDlg.h
# End Source File
# Begin Source File

SOURCE=.\MidiSetupRow.cpp
# End Source File
# Begin Source File

SOURCE=.\MidiSetupRow.h
# End Source File
# Begin Source File

SOURCE=.\MissingFilesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MissingFilesDlg.h
# End Source File
# Begin Source File

SOURCE=.\MovieExportDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MovieExportDlg.h
# End Source File
# Begin Source File

SOURCE=.\MoviePlayerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MoviePlayerDlg.h
# End Source File
# Begin Source File

SOURCE=.\MovieResizeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MovieResizeDlg.h
# End Source File
# Begin Source File

SOURCE=.\MultiFileDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiFileDlg.h
# End Source File
# Begin Source File

SOURCE=.\NoAccessKeys.cpp
# End Source File
# Begin Source File

SOURCE=.\NoAccessKeys.h
# End Source File
# Begin Source File

SOURCE=.\NoteEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteEdit.h
# End Source File
# Begin Source File

SOURCE=.\NumbersDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NumbersDlg.h
# End Source File
# Begin Source File

SOURCE=.\NumEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\NumEdit.h
# End Source File
# Begin Source File

SOURCE=.\NumSpin.cpp
# End Source File
# Begin Source File

SOURCE=.\NumSpin.h
# End Source File
# Begin Source File

SOURCE=.\OptionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsDlg.h
# End Source File
# Begin Source File

SOURCE=.\OptsDisplayDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptsDisplayDlg.h
# End Source File
# Begin Source File

SOURCE=.\OptsFolderDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptsFolderDlg.h
# End Source File
# Begin Source File

SOURCE=.\OptsInputDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptsInputDlg.h
# End Source File
# Begin Source File

SOURCE=.\OptsMidiDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptsMidiDlg.h
# End Source File
# Begin Source File

SOURCE=.\Oscillator.cpp
# End Source File
# Begin Source File

SOURCE=.\Oscillator.h
# End Source File
# Begin Source File

SOURCE=.\ParmDef.h
# End Source File
# Begin Source File

SOURCE=.\ParmInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\ParmInfo.h
# End Source File
# Begin Source File

SOURCE=.\ParmRow.cpp
# End Source File
# Begin Source File

SOURCE=.\ParmRow.h
# End Source File
# Begin Source File

SOURCE=.\ParmsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ParmsDlg.h
# End Source File
# Begin Source File

SOURCE=.\Patch.cpp
# End Source File
# Begin Source File

SOURCE=.\Patch.h
# End Source File
# Begin Source File

SOURCE=.\PatchLink.cpp
# End Source File
# Begin Source File

SOURCE=.\PatchLink.h
# End Source File
# Begin Source File

SOURCE=.\PathStr.cpp
# End Source File
# Begin Source File

SOURCE=.\PathStr.h
# End Source File
# Begin Source File

SOURCE=.\Persist.cpp
# End Source File
# Begin Source File

SOURCE=.\Persist.h
# End Source File
# Begin Source File

SOURCE=.\PersistDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PersistDlg.h
# End Source File
# Begin Source File

SOURCE=.\PersistVal.h
# End Source File
# Begin Source File

SOURCE=.\Playlist.cpp
# End Source File
# Begin Source File

SOURCE=.\Playlist.h
# End Source File
# Begin Source File

SOURCE=.\PlaylistDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PlaylistDlg.h
# End Source File
# Begin Source File

SOURCE=.\PlaylistMidiDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PlaylistMidiDlg.h
# End Source File
# Begin Source File

SOURCE=.\ProgressDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressDlg.h
# End Source File
# Begin Source File

SOURCE=.\RandList.cpp
# End Source File
# Begin Source File

SOURCE=.\RandList.h
# End Source File
# Begin Source File

SOURCE=.\RealTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\RealTimer.h
# End Source File
# Begin Source File

SOURCE=.\ReplaceFilesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ReplaceFilesDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\RowDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\RowDialog.h
# End Source File
# Begin Source File

SOURCE=.\RowDialogForm.cpp
# End Source File
# Begin Source File

SOURCE=.\RowDialogForm.h
# End Source File
# Begin Source File

SOURCE=.\RowDialogRow.cpp
# End Source File
# Begin Source File

SOURCE=.\RowDialogRow.h
# End Source File
# Begin Source File

SOURCE=.\RunAvg.cpp
# End Source File
# Begin Source File

SOURCE=.\RunAvg.h
# End Source File
# Begin Source File

SOURCE=.\SlideShowDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SlideShowDlg.h
# End Source File
# Begin Source File

SOURCE=.\SnapMovie.cpp
# End Source File
# Begin Source File

SOURCE=.\SnapMovie.h
# End Source File
# Begin Source File

SOURCE=.\Snapshot.cpp
# End Source File
# Begin Source File

SOURCE=.\Snapshot.h
# End Source File
# Begin Source File

SOURCE=.\SortStringArray.cpp
# End Source File
# Begin Source File

SOURCE=.\SortStringArray.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SubFileFind.cpp
# End Source File
# Begin Source File

SOURCE=.\SubFileFind.h
# End Source File
# Begin Source File

SOURCE=.\ThumbList.cpp
# End Source File
# Begin Source File

SOURCE=.\ThumbList.h
# End Source File
# Begin Source File

SOURCE=.\ToolDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolDlg.h
# End Source File
# Begin Source File

SOURCE=.\Version.h
# End Source File
# Begin Source File

SOURCE=.\VersionInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\VersionInfo.h
# End Source File
# Begin Source File

SOURCE=.\Video.cpp
# End Source File
# Begin Source File

SOURCE=.\Video.h
# End Source File
# Begin Source File

SOURCE=.\VideoComprDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoComprDlg.h
# End Source File
# Begin Source File

SOURCE=.\VideoComprList.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoComprList.h
# End Source File
# Begin Source File

SOURCE=.\VideoComprState.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoComprState.h
# End Source File
# Begin Source File

SOURCE=.\VideoList.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoList.h
# End Source File
# Begin Source File

SOURCE=.\ViewDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewDialog.h
# End Source File
# Begin Source File

SOURCE=.\WCritSec.cpp
# End Source File
# Begin Source File

SOURCE=.\WCritSec.h
# End Source File
# Begin Source File

SOURCE=.\WEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\WEvent.h
# End Source File
# Begin Source File

SOURCE=.\Whorld.cpp
# End Source File
# Begin Source File

SOURCE=.\Whorld.h
# End Source File
# Begin Source File

SOURCE=.\Whorld.rc
# End Source File
# Begin Source File

SOURCE=.\WhorldDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\WhorldDoc.h
# End Source File
# Begin Source File

SOURCE=.\WhorldRCDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\WhorldRCDoc.h
# End Source File
# Begin Source File

SOURCE=.\WhorldView.cpp
# End Source File
# Begin Source File

SOURCE=.\WhorldView.h
# End Source File
# Begin Source File

SOURCE=.\WhorldViewDD.cpp
# End Source File
# Begin Source File

SOURCE=.\WhorldViewDD.h
# End Source File
# Begin Source File

SOURCE=.\Win32Console.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32Console.h
# End Source File
# Begin Source File

SOURCE=.\WObject.h
# End Source File
# Begin Source File

SOURCE=.\WThread.cpp
# End Source File
# Begin Source File

SOURCE=.\WThread.h
# End Source File
# Begin Source File

SOURCE=.\WTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\WTimer.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\dragmulti.cur
# End Source File
# Begin Source File

SOURCE=.\res\dragsingle.cur
# End Source File
# Begin Source File

SOURCE=.\res\go_in.ico
# End Source File
# Begin Source File

SOURCE=.\res\go_start.ico
# End Source File
# Begin Source File

SOURCE=.\res\goto_end.ico
# End Source File
# Begin Source File

SOURCE=.\res\goto_start.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\loopd.ico
# End Source File
# Begin Source File

SOURCE=.\res\loopu.ico
# End Source File
# Begin Source File

SOURCE=.\res\open.ico
# End Source File
# Begin Source File

SOURCE=.\res\paused.ico
# End Source File
# Begin Source File

SOURCE=.\res\pauseu.ico
# End Source File
# Begin Source File

SOURCE=.\res\pl_norma.ico
# End Source File
# Begin Source File

SOURCE=.\res\pl_normal.ico
# End Source File
# Begin Source File

SOURCE=.\res\pl_playing.ico
# End Source File
# Begin Source File

SOURCE=.\res\pl_select.ico
# End Source File
# Begin Source File

SOURCE=.\res\pl_xfade.ico
# End Source File
# Begin Source File

SOURCE=.\res\pl_xfading.ico
# End Source File
# Begin Source File

SOURCE=.\res\playd.ico
# End Source File
# Begin Source File

SOURCE=.\res\playu.ico
# End Source File
# Begin Source File

SOURCE=.\res\save.ico
# End Source File
# Begin Source File

SOURCE=.\res\set_in.ico
# End Source File
# Begin Source File

SOURCE=.\res\set_in2.ico
# End Source File
# Begin Source File

SOURCE=.\res\set_out.ico
# End Source File
# Begin Source File

SOURCE=.\res\stopd.ico
# End Source File
# Begin Source File

SOURCE=.\res\stopu.ico
# End Source File
# Begin Source File

SOURCE=.\res\take_sna.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Whorld.ico
# End Source File
# Begin Source File

SOURCE=.\res\Whorld.rc2
# End Source File
# Begin Source File

SOURCE=.\res\WhorldDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\WhorldMovie.ico
# End Source File
# Begin Source File

SOURCE=.\res\WhorldPlaylist.ico
# End Source File
# Begin Source File

SOURCE=.\res\WhorldSnapshot.ico
# End Source File
# End Group
# End Target
# End Project
# Section Whorld : {B09A0CA2-8680-4126-9794-57FC72C536EF}
# 	1:13:IDD_ABOUTBOX1:102
# 	2:16:Resource Include:resource.h
# 	2:12:AboutDlg.cpp:AboutDlg.cpp
# 	2:10:ENUM: enum:enum
# 	2:16:CLASS: CAboutDlg:CAboutDlg
# 	2:19:Application Include:Whorld.h
# 	2:10:AboutDlg.h:AboutDlg.h
# 	2:12:IDD_ABOUTBOX:IDD_ABOUTBOX1
# End Section
# Section Whorld : {14CEB719-1628-4E15-976F-5A0576FBE91E}
# 	1:19:IDD_FILE_SEARCH_DLG:121
# 	2:21:CLASS: CFileSearchDlg:CFileSearchDlg
# 	2:16:Resource Include:resource.h
# 	2:17:FileSearchDlg.cpp:FileSearchDlg.cpp
# 	2:19:IDD_FILE_SEARCH_DLG:IDD_FILE_SEARCH_DLG
# 	2:10:ENUM: enum:enum
# 	2:15:FileSearchDlg.h:FileSearchDlg.h
# 	2:19:Application Include:whorld.h
# End Section
# Section Whorld : {92C2BF02-0DAC-4E74-AFBA-775457F78B2F}
# 	1:21:IDD_MISSING_FILES_DLG:119
# 	2:16:Resource Include:resource.h
# 	2:17:MissingFilesDlg.h:MissingFilesDlg.h
# 	2:21:IDD_MISSING_FILES_DLG:IDD_MISSING_FILES_DLG
# 	2:10:ENUM: enum:enum
# 	2:19:MissingFilesDlg.cpp:MissingFilesDlg.cpp
# 	2:23:CLASS: CMissingFilesDlg:CMissingFilesDlg
# 	2:19:Application Include:whorld.h
# End Section
# Section Whorld : {C3926611-4B2A-4B5B-9309-271E8192D601}
# 	1:10:IDD_OPTROW:103
# 	2:16:Resource Include:resource.h
# 	2:14:CLASS: COptRow:COptRow
# 	2:10:IDD_OPTROW:IDD_OPTROW
# 	2:10:ENUM: enum:enum
# 	2:8:OptRow.h:OptRow.h
# 	2:19:Application Include:Whorld.h
# 	2:10:OptRow.cpp:OptRow.cpp
# End Section
# Section Whorld : {FC140479-10DE-47FD-81F7-68431616C92C}
# 	1:21:IDD_REPLACE_FILES_DLG:120
# 	2:16:Resource Include:resource.h
# 	2:10:ENUM: enum:enum
# 	2:17:ReplaceFilesDlg.h:ReplaceFilesDlg.h
# 	2:23:CLASS: CReplaceFilesDlg:CReplaceFilesDlg
# 	2:21:IDD_REPLACE_FILES_DLG:IDD_REPLACE_FILES_DLG
# 	2:19:Application Include:whorld.h
# 	2:19:ReplaceFilesDlg.cpp:ReplaceFilesDlg.cpp
# End Section
# Section Whorld : {FFF744F3-C987-4F5D-BD24-22EEA652F3ED}
# 	2:15:CLASS: CNumEdit:CNumEdit
# 	2:11:NumEdit.cpp:NumEdit.cpp
# 	2:9:NumEdit.h:NumEdit.h
# 	2:19:Application Include:Whorld.h
# End Section
# Section Whorld : {823A6916-21D7-4F24-904A-3BA76B741CB5}
# 	2:19:ClickSliderCtrl.cpp:ClickSliderCtrl.cpp
# 	2:10:ENUM: enum:enum
# 	2:23:CLASS: CClickSliderCtrl:CClickSliderCtrl
# 	2:17:ClickSliderCtrl.h:ClickSliderCtrl.h
# 	2:19:Application Include:Whorld.h
# End Section
# Section Whorld : {FA9EA80A-8443-4166-9985-F828C99C78CD}
# 	1:11:IDD_OPTIONS:104
# 	2:16:Resource Include:resource.h
# 	2:12:OptionsDlg.h:OptionsDlg.h
# 	2:10:ENUM: enum:enum
# 	2:11:IDD_OPTIONS:IDD_OPTIONS
# 	2:18:CLASS: COptionsDlg:COptionsDlg
# 	2:14:OptionsDlg.cpp:OptionsDlg.cpp
# 	2:19:Application Include:Whorld.h
# End Section
