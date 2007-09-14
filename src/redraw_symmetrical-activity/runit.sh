#!/bin/sh
path=`dirname $0`

activity=redraw_symmetrical

plugindir=$path
if [ -f $path/../boards/.libs/libpython.so ]
then
  plugindir=$path/../boards/.libs
fi

$path/gcompris.bin -L $plugindir -P $path./redraw-activity -A $path/resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /math/geometry/$activity $*

