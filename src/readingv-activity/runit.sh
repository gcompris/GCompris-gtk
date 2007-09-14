#!/bin/sh
path=`dirname $0`

activity=readingv
type=reading

plugindir=$path
if [ -f $path/../readingh-activity/.libs/lib$type.so ]
then
  plugindir=$path/../readingh-activity/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $path/../readingh-activity/resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /reading/$activity $*

