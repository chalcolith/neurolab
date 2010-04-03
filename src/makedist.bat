@echo off

if "%1" == "help" goto help
if "%1" == "-help" goto help
if "%1" == "--help" goto help
goto build

:help
echo usage: makedist.bat [-bump]
echo  -bump: increment version number and tag mercurial repository
goto :EOF

REM ---------------------
REM Build settings

:build
set QTDIR=C:\Qt\2010.02.1

g++ --version > "%TEMP%\deploy.log"
if ERRORLEVEL 1 goto setpath

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

REM ---------------------
REM hg id

for /f usebackq %%i in (`hg -q id`) do set HG_ID=%%i
echo hg id is %HG_ID%

REM ---------------------
REM build NeuroLab

call :build . neurolab_all.pro
if ERRORLEVEL 1 goto :EOF

REM ---------------------
REM create release directory

if not exist distrib\zips mkdir distrib\zips
set RELEASE_DIR=distrib\%NEUROLAB_VERSION%
if exist %RELEASE_DIR% rmdir /q /s %RELEASE_DIR%
if ERRORLEVEL 1 goto :EOF
mkdir %RELEASE_DIR%
mkdir %RELEASE_DIR%\plugins
mkdir %RELEASE_DIR%\licenses\qt
mkdir %RELEASE_DIR%\licenses\qtpropertybrowser
if ERRORLEVEL 1 goto :EOF

REM ---------------------
REM copy files

call :copyfile "%QTDIR%\qt\LICENSE.LGPL" %RELEASE_DIR%\licenses\qt
call :copyfile thirdparty\qtpropertybrowser\qtpropertybrowser-2.5_1-opensource\LICENSE.LGPL %RELEASE_DIR%\licenses\qtpropertybrowser
call :copyfile ..\LICENSE.txt %RELEASE_DIR%
if ERRORLEVEL 1 goto :EOF

call :copyfile "%QTDIR%\mingw\bin\libgcc_s_dw2-1.dll" %RELEASE_DIR%
call :copyfile "%QTDIR%\mingw\bin\mingwm10.dll" %RELEASE_DIR%
call :copyfile "%QTDIR%\qt\bin\qtcore4.dll" %RELEASE_DIR%
call :copyfile "%QTDIR%\qt\bin\qtgui4.dll" %RELEASE_DIR%
call :copyfile "%QTDIR%\qt\bin\qtsvg4.dll" %RELEASE_DIR%
if ERRORLEVEL 1 goto :EOF

call :copyfile release\qtpropertybrowser?.dll %RELEASE_DIR%
call :copyfile release\automata?.dll %RELEASE_DIR%
call :copyfile release\neurolib?.dll %RELEASE_DIR%
call :copyfile release\neurogui?.dll %RELEASE_DIR%
call :copyfile release\neurolab.exe %RELEASE_DIR%
if ERRORLEVEL 1 goto :EOF

REM ---------------------
REM make zip file
call :makezip %NEUROLAB_VERSION% %HG_ID%
if ERRORLEVEL 1 goto :EOF

REM ---------------------
REM Done
goto :EOF

REM ---------------------
REM functions

:setpath
pushd .
echo adding %QTDIR%\qt\bin and %QTDIR%\mingw\bin to path
set PATH=%QTDIR%\qt\bin;%QTDIR%\mingw\bin;%PATH%
set ERRORLEVEL=0
popd
goto :EOF

:distclean
echo cleaning...
pushd .
call distclean.bat
if ERRORLEVEL 1 goto error
popd
goto :EOF

:makezip
pushd distrib\%1
set ZIPFILE=..\zips\neurocogling-neurolab-%1-%2-win32.zip
..\..\thirdparty\zip-3.0-bin\bin\zip -r %ZIPFILE% * > %TEMP%\deploy.log
if ERRORLEVEL 1 goto error
echo created %ZIPFILE%
popd
goto :EOF

:copyfile
pushd .
REM echo copy /y "%1" "%2"
copy /y "%1" "%2" > %TEMP%\deploy.log
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
if ERRORLEVEL 1 goto error
hg commit -m "AUTOMATED DEPLOY: incremented version to %HG_TAG%"
if ERRORLEVEL 1 goto error
hg tag %HG_TAG%
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
"%QTDIR%\qt\bin\qmake.exe" "%2" -spec win32-g++ -r CONFIG-=debug CONFIG+=release
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
