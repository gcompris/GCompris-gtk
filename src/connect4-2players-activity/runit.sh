#!/bin/sh
path=`dirname $0`

activity=connect4-2players

plugindir=$path
if [ -f $path/../boards/.libs/libpython.so ]
then
  plugindir=$path/../boards/.libs
fi

pythonplugindir=.
resources=resources
if [ -f ../connect4-activity/connect4.py ]
then
  pythonplugindir=../connect4-activity/
  resources=../connect4-activity/resources
fi

$path/gcompris.bin -L $plugindir -P $pathpythonplugindir -A $path/$resources \
    --config-dir=. -M $path \
    --locale_dir=$path/locale -l /strategy/$activity $*

