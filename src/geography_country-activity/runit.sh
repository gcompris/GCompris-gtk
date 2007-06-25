activity=geography_country
type=shapegame

plugindir=.
if [ -f ../babymatch-activity/.libs/lib$type.so ]
then
  plugindir=../babymatch-activity/.libs
fi

./gcompris.bin -L $plugindir -P . -A resources \
    --config-dir=. -M activity \
    --locale_dir=./locale -l /discovery/miscelaneous/$activity $*

