#!/bin/sh
path=`dirname $0`
resources=$path/../memory-activity/resources

activity=memory_add
type=memory

plugindir=$path
if [ -f $path/../memory-activity/.libs/lib$type.so ]
then
  plugindir=$path/../memory-activity/.libs
fi

$path/gcompris.bin -L $plugindir -P $path -A $resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /math/algebramenu/memory_op_group/$activity $*

