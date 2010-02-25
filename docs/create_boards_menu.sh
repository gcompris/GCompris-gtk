#!/bin/sh

LC_ALL=C

mkdir -p boards

for m in ../boards/*.xml.in ../src/*/*.xml.in
do
  menu=`basename $m`
  /usr/bin/intltool-merge -x -u -c ../po/.intltool-merge-cache ../po $m boards/$menu
done

rm boards/pythontemplate.xml.in
rm boards/pythontest.xml.in
rm boards/tuxpaint.xml.in
