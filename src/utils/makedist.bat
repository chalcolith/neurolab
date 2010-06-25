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
set PROJECT_NAME=neurolab_all
set SHADOW_DIR=..\%PROJECT_NAME%-build-desktop

set QT_DIST_DIR=C:\Qt\2010.02.1
set QTDIR=%QT_DIST_DIR%\qt
set QMAKESPEC=win32-g++

echo determining qt path
qmake --version
if ERRORLEVEL 1 call :setpath

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

if not exist "%SHADOW_DIR%" mkdir "%SHADOW_DIR%"
call :build "%SHADOW_DIR%" ..\src\neurolab_all.pro
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

call :copyfile "%QT_DIST_DIR%\qt\LICENSE.LGPL" %RELEASE_DIR%\licenses\qt
call :copyfile thirdparty\qtpropertybrowser\qtpropertybrowser-2.5_1-opensource\LICENSE.LGPL %RELEASE_DIR%\licenses\qtpropertybrowser
call :copyfile ..\LICENSE.txt %RELEASE_DIR%
call :copyfile ..\README.txt %RELEASE_DIR%
if ERRORLEVEL 1 goto :EOF

call :copyfile "%QT_DIST_DIR%\mingw\bin\libgcc_s_dw2-1.dll" %RELEASE_DIR%
call :copyfile "%QT_DIST_DIR%\mingw\bin\mingwm10.dll" %RELEASE_DIR%
call :copyfile "%QT_DIST_DIR%\qt\bin\qtcore4.dll" %RELEASE_DIR%
call :copyfile "%QT_DIST_DIR%\qt\bin\qtgui4.dll" %RELEASE_DIR%
call :copyfile "%QT_DIST_DIR%\qt\bin\qtsvg4.dll" %RELEASE_DIR%
if ERRORLEVEL 1 goto :EOF

call :copyfile %SHADOW_DIR%\release\qtpropertybrowser?.dll %RELEASE_DIR%
call :copyfile %SHADOW_DIR%\release\automata?.dll %RELEASE_DIR%
call :copyfile %SHADOW_DIR%\release\neurolib?.dll %RELEASE_DIR%
call :copyfile %SHADOW_DIR%\release\neurogui?.dll %RELEASE_DIR%
call :copyfile %SHADOW_DIR%\release\neurolab.exe %RELEASE_DIR%
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
echo adding %QT_DIST_DIR%\qt\bin and %QT_DIST_DIR%\mingw\bin to path
set PATH=%QT_DIST_DIR%\qt\bin;%QT_DIST_DIR%\mingw\bin;%PATH%
set ERRORLEVEL=0
popd
goto :EOF

:distclean
echo cleaning...
pushd .
call utils\distclean.bat
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
copy /y "%QT_DIST_DIR%\mingw\bin\libgcc_s_dw2-1.dll" utils\incversion\release > "%TEMP%\deploy.log"
if ERRORLEVEL 1 goto error
copy /y "%QT_DIST_DIR%\mingw\bin\mingwm10.dll" utils\incversion\release > "%TEMP%\deploy.log"
if ERRORLEVEL 1 goto error
copy /y "%QT_DIST_DIR%\qt\bin\qtcore4.dll" utils\incversion\release > "%TEMP%\deploy.log"
if ERRORLEVEL 1 goto error
popd
goto :EOF

:bump
echo incrementing version...
pushd .
for /f usebackq %%v in (`utils\incversion\release\incversion.exe version.txt`) do set HG_TAG=%%v
if ERRORLEVEL 1 goto error
hg commit -m "makedist: incremented version.txt to %HG_TAG%"
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
"%QT_DIST_DIR%\qt\bin\qmake.exe" "%2" -spec %QMAKESPEC% -r CONFIG-=debug CONFIG+=release
if ERRORLEVEL 1 goto error
popd
goto :EOF

:make
echo running make in %1...
pushd "%1"
"%QT_DIST_DIR%\mingw\bin\mingw32-make.exe" --no-print-directory
if ERRORLEVEL 1 goto error
popd
goto :EOF

:error
popd
echo aborted!
exit /b 1
goto :EOF
