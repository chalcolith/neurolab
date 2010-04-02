%echo off

set QTDIR=C:\Qt\2010.02.1

call :distclean
if ERRORLEVEL 1 goto :EOF

call :build utils\incversion incversion.pro
if ERRORLEVEL 1 goto :EOF

call :build . neurolab_all.pro

goto :EOF

:distclean
pushd .
call distclean.bat
if ERRORLEVEL 1 goto error
popd
goto :EOF

:build
call :qmake %1 %2
call :make %1
goto :EOF

:qmake
echo running qmake in %1
pushd "%1"
"%QTDIR%\qt\bin\qmake.exe" "%2" -spec win32-g++ -r CONFIG+=release
if ERRORLEVEL 1 goto error
popd
goto :EOF

:make
echo running make in %1
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
