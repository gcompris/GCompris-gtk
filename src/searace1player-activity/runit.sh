#!/bin/sh
path=`dirname $0`

activity=searace1player

plugindir=$path
if [ -f $path/../boards/.libs/libpython.so ]
then
  plugindir=$path/../boards/.libs
fi

$path/gcompris.bin -L $plugindir -P $path./searace-activity \
    -A $path/../searace-activity/resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /experience/$activity $*

