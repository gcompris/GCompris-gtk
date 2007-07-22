#!/bin/sh
#
# A far from complete script that can create a binary tarball
# for the given activity.
#
if test -z "$1"; then
    echo "Usage: ./bundleit.sh directory-activity"
    exit 1
fi

if test "$1" != "draw-activity" && test "$1" != "anim-activity"; then
  draw="--exclude resources/skins/gartoon/draw"
else
  draw=""
fi

py=`ls $1/*.py 2>/dev/null`
if test "$py" != ""; then
  echo "Python activity not supported yet"
  exit
fi

tar -cjf $1.tar.bz2 -h \
    --exclude ".svn" --exclude "resources/skins/babytoy" \
    $draw \
    --exclude "resources/skins/gartoon/timers" \
    --exclude ".deps" \
    --exclude "Makefile*" \
    --exclude "*.c" \
    --exclude "*.py" \
    --exclude "*.la" \
    --exclude "*.lo" \
    --exclude "*.o" \
    --exclude "*.lai" \
    $1
