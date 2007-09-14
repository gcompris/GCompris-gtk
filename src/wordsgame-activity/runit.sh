#!/bin/sh
path=`dirname $0`

activity=wordsgame

plugindir=$path
if [ -f $path/.libs/lib$activity.so ]
then
  plugindir=$path/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $path/../readingh-activity/resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /computer/keyboard/$activity $*

