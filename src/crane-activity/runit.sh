#!/bin/sh
path=`dirname $0`

activity=crane

plugindir=$path
if [ -f $path/.libs/lib$activity.so ]
then
  plugindir=$path/.libs
fi

$path/gcompris.bin -L $plugindir -P . -A $path/resources \
    --config-dir=. -M $path/activity \
    --locale_dir=./locale -l /puzzle/$activity $*

