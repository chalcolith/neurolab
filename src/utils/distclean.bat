@echo off

if not exist Makefile echo > Makefile
del /s /q Makefile* > "%TEMP%\distclean.log"

rem if not exist neurolab_all.user echo > neurolab_all.user
rem del /s /q *.user > "%TEMP%\distclean.log"

if exist ..\build rmdir /s /q ..\build > "%TEMP%\distclean.log"

echo > dummy.o
del /s /q *.o > "%TEMP%\distclean.log"

echo > moc_dummy
del /s /q moc_* > "%TEMP%\distclean.log"

echo > uidummy.h
del /s /q ui*.h > "%TEMP%\distclean.log"

echo > object_scriptdummy
del /s /q object_script* > "%TEMP%\distclean.log"
