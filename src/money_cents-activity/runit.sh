activity=money_cents
type=money

plugindir=.
if [ -f ../money-activity/.libs/lib$type.so ]
then
  plugindir=../money-activity/.libs
fi

./gcompris.bin -L $plugindir -P . -A ../money-activity/resources \
    --config-dir=. -M activity \
    --locale_dir=./locale -l /math/numeration/$activity $*

