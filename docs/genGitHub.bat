@echo off
echo y | rd /s C:\Chris\MyProjects\Whorld\web\Help
ren helpframe.html helpframe.txt
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
if errorlevel 1 goto err
ren helpframe.txt helpframe.html
md Help
C:\Chris\MyProjects\doc2web\release\doc2web /nospaces C:\Chris\MyProjects\Whorld\Help\help.txt Help Contents.htm C:\Chris\MyProjects\Whorld\doc\WhorldHelp.htm "Whorld Help"
if errorlevel 1 goto err
cd Help
md images
copy C:\Chris\MyProjects\Whorld\Help\images\*.* images
copy ..\helptopic.css content.css
C:\Chris\tools\navgen C:\Chris\MyProjects\Whorld\Help\template.txt .
copy ..\helpheader.txt x
copy x + Contents.htm
echo ^<body^>^<html^> >>x
del Contents.htm
ren x Contents.htm
md printable
move C:\Chris\MyProjects\Whorld\doc\whorldhelp.htm printable
copy C:\Chris\MyProjects\Whorld\doc\whorldhelp.doc printable
copy C:\Chris\MyProjects\Whorld\doc\whorldhelp.pdf printable
cd printable
echo y | C:\Chris\tools\fsr whorld~1.htm ../.. ../
del fsr.tmp
cd ..
goto exit
:err
pause Error!
:exit
