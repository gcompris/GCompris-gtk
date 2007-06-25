activity=smallnumbers2
type=smallnumbers

plugindir=.
if [ -f ../smallnumbers-activity/.libs/lib$type.so ]
then
  plugindir=../smallnumbers-activity/.libs
fi

./gcompris.bin -L $plugindir -P . -A ../smallnumbers-activity/resources \
    --config-dir=. -M activity \
    --locale_dir=./locale -l /math/numeration/$activity $*

