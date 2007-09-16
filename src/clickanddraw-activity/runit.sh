#!/bin/sh
path=`dirname $0`
resources=$path/../drawnumber-activity/resources

activity=clickanddraw

plugindir=$path
if [ -f $path/../boards/.libs/libpython.so ]
then
  plugindir=$path/../boards/.libs
fi

$path/gcompris.bin -L $plugindir -P $path./drawnumber-activity \
    -A $resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /computer/mouse/$activity $*

