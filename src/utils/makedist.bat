@echo off

REM Make sure we are in the src directory.
pushd "%~dp0\.."

if not exist utils goto wrongdir
if ERRORLEVEL 1 goto error

if "%1" == "help" goto help
if "%1" == "-help" goto help
if "%1" == "--help" goto help
goto build

:help
echo usage: makedist.bat [-v version|-bump]
echo  -bump: increment version number and tag mercurial repository
goto done

REM ------------------------------------------------------------------
REM Build settings
:build
echo setting build settings...
set PROJECT_NAME=neurolab_all
set SHADOW_DIR=..\build-release
set DISTRIB_DIR=..\distrib

set QT_DIST_DIR=C:\QtSDK\Desktop\Qt\4.7.4\mingw
set QTDIR=%QT_DIST_DIR%
set MINGW_DIR=C:\QtSDK\mingw
set QMAKESPEC=win32-g++

echo determining qt path...
qmake --version
if ERRORLEVEL 1 call :setpath
echo done setting build settings.

REM ------------------------------------------------------------------
REM Clean build
call :distclean
if ERRORLEVEL 1 goto :EOF

REM ------------------------------------------------------------------
REM Update
REM echo updating...
REM hg pull -u
REM if ERRORLEVEL 1 goto error

REM ------------------------------------------------------------------
REM Build Tools
echo building incversion...
call :build utils\incversion incversion.pro
if ERRORLEVEL 1 goto :EOF

REM ------------------------------------------------------------------
REM Increment version if necessary
echo incrementing version if necessary...
call :bumpdlls
if "%1" == "-bump" call :bump
if ERRORLEVEL 1 goto :EOF

REM ------------------------------------------------------------------
REM get version
echo getting version...
if "%1" == "-v" goto manualversion
for /f usebackq %%v in (`utils\incversion\release\incversion.exe -nobump version.txt`) do set NEUROLAB_VERSION=%%v
if ERRORLEVEL 1 goto error
goto doneversion
:manualversion
set NEUROLAB_VERSION=%2
:doneversion
echo version is %NEUROLAB_VERSION%

REM ------------------------------------------------------------------
REM hg id
echo getting hg id...
for /f usebackq %%i in (`hg -q id`) do set HG_ID=%%i
if ERRORLEVEL 1 goto error
echo hg id is %HG_ID%

REM ------------------------------------------------------------------
REM build NeuroLab
echo build neurolab...
if not exist "%SHADOW_DIR%" mkdir "%SHADOW_DIR%"
call :build "%SHADOW_DIR%" ..\src\neurolab_all.pro
if ERRORLEVEL 1 goto :EOF

REM ------------------------------------------------------------------
REM create release directory
echo create release directory...
if not exist "%DISTRIB_DIR%\zips" mkdir "%DISTRIB_DIR%\zips"
set RELEASE_DIR=%DISTRIB_DIR%\%NEUROLAB_VERSION%-%HG_ID%

echo creating release directory %RELEASE_DIR%
if exist %RELEASE_DIR% rmdir /q /s %RELEASE_DIR%
if ERRORLEVEL 1 goto error

echo creating other directories...
mkdir %RELEASE_DIR%
mkdir %RELEASE_DIR%\plugins
mkdir %RELEASE_DIR%\licenses\qt
mkdir %RELEASE_DIR%\licenses\qtpropertybrowser
mkdir %RELEASE_DIR%\samples
if ERRORLEVEL 1 goto error

REM ------------------------------------------------------------------
REM copy files
echo copying licenses...
call :copyfile "%QT_DIST_DIR%\qt\LICENSE.LGPL" %RELEASE_DIR%\licenses\qt
call :copyfile thirdparty\qtpropertybrowser\qtpropertybrowser-2.5_1-opensource\LICENSE.LGPL %RELEASE_DIR%\licenses\qtpropertybrowser
call :copyfile ..\LICENSE.txt %RELEASE_DIR%
call :copyfile ..\README.txt %RELEASE_DIR%
call :copyfile ..\doc\manual.pdf %RELEASE_DIR%\NeuroLab_UserManual.pdf
if ERRORLEVEL 1 goto :EOF

echo copying samples...
xcopy /y ..\samples\*.nln %RELEASE_DIR%\samples 
xcopy /y ..\samples\*.nnn %RELEASE_DIR%\samples 
if ERRORLEVEL 1 goto :EOF

echo copying plugins...
call :copyfile %SHADOW_DIR%\release\plugins\*.dll %RELEASE_DIR%\plugins

echo copying libraries...
call :copyfile "%MINGW_DIR%\bin\libgcc_s_dw2-1.dll" %RELEASE_DIR%
call :copyfile "%MINGW_DIR%\bin\mingwm10.dll" %RELEASE_DIR%
call :copyfile "%QT_DIST_DIR%\bin\qtcore4.dll" %RELEASE_DIR%
call :copyfile "%QT_DIST_DIR%\bin\qtgui4.dll" %RELEASE_DIR%
call :copyfile "%QT_DIST_DIR%\bin\qtsvg4.dll" %RELEASE_DIR%
call :copyfile "%QT_DIST_DIR%\bin\qtopengl4.dll" %RELEASE_DIR%
if ERRORLEVEL 1 goto :EOF

echo copying program...
call :copyfile %SHADOW_DIR%\release\qtpropertybrowser?.dll %RELEASE_DIR%
call :copyfile %SHADOW_DIR%\release\common?.dll %RELEASE_DIR%
call :copyfile %SHADOW_DIR%\release\automata?.dll %RELEASE_DIR%
call :copyfile %SHADOW_DIR%\release\neurolib?.dll %RELEASE_DIR%
call :copyfile %SHADOW_DIR%\release\neurogui?.dll %RELEASE_DIR%
call :copyfile %SHADOW_DIR%\release\neurolab.exe %RELEASE_DIR%
if ERRORLEVEL 1 goto :EOF

REM ------------------------------------------------------------------
REM make zip file
call :makezip %NEUROLAB_VERSION% %HG_ID%
if ERRORLEVEL 1 goto :EOF

REM ------------------------------------------------------------------
REM Done
:done
popd
goto :EOF

REM ------------------------------------------------------------------
REM functions

:setpath
pushd .
echo adding %QT_DIST_DIR%\bin and %MINGW_DIR%\bin to path
set PATH=%QT_DIST_DIR%\bin;%MINGW_DIR%\bin;%PATH%
set ERRORLEVEL=0
popd
goto :EOF

REM ----------------------------
:distclean
echo cleaning...
pushd .
call utils\distclean.bat
if ERRORLEVEL 1 goto error
popd
echo done cleaning
goto :EOF

REM ----------------------------
:makezip
pushd "%DISTRIB_DIR%\%1-%2"
set ZIPFILE=..\zips\neurolab-%1-%2-win32.zip
echo deleting existing zip
if exist "%ZIPFILE%" del /q "%ZIPFILE%"
echo making zip %ZIPFILE%
..\..\src\thirdparty\zip-3.0-bin\bin\zip -r "%ZIPFILE%" * 
if ERRORLEVEL 1 goto error
echo made %ZIPFILE%
popd
goto :EOF

REM ----------------------------
:copyfile
pushd .
REM echo copy /y "%1" "%2"
copy /y "%1" "%2" 
if ERRORLEVEL 1 goto error
popd
goto :EOF

REM ----------------------------
:bumpdlls
pushd .
copy /y "%MINGW_DIR%\bin\libgcc_s_dw2-1.dll" utils\incversion\release 
if ERRORLEVEL 1 goto error
copy /y "%MINGW_DIR%\bin\mingwm10.dll" utils\incversion\release 
if ERRORLEVEL 1 goto error
copy /y "%QT_DIST_DIR%\bin\qtcore4.dll" utils\incversion\release 
if ERRORLEVEL 1 goto error
popd
goto :EOF

REM ----------------------------
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

REM ----------------------------
:build
pushd .
call :qmake %1 %2
if ERRORLEVEL 1 goto error
call :make %1
if ERRORLEVEL 1 goto error
popd
echo done build
goto :EOF

:qmake
REM ----------------------------
echo running qmake in %1...
pushd "%1"
"%QT_DIST_DIR%\bin\qmake.exe" "%2" -spec %QMAKESPEC% -r CONFIG-=debug CONFIG+=release
if ERRORLEVEL 1 goto error
popd
goto :EOF

REM ----------------------------
:make
echo running make in %1...
pushd "%1"
"%MINGW_DIR%\bin\mingw32-make.exe"
if ERRORLEVEL 1 goto error
popd
goto :EOF

REM ----------------------------
:wrongdir
pushd .
echo You must be in the src directory to run the makedist.bat script!
goto error

REM ----------------------------
:error
popd
cd %OLDDIR%
echo Aborted!
exit /b 1
goto :EOF
