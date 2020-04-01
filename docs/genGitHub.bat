@echo off
copy C:\Chris\MyProjects\Whorld\web\*.html
copy downloadGitHub.html download.html
rem echo y | rd /s C:\Chris\MyProjects\Whorld\web\Help
rem ren helpframe.html helpframe.txt
rem attrib +r gallery\*.* /s
rem attrib +r temp\*.* /s
C:\Chris\tools\navgen templateGitHub.html .
if errorlevel 1 goto err
"C:\Chris\MyProjects\FixSelfUrl\Release\FixSelfUrl" *.html
if errorlevel 1 goto err
rem attrib -r gallery\*.* /s
rem attrib -r temp\*.* /s
rem cd gallery
rem navgen template.html .
rem if errorlevel 1 goto err
rem "C:\Chris\MyProjects\FixSelfUrl\Release\FixSelfUrl" *.html
rem if errorlevel 1 goto err
rem cd..
rem if errorlevel 1 goto err
rem ren helpframe.txt helpframe.html
rem md Help
rem C:\Chris\MyProjects\doc2web\release\doc2web /nospaces C:\Chris\MyProjects\Whorld\Help\help.txt Help Contents.htm C:\Chris\MyProjects\Whorld\doc\WhorldHelp.htm "Whorld Help"
rem if errorlevel 1 goto err
rem cd Help
rem md images
rem copy C:\Chris\MyProjects\Whorld\Help\images\*.* images
rem copy ..\helptopic.css content.css
rem C:\Chris\tools\navgen C:\Chris\MyProjects\Whorld\Help\template.txt .
rem copy ..\helpheader.txt x
rem copy x + Contents.htm
rem echo ^<body^>^<html^> >>x
rem del Contents.htm
rem ren x Contents.htm
rem md printable
rem move C:\Chris\MyProjects\Whorld\doc\whorldhelp.htm printable
rem copy C:\Chris\MyProjects\Whorld\doc\whorldhelp.doc printable
rem copy C:\Chris\MyProjects\Whorld\doc\whorldhelp.pdf printable
rem cd printable
rem echo y | C:\Chris\tools\fsr whorld~1.htm ../.. ../
rem del fsr.tmp
rem cd ..
rd /s /q Help
md Help
xcopy /s /y C:\Chris\MyProjects\Whorld\web\Help\*.* Help
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
