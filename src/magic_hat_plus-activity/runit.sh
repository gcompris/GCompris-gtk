#!/bin/sh
path=`dirname $0`

activity=magic_hat_plus
type=magic_hat

plugindir=$path
if [ -f $path/../magic_hat_minus-activity/.libs/lib$type.so ]
then
  plugindir=$path/../magic_hat_minus-activity/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $path/../magic_hat_minus-activity/resources \
    --config-dir=. -M $path/activity \
    --locale_dir=$path/locale -l /math/numeration/$activity $*

