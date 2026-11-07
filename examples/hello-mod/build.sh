#!/bin/sh
set -e
clang++ -shared -fPIC -O2 -fno-exceptions -fno-rtti -nostdlib++ -I../../sdk -o mod.so main.cpp
rm -f hello.rift
zip -q hello.rift mod.json mod.so
echo "jadi: hello.rift"
