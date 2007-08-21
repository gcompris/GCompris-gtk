#!/bin/sh
path=`dirname $0`

activity=maze2DRelative
type=maze

plugindir=$path
if [ -f $path/../maze-activity/.libs/lib$type.so ]
then
  plugindir=$path/../maze-activity/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $path/../maze-activity/resources \
    --config-dir=. -M $path/activity \
    --locale_dir=$path/locale -l /discovery/mazeMenu/$activity $*

