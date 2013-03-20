@echo off

SET CURDIRPATH="%CD%"

IF EXIST Deploy rd Deploy /Q /S

call :IMPL "%CD%"
rem call :IMPL "..\"
rem call :IMPL "Launcher\"


rem IF EXIST "%CURDIRPATH%\Launcher\app" rd "%CURDIRPATH%\Launcher\app" /Q /S

cd %CURDIRPATH%

goto :EOF

:IMPL

cd %CURDIRPATH%
cd %1

IF EXIST Debug rd Debug /Q /S
IF EXIST Release rd Release /Q /S
IF EXIST Shipping rd Shipping /Q /S
IF EXIST GeneratedFiles rd GeneratedFiles /Q /S
IF EXIST debug_qt rd debug_qt /Q /S
IF EXIST release_qt rd release_qt /Q /S
for /f "tokens=* delims= " %%a in ('"dir/b/ad | find "-build-desktop-Qt""') do rd "%%a" /Q /S
IF EXIST *.ncb del *.ncb
IF EXIST *.aps del *.aps
IF EXIST *.vcb del *.vcb
IF EXIST *.VCL del *.VCL
IF EXIST *.VCO del *.VCO
IF EXIST *.i del *.i
IF EXIST *.vspscc del *.vspscc
IF EXIST *.user del *.user
IF EXIST *.suo del /A:H *.suo

goto :EOF

