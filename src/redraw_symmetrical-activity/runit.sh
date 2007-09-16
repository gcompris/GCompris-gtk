#!/bin/sh
path=`dirname $0`
resources=$path/resources

activity=redraw_symmetrical

plugindir=$path
if [ -f $path/../boards/.libs/libpython.so ]
then
  plugindir=$path/../boards/.libs
fi

$path/gcompris.bin -L $plugindir -P $path./redraw-activity -A $resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /math/geometry/$activity $*

