activity=clickanddraw

plugindir=.
if [ -f ../boards/.libs/libpython.so ]
then
  plugindir=../boards/.libs
fi

./gcompris.bin -L $plugindir -P ../drawnumber-activity \
    -A ../drawnumber-activity/resources \
    --config-dir=. -M activity \
    --locale_dir=./locale -l /computer/mouse/$activity $*

