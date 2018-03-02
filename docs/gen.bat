@echo off
echo y | rd /s C:\Chris\MyProjects\Whorld\web\Help
ren helpframe.html helpframe.txt
attrib +r gallery\*.* /s
attrib +r temp\*.* /s
navgen template.html .
if errorlevel 1 goto err
"C:\Chris\MyProjects\FixSelfUrl\Release\FixSelfUrl" *.html
if errorlevel 1 goto err
attrib -r gallery\*.* /s
attrib -r temp\*.* /s
cd gallery
navgen template.html .
if errorlevel 1 goto err
"C:\Chris\MyProjects\FixSelfUrl\Release\FixSelfUrl" *.html
if errorlevel 1 goto err
cd..
if errorlevel 1 goto err
ren helpframe.txt helpframe.html
md Help
C:\Chris\MyProjects\doc2web\release\doc2web /nospaces C:\Chris\MyProjects\Whorld\Help\help.txt Help Contents.htm C:\Chris\MyProjects\Whorld\doc\WhorldHelp.htm "Whorld Help"
if errorlevel 1 goto err
cd Help
md images
copy C:\Chris\MyProjects\Whorld\Help\images\*.* images
copy ..\helptopic.css content.css
navgen C:\Chris\MyProjects\Whorld\Help\template.txt .
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
echo y|fsr whorld~1.htm ../.. ../
del fsr.tmp
cd ..
goto exit
:err
pause Error!
:exit
