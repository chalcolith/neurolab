@echo off

if exist distclean.bat cd ..

if not exist Makefile echo > Makefile
del /s /q Makefile* > "%TEMP%\distclean.log"

rem if not exist neurolab_all.user echo > neurolab_all.user
rem del /s /q *.user > "%TEMP%\distclean.log"

for /d %%d in (..\*build*) do rmdir /s /q %%d > "%TEMP%\distclean.log"

if not exist debug mkdir debug
for /f usebackq %%d in (`dir /s /b *debug*`) do rmdir /s /q "%%d"

if not exist release mkdir release
for /f usebackq %%d in (`dir /s /b *release*`) do rmdir /s /q "%%d"

echo > dummy.o
del /s /q *.o > "%TEMP%\distclean.log"

echo > moc_dummy
del /s /q moc_* > "%TEMP%\distclean.log"

echo > uidummy.h
del /s /q ui*.h > "%TEMP%\distclean.log"

echo > object_scriptdummy
del /s /q object_script* > "%TEMP%\distclean.log"
