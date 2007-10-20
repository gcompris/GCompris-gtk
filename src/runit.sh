#!/bin/sh
#set -x
if [ -z $1 ]; then
  echo "Usage: runit.sh [GCompris activity directory]"
  exit 1
fi

path=`dirname $0`
. $1/init_path.sh

menudir=$path
localedir=$path/locale

if [ ! -f $localedir ]; then
  localedir=/usr/share/locale
fi

if [ ! -d $plugindir ]; then
  plugindir=$path
fi

gcompris=$1/gcompris.bin
if [ ! -f $gcompris ]; then
  gcompris=$1/../gcompris/gcompris
fi

$gcompris -L $plugindir \
    -P $pythonplugindir \
    -A $resourcedir \
    -M $menudir \
    --locale_dir=$path/locale \
    -l $section/$activity $*

