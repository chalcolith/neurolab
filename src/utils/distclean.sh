#!/bin/sh
if [ ! -d utils ]; then cd ..; fi

rm -rf `find . -name 'Makefile*'`
#rm -rf `find . -name '*.user'`
rm -rf `find . -name 'debug'`
rm -rf `find . -name 'release'`
rm -rf `find . -name '*.o'`

rm -rf ../*-build-*
