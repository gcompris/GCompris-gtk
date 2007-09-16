#!/bin/sh
path=`dirname $0`
resources=$path/../maze-activity/resources

activity=mazeInvisible
type=maze

plugindir=$path
if [ -f $path/../maze-activity/.libs/lib$type.so ]
then
  plugindir=$path/../maze-activity/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /discovery/mazeMenu/$activity $*

