activity=erase_clic
type=erase

plugindir=.
if [ -f ../erase-activity/.libs/lib$type.so ]
then
  plugindir=../erase-activity/.libs
fi

./gcompris.bin -L $plugindir -P . -A ../erase-activity/resources \
    --config-dir=. -M activity \
    --locale_dir=./locale -l /computer/mouse/$activity $*

