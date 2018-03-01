*** Whorld source distribution ***

REQUIREMENTS:

Whorld is designed to be an MFC VC++ 6.0 project.  It may or may not
compile correctly in other versions of MFC, or in .NET.  Whorld has
mainly been tested under Windows 2000 SP4, but it should work fine
under XP SP2.

You'll need the following files from the DirectX 8.1 SDK:

ddraw.h
ddrawex.h
ddraw.lib
dxguid.lib
dshow.h (and all its dependencies)
strmbase.lib
quartz.lib

Note that Whorld may not be compatible with later versions of the SDK.

You'll also need the following files from HTML help:

htmlhelp.h
htmlhelp.lib

The files are available via links on the Whorld web site's development
page.  You can add include and lib paths for them, or simply copy them
into Whorld's folder.

INSTALLATION:

The source distribution of Whorld doesn't have an installer.  Whorld
assumes the following files exist in the same folder somewhere on your
hard drive:

Whorld.exe
Whorld.chm

Note that to use Whorld's movie export feature, you must register the
BmpToAvi DirectShow source filter, which is included in this release.
The filter only needs to be registered once, via the following command
(assuming BmpToAvi.ax is in the current folder):

regsvr32 BmpToAvi.ax

UNINSTALLING:

To uninstall Whorld, you need to do three things:

1.  Delete Whorld's program files from the folder you copied them to.
2.  Delete Whorld's registry entries, using regedit.  The key to
delete is:

"HKEY_CURRENT_USER\Software\Anal Software\Whorld"

3.  If you registered the BmpToAvi filter, unregister it, via the 
following command:

regsvr32 /u BmpToAvi.ax

NOTES:

The binary version of this release is available from the Whorld web
site, http://whorld.org.  Additional patches and printable versions of
the help are available from the Download page.

LICENSE:

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.  This program is distributed in the hope that
it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.  You should have
received a copy of the GNU General Public License along with this
program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111 USA.

END
