#!/bin/sh
#
# A far from complete script that can create a binary tarball
# for the given activity.
#
if test -z "$1"; then
    echo "Usage: ./bundleit.sh directory-activity"
    exit 1
fi

if test "$1" != "draw-activity" && test "$1" != "anim-activity"; then
  draw="--exclude resources/skins/gartoon/draw"
else
  draw=""
fi

py=`ls $1/*.py 2>/dev/null`
if test "$py" != ""; then
  echo "Python activity not supported yet"
  exit
fi

# Create the Sugar specific startup scripts
activity_name=`basename $1 -activity`
cp activity-gcompris.svg $1/activity
cp activity.info $1/activity
sed -i s/@ACTIVITY_NAME@/$activity_name/g $1/activity/activity.info
cp gcompris-instance $1/
cp gcompris-factory $1/

tar -cjf $1.tar.bz2 -h \
    --exclude ".svn" --exclude "resources/skins/babytoy" \
    $draw \
    --exclude "resources/skins/gartoon/timers" \
    --exclude ".deps" \
    --exclude "Makefile*" \
    --exclude "*.c" \
    --exclude "*.py" \
    --exclude "*.la" \
    --exclude "*.lo" \
    --exclude "*.o" \
    --exclude "*.lai" \
    $1

# Create the sugar .xo zip bundle
tar -tjf $1.tar.bz2 | zip $1.xo -@

# Sugar cleanup
rm $1/activity/activity.info
rm $1/activity/activity-gcompris.svg
rm $1/gcompris-instance
rm $1/gcompris-factory
