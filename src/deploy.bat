%echo off

REM ---------------------
REM Build settings

set QTDIR=C:\Qt\2010.02.1

REM ---------------------
REM Clean build

call :distclean
if ERRORLEVEL 1 goto :EOF

REM ---------------------
REM Update

echo updating...
hg -q update
if ERRORLEVEL 1 goto :EOF

REM ---------------------
REM Build Tools

call :build utils\incversion incversion.pro
if ERRORLEVEL 1 goto :EOF

REM ---------------------
REM Increment version if necessary
call :bumpdlls
if "%1" == "-bump" call :bump
if ERRORLEVEL 1 goto :EOF

REM ---------------------
REM get version
for /f usebackq %%v in (`utils\incversion\release\incversion.exe -nobump version.txt`) do set NEUROLAB_VERSION=%%v
if ERRORLEVEL 1 goto :EOF
echo version is %NEUROLAB_VERSION%

goto :EOF

REM ---------------------
REM Build NeuroLab
call :build . neurolab_all.pro

REM ---------------------
REM Done
goto :EOF

REM ---------------------
REM functions

:distclean
pushd .
call distclean.bat
if ERRORLEVEL 1 goto error
popd
goto :EOF

:bumpdlls
pushd .
copy /y "%QTDIR%\mingw\bin\libgcc_s_dw2-1.dll" utils\incversion\release > "%TEMP%\deploy.log"
if ERRORLEVEL 1 goto error
copy /y "%QTDIR%\mingw\bin\mingwm10.dll" utils\incversion\release > "%TEMP%\deploy.log"
if ERRORLEVEL 1 goto error
copy /y "%QTDIR%\qt\bin\qtcore4.dll" utils\incversion\release > "%TEMP%\deploy.log"
if ERRORLEVEL 1 goto error
popd
goto :EOF

:bump
echo incrementing version...
pushd .
for /f usebackq %%v in (`utils\incversion\release\incversion.exe version.txt`) do set HG_TAG=%%v
hg tag %%v
hg commit -m "tagged version %%v"
if ERRORLEVEL 1 goto error
popd
goto :EOF

:build
call :qmake %1 %2
call :make %1
goto :EOF

:qmake
echo running qmake in %1...
pushd "%1"
"%QTDIR%\qt\bin\qmake.exe" "%2" -spec win32-g++ -r CONFIG+=release
if ERRORLEVEL 1 goto error
popd
goto :EOF

:make
echo running make in %1...
pushd "%1"
"%QTDIR%\mingw\bin\mingw32-make.exe" --no-print-directory
if ERRORLEVEL 1 goto error
popd
goto :EOF

:error
popd
echo aborted!
exit /b 1
goto :EOF
