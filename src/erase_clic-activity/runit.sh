#!/bin/sh
path=`dirname $0`
resources=$path/../erase-activity/resources

activity=erase_clic
type=erase

plugindir=$path
if [ -f $path/../erase-activity/.libs/lib$type.so ]
then
  plugindir=$path/../erase-activity/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /computer/mouse/$activity $*

