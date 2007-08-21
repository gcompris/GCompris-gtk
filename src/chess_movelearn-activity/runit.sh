#!/bin/sh
path=`dirname $0`

activity=chess_movelearn
type=chess

plugindir=$path
if [ -f $path/../chess_computer-activity/.libs/lib$type.so ]
then
  plugindir=$path/../chess_computer-activity/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $path/../chess_computer-activity/resources \
    --config-dir=. -M $path/activity \
    --locale_dir=$path/locale -l /strategy/chess/$activity $*

