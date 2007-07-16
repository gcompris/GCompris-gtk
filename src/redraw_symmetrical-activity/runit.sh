activity=redraw_symmetrical

plugindir=.
if [ -f ../boards/.libs/libpython.so ]
then
  plugindir=../boards/.libs
fi

./gcompris.bin -L $plugindir -P ../redraw-activity -A resources \
    --config-dir=. -M activity \
    --locale_dir=./locale -l /math/geometry/$activity $*

