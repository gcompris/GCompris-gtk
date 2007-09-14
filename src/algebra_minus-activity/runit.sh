#!/bin/sh
path=`dirname $0`

activity=algebra_minus
type=algebra

plugindir=$path
if [ -f $path/../algebra_by-activity/.libs/lib$type.so ]
then
  plugindir=$path/../algebra_by-activity/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $path/../algebra_by-activity/resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /math/algebramenu/algebra_group/$activity $*

