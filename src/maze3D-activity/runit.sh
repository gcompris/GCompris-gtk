activity=maze3D

plugindir=.
if [ -f .libs/lib$activity.so ]
then
  plugindir=.libs
fi

./gcompris.bin -L $plugindir -P . -A resources \
    --config-dir=. -M activity \
    --locale_dir=./locale -l /discovery/mazeMenu/$activity $*

