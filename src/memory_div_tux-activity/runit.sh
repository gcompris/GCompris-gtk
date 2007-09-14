#!/bin/sh
path=`dirname $0`

activity=memory_div_tux
type=memory

plugindir=$path
if [ -f $path/../memory-activity/.libs/lib$type.so ]
then
  plugindir=$path/../memory-activity/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $path/../memory-activity/resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /math/algebramenu/memory_op_group_tux/$activity $*

