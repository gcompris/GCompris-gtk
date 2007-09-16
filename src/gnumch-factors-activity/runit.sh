#!/bin/sh
path=`dirname $0`
resources=$path/../gnumch-equality-activity/resources

activity=gnumch-factors

plugindir=$path
if [ -f $path/../boards/.libs/libpython.so ]
then
  plugindir=$path/../boards/.libs
fi

$path/gcompris.bin -L $plugindir -P $path./gnumch-equality-activity \
    -A $resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /math/algebramenu/gnumchmenu/$activity $*

