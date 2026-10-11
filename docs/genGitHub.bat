@echo off
copy C:\Chris\MyProjects\Whorld\web\*.html
copy downloadGitHub.html download.html
rem echo y | rd /s C:\Chris\MyProjects\Whorld\web\Help
rem ren helpframe.html helpframe.txt
rem attrib +r gallery\*.* /s
rem rem attrib +r temp\*.* /s
C:\Chris\tools\navgen templateGitHub.html .
if errorlevel 1 goto err
"C:\Chris\MyProjects\FixSelfUrl\Release\FixSelfUrl" *.html
if errorlevel 1 goto err
attrib -r gallery\*.* /s
rem attrib -r temp\*.* /s
cd gallery
copy D:\Temp\MyProjects\Whorld\web\Gallery\*.html
C:\Chris\tools\navgen templateGitHub.html .
if errorlevel 1 goto err
"C:\Chris\MyProjects\FixSelfUrl\Release\FixSelfUrl" *.html
if errorlevel 1 goto err
cd..
if errorlevel 1 goto err
ren helpframe.txt helpframe.html
md Help
C:\Chris\MyProjects\doc2web\release\doc2web /nospaces D:\Temp\MyProjects\Whorld\Help\help.txt Help Contents.htm D:\Temp\MyProjects\Whorld\doc\WhorldHelp.htm "Whorld Help"
if errorlevel 1 goto err
cd Help
md images
copy D:\Temp\MyProjects\Whorld\Help\images\*.* images
copy ..\helptopic.css content.css
C:\Chris\tools\navgen D:\Temp\MyProjects\Whorld\Help\template.txt .
copy ..\helpheader.txt x
copy x + Contents.htm
echo ^<body^>^<html^> >>x
del Contents.htm
ren x Contents.htm
md printable
move D:\Temp\MyProjects\Whorld\doc\whorldhelp.htm printable
copy D:\Temp\MyProjects\Whorld\doc\whorldhelp.doc printable
copy D:\Temp\MyProjects\Whorld\doc\whorldhelp.pdf printable
cd printable
echo y | C:\Chris\tools\fsr whorld~1.htm ../.. ../
del fsr.tmp
cd ..
cd ..
rem rd /s /q Help
rem md Help
rem xcopy /s /y D:\Temp\MyProjects\Whorld\web\Help\*.* Help
if errorlevel 1 goto err
ren links.html links.htm
echo y | C:\Chris\tools\fsr links.htm "\"http://chordease.sourceforge.net/\"" "\"https://victimofleisure.github.io/ChordEase/\""
echo y | C:\Chris\tools\fsr links.htm "\"http://ffrend.sourceforge.net/\"" "\"https://victimofleisure.github.io/FFRend/\""
echo y | C:\Chris\tools\fsr links.htm "\"http://fractice.sourceforge.net/\"" "\"https://victimofleisure.github.io/Fractice/\""
echo y | C:\Chris\tools\fsr links.htm "\"http://mixere.sourceforge.net/\"" "\"https://victimofleisure.github.io/Mixere/\""
echo y | C:\Chris\tools\fsr links.htm "\"https://polymeter.sourceforge.io/\"" "\"https://victimofleisure.github.io/Polymeter/\""
echo y | C:\Chris\tools\fsr links.htm "\"http://potterdraw.sourceforge.net/\"" "\"https://victimofleisure.github.io/PotterDraw/\""
echo y | C:\Chris\tools\fsr links.htm "\"http://sourceforge.net/projects/triplight/\"" "\"https://github.com/victimofleisure/TripLight/\""
echo y | C:\Chris\tools\fsr links.htm "\"http://waveshop.sourceforge.net/\"" "\"https://victimofleisure.github.io/WaveShop/\""
echo y | C:\Chris\tools\fsr links.htm "\"http://whorld.sourceforge.net/\"" "\"https://victimofleisure.github.io/Whorld/\""
echo y | C:\Chris\tools\fsr links.htm "\"http://whorld.org/\"" "\"https://victimofleisure.github.io/Whorld/\""
ren links.htm links.html
del fsr.tmp
goto exit
:err
pause Error!
:exit
