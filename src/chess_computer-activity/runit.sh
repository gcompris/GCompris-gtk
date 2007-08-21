#!/bin/sh
path=`dirname $0`

activity=chess_computer
type=chess

plugindir=$path
if [ -f $path/.libs/lib$type.so ]
then
  plugindir=$path/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $path/resources \
    --config-dir=. -M $path/activity \
    --locale_dir=$path/locale -l /strategy/chess/$activity $*

