#!/bin/sh
path=`dirname $0`
resources=$path/../smallnumbers-activity/resources

activity=smallnumbers2
type=smallnumbers

plugindir=$path
if [ -f $path/../smallnumbers-activity/.libs/lib$type.so ]
then
  plugindir=$path/../smallnumbers-activity/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /math/numeration/$activity $*

