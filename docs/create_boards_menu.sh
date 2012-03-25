#!/bin/sh

LC_ALL=C

mkdir -p boards
rm -f boards/*

for m in ../boards/*.xml.in ../src/*/*.xml.in
do
  menu=`basename $m .in`
  /usr/bin/intltool-merge -x -u -c ../po/.intltool-merge-cache ../po $m boards/$menu
done

for f in $(grep -l 'section="/experimental"' boards/*)
do
  echo removing experimental $f
  rm -f $f
done
rm boards/tuxpaint.xml
