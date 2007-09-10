#!/bin/sh
#
# A far from complete script that can create a binary tarball
# for the given activity.
#
if test -z "$1"; then
    echo "Usage: ./bundleit.sh directory-activity"
    exit 1
fi

if test "$1" != "draw-activity" && \
    test "$1" != "anim-activity" && \
    test "$1" != "electric-activity" ; then
  draw="--exclude resources/skins/gartoon/draw"
else
  draw=""
fi

# Create the Sugar specific startup scripts
activity_name=`basename $1 -activity`
activity_dir=${activity_name}.activity
cp -a $1 $activity_dir
cp activity-gcompris.svg $activity_dir/activity
cp activity.info $activity_dir/activity
sed -i s/@ACTIVITY_NAME@/$activity_name/g $activity_dir/activity/activity.info
cp gcompris-instance $activity_dir/
cp gcompris-factory $activity_dir/
if [ -f $activity_dir/.libs/*.so ]; then
  mv $activity_dir/.libs/*.so $activity_dir
fi
rm -rf $activity_dir/.libs

# Add the python plugin if needed
py=`ls $1/*.py 2>/dev/null`
if test "$py" != ""; then
    cp $1/../boards/.libs/libpython.so $activity_dir
fi

tar -cjf $activity_dir.tar.bz2 -h \
    --exclude ".svn" --exclude "resources/skins/babytoy" \
    $draw \
    --exclude "resources/skins/gartoon/timers" \
    --exclude ".deps" \
    --exclude "Makefile*" \
    --exclude "*.c" \
    --exclude "*.la" \
    --exclude "*.lo" \
    --exclude "*.o" \
    --exclude "*.lai" \
    $activity_dir

# Create the sugar .xo zip bundle
rm -f $activity_dir.xo
tar -tjf $activity_dir.tar.bz2 | zip $activity_dir.xo -@

# Sugar cleanup
rm -rf $activity_dir
rm $activity_dir.tar.bz2