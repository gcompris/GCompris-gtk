activity=searace1player

plugindir=.
if [ -f ../boards/.libs/libpython.so ]
then
  plugindir=../boards/.libs
fi

./gcompris.bin -L $plugindir -P . -A resources \
    --config-dir=. -M activity \
    --locale_dir=./locale -l /experience/$activity $*
