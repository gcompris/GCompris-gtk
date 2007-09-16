#!/bin/sh
path=`dirname $0`
resources=$path/../readingh-activity/resources

activity=wordsgame

plugindir=$path
if [ -f $path/.libs/lib$activity.so ]
then
  plugindir=$path/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /computer/keyboard/$activity $*

