#!/bin/sh
path=`dirname $0`

activity=clickanddraw

plugindir=$path
if [ -f $path/../boards/.libs/libpython.so ]
then
  plugindir=$path/../boards/.libs
fi

$path/gcompris.bin -L $plugindir -P $path./drawnumber-activity \
    -A $path/../drawnumber-activity/resources \
    --config-dir=. -M $path/activity \
    --locale_dir=$path/locale -l /computer/mouse/$activity $*

