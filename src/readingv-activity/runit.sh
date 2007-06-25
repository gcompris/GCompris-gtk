activity=readingv
type=reading

plugindir=.
if [ -f ../readingh-activity/.libs/lib$type.so ]
then
  plugindir=../readingh-activity/.libs
fi

./gcompris.bin -L $plugindir -P . -A ../readingh-activity/resources \
    --config-dir=. -M activity \
    --locale_dir=./locale -l /reading/$activity $*

