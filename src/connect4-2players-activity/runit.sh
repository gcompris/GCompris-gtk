activity=connect4-2players

plugindir=.
if [ -f ../boards/.libs/libpython.so ]
then
  plugindir=../boards/.libs
fi

pythonplugindir=.
resources=resources
if [ -f ../connect4-activity/connect4.py ]
then
  pythonplugindir=../connect4-activity/
  resources=../connect4-activity/resources
fi

./gcompris.bin -L $plugindir -P $pythonplugindir -A $resources \
    --config-dir=. -M activity \
    --locale_dir=./locale -l /strategy/$activity $*

