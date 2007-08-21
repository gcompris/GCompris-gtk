#!/bin/sh
path=`dirname $0`

activity=money_cents
type=money

plugindir=$path
if [ -f $path/../money-activity/.libs/lib$type.so ]
then
  plugindir=$path/../money-activity/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $path/../money-activity/resources \
    --config-dir=. -M $path/activity \
    --locale_dir=$path/locale -l /math/numeration/$activity $*

