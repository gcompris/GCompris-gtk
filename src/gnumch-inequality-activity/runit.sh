activity=gnumch-inequality

plugindir=.
if [ -f ../boards/.libs/libpython.so ]
then
  plugindir=../boards/.libs
fi

./gcompris.bin -L $plugindir -P ../gnumch-equality-activity \
    -A ../gnumch-equality-activity/resources \
    --config-dir=. -M activity \
    --locale_dir=./locale -l /math/algebramenu/gnumchmenu/$activity $*

