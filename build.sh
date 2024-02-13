#!/bin/bash

set -xe

gcc -o httest -ggdb main.c -lcurl
#x86_64-w64-mingw32-gcc -o httest.exe -ggdb  main.c -L/usr/include/x86_64-linux-gnu -lcurl
