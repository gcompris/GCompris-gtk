#!/bin/sh
path=`dirname $0`

activity=superbrain

plugindir=$path
if [ -f $path/.libs/lib$activity.so ]
then
  plugindir=$path/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $path/resources \
    --config-dir=. -M $path/activity \
    --locale_dir=$path/locale -l /puzzle/$activity $*

