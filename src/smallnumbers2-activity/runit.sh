#!/bin/sh
path=`dirname $0`

activity=smallnumbers2
type=smallnumbers

plugindir=$path
if [ -f $path/../smallnumbers-activity/.libs/lib$type.so ]
then
  plugindir=$path/../smallnumbers-activity/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $path/../smallnumbers-activity/resources \
    --config-dir=. -M $path/activity \
    --locale_dir=$path/locale -l /math/numeration/$activity $*

