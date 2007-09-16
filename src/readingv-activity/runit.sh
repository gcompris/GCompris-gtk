#!/bin/sh
path=`dirname $0`
resources=$path/../readingh-activity/resources

activity=readingv
type=reading

plugindir=$path
if [ -f $path/../readingh-activity/.libs/lib$type.so ]
then
  plugindir=$path/../readingh-activity/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /reading/$activity $*

