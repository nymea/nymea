#!/bin/bash

echo generating defines... $@
echo \#include \<QMetaType\> > testdefines.h
for i in $@; do
  echo \#include \"${i,,}.h\" >> testdefines.h
done

echo >> testdefines.h

echo \#define REGISTER_METATYPES \\ >> testdefines.h

for i in $@; do
  echo qRegisterMetaType\<$i\>\(\"$i\"\)\; \\ >> testdefines.h
done

echo >> testdefines.h

echo \#define TESTCASES \"$@\" >> testdefines.h
