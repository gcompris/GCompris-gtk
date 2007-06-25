activity=chess_partyend
type=chess

plugindir=.
if [ -f ../chess_computer-activity/.libs/lib$type.so ]
then
  plugindir=../chess_computer-activity/.libs
fi

./gcompris.bin -L $plugindir -P . -A ../chess_computer-activity/resources \
    --config-dir=. -M activity \
    --locale_dir=./locale -l /strategy/chess/$activity $*

