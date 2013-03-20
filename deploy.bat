@echo off 


if "%QTDIR%" neq ""  SET QT_DIR=%QTDIR%
if "%QT_DIR%" equ "" SET QT_DIR="c:\projects\Qt\QtWin4.8.0"

SET CUR_DIR="%~dp0"
SET DEPLOY="%CUR_DIR%\Deploy"
SET CONFNAME=Release

if EXIST %DEPLOY% call :CheckError rd /S /Q %DEPLOY%

call :CheckError md %DEPLOY%

echo ##########################################################################
echo # iFlashcards
echo ##########################################################################

call :CheckError copy %CUR_DIR%\%CONFNAME%\iFlashcards.exe %DEPLOY%\iFlashcards.exe 
call :CheckError copy %QT_DIR%\bin\QtCore4.dll %DEPLOY% 
call :CheckError copy %QT_DIR%\bin\QtGui4.dll %DEPLOY%
call :CheckError copy %CUR_DIR%\settings.xml %DEPLOY%


rem The esiest way is to dowload vcredist_x86.exe
rem http://download.microsoft.com/download/9/7/7/977B481A-7BA6-4E30-AC40-ED51EB2028F2/vcredist_x86.exe
rem http://www.microsoft.com/en-us/download/details.aspx?id=11895
rem http://www.microsoft.com/downloads/details.aspx?familyid=D5692CE4-ADAD-4000-ABFE-64628A267EF0&displaylang=en

rem But wasn't successeful with more complicated way
rem http://blog.kalmbach-software.de/2008/05/03/howto-deploy-vc2008-apps-without-installing-vcredist_x86exe/
rem http://www.peeep.us/23dfadbc
rem http://stackoverflow.com/a/177097
rem http://stackoverflow.com/a/2289740
rem http://stackoverflow.com/a/8166469
rem SET CRT_DIR="%DEPLOY%\Microsoft.VC90.CRT"
rem call :CheckError md %CRT_DIR%
rem call :CheckError copy "%ProgramFiles%\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\msvcp90.dll" %CRT_DIR% 
rem call :CheckError copy "%ProgramFiles%\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\msvcr90.dll" %CRT_DIR% 
rem call :CheckError copy "%CUR_DIR%\iFlashcards.exe.manifest" %DEPLOY%  
rem call :CheckError copy "%CUR_DIR%\Microsoft.VC90.CRT.manifest" %CRT_DIR% 

echo All operations completed successfully
call :DoPause
goto :EOF


rem ##########################################################################
:DoPause
    if "%DO_NOT_PAUSE%" neq "true" pause
goto :EOF

rem ##########################################################################
:ShowError
   @echo.
   @echo ======================================================================
   @echo  ERROR: %*
   @echo ======================================================================
   @echo. 
   call :DoPause
   exit 1
goto :EOF

rem ##########################################################################
:CheckError
    %*
    if %ERRORLEVEL% neq 0 (
       call :ShowError %* 
    )
goto :EOF

rem ##########################################################################