#!/bin/sh

out_file=../../doc/interfacelist.qdoc
cur_dir=`pwd`
echo > $out_file
echo "\\list" >> $out_file
for i in `ls *.json`; do
  echo "\\li \l{$i}" | sed s/\.json// >> $out_file
done
echo "\\endlist" >> $out_file

for i in `ls *.json`; do
  echo "\\\target $i" | sed s/\.json// >> $out_file
  echo "\\section2 $i" | sed s/\.json// >> $out_file
  echo "\\quotefile $cur_dir/$i" >> $out_file
done
