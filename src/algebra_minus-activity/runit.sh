activity=algebra_minus
type=algebra

plugindir=.
if [ -f ../algebra_by-activity/.libs/lib$type.so ]
then
  plugindir=../algebra_by-activity/.libs
fi

./gcompris.bin -L $plugindir -P . -A ../algebra_by-activity/resources \
    --config-dir=. -M activity \
    --locale_dir=./locale -l /math/algebramenu/algebra_group/$activity $*

