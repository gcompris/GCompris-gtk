activity=scalesboard
type=scale

plugindir=.
if [ -f .libs/lib$type.so ]
then
  plugindir=.libs
fi

./gcompris.bin -L $plugindir -P . -A resources \
    --config-dir=. -M activity \
    --locale_dir=./locale -l /math/algebramenu/$activity $*

