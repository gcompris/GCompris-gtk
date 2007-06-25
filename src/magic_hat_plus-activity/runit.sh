activity=magic_hat_plus
type=magic_hat

plugindir=.
if [ -f ../magic_hat_minus-activity/.libs/lib$type.so ]
then
  plugindir=../magic_hat_minus-activity/.libs
fi

./gcompris.bin -L $plugindir -P . -A ../magic_hat_minus-activity/resources \
    --config-dir=. -M activity \
    --locale_dir=./locale -l /math/numeration/$activity $*

