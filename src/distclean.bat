%echo off
echo cleaning...

echo > Makefile_dummy
call :clean Makefile*

echo > dummy.user
call :clean *.user

if not exist debug mkdir debug
call :clean debug 

echo > dummy.o
call :clean *.o 

echo > moc_dummy
call :clean moc_* 

echo > uidummy.h
call :clean ui*.h 

echo > object_scriptdummy
call :clean object_script* 

goto :EOF

:clean
del /s /q %* > %TEMP%\distclean.log
goto :EOF
