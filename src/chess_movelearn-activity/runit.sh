#!/bin/sh
path=`dirname $0`
resources=$path/../chess_computer-activity/resources

activity=chess_movelearn
type=chess

plugindir=$path
if [ -f $path/../chess_computer-activity/.libs/lib$type.so ]
then
  plugindir=$path/../chess_computer-activity/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /strategy/chess/$activity $*

