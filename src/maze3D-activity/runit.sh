activity=maze3D
type=maze

plugindir=.
if [ -f ../maze-activity/.libs/lib$type.so ]
then
  plugindir=../maze-activity/.libs
fi

./gcompris.bin -L $plugindir -P . -A ../maze-activity/resources \
    --config-dir=. -M activity \
    --locale_dir=./locale -l /discovery/mazeMenu/$activity $*

