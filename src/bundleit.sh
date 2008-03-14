#!/bin/sh
#
# A far from complete script that can create a binary tarball
# for the given activity.
#

if test -z "$1"; then
    echo "Usage: bundleit.sh directory-activity [locale code]"
    echo "Example (for french locale):"
    echo "./bundleit.sh crane-activity fr"
    exit 1
fi

lang=
if test -n "$2"; then
    lang=$2
fi

activitysrc=`basename $1`
echo "Processing activity $activitysrc"
if test "$activitysrc" != "draw-activity" && \
    test "$activitysrc" != "anim-activity" && \
    test "$activitysrc" != "pythontest-activity" && \
    test "$activitysrc" != "electric-activity" ; then
  draw="--exclude resources/skins/gartoon/draw"
else
  draw=""
fi

if test "$activitysrc" = "administration-activity" || \
   test "$activitysrc" = "tuxpaint-activity" || \
   test "$activitysrc" = "melody-activity" || \
   test "$activitysrc" = "gcompris-activity" ; then
  echo "Skipping $activitysrc"
  exit 0
fi

extra_bin=""
if test "$activitysrc" = "chess_computer-activity" || \
   test "$activitysrc" = "chess_movelearn-activity" || \
   test "$activitysrc" = "chess_partyend-activity" ; then
 extra_bin=`which gnuchess`
fi

if test "$activitysrc" = "electric-activity" ; then
 extra_bin=`which gnucap`
fi

if test -f $activitysrc/init_path.sh; then
  . $activitysrc/init_path.sh
else
  echo "ERROR: Cannot find $activitysrc/init_path.sh"
  exit 1
fi

with_clock="--exclude resources/skins/gartoon/timers"
for act in `grep timers/clock */*.c | cut -d/ -f1 | sort -u | xargs`
do
  if test "$activitysrc" = $act; then
    echo "Adding timers/clock files"
    with_clock=""
  fi
done

# Create the Sugar specific startup scripts
activity_dir=${activity}.activity
if [ -d $activity_dir ]
then
  echo "The temporary directory '$activity_dir' already exists, delete it first"
  exit 1
fi

cp -a $activitysrc $activity_dir
mkdir -p $activity_dir/activity
mkdir -p $activity_dir/bin
mv $activity_dir/*.svg $activity_dir/activity/activity-gcompris.svg
cp activity.info $activity_dir/activity
sed -i s/@ACTIVITY_NAME@/$activity/g $activity_dir/activity/activity.info
cp old-gcompris-instance $activity_dir/
cp old-gcompris-factory $activity_dir/
cp old-gcompris-activity $activity_dir/
sed -i s/@ACTIVITY_NAME@/$activity/g $activity_dir/old-gcompris-activity
cp $activity_dir/old-gcompris-activity $activity_dir/gcompris-activity
cp gcompris-activity $activity_dir/bin/
cp gcompris/gcompris $activity_dir/gcompris.bin
if [ -f $activity_dir/.libs/*.so ]; then
  mv $activity_dir/.libs/*.so $activity_dir
fi
rm -rf $activity_dir/.libs
rm -rf $activity_dir/*.in
if [ -f "$extra_bin" ]; then
  cp $extra_bin $activity_dir/bin
fi

# Add the locale translation file
dir=$activity_dir/locale/$lang/LC_MESSAGES
mkdir -p $dir
if test -r ../po/$lang.gmo; then
    cp ../po/$lang.gmo $dir/gcompris.mo
    echo "installing $lang.gmo as $dir/gcompris.mo"
else
    echo "WARNING: No translation found in ../po/$lang.gmo"
fi

# Add the mandatory sounds of this activity
mandatory_sound_dir=`grep mandatory_sound_dir $activity_dir/*.xml | cut -d= -f2 | sed s/\"//g`
if test -n "$mandatory_sound_dir"
then
    echo "This activity defines a mandatory_sound_dir in $mandatory_sound_dir"
    mandatory_sound_dir=`echo "$mandatory_sound_dir" | sed 's/\$LOCALE/'$lang/`
    echo "Adding mandatory sound dir directory: $mandatory_sound_dir"
    up=`dirname $mandatory_sound_dir`
    mkdir -p $activity_dir/resources/$up
    dotdot=`echo $up | sed s/[^/]*/../g`
    ln -s $dotdot/../../../boards/$mandatory_sound_dir -t $activity_dir/resources/$up
fi

# Add the resources if they are in another activity
if [ ! -d $activity_dir/resources ]; then
  echo "This activity has it's resources in $resourcedir/"
  ln -s ../$resourcedir -t $activity_dir
fi

# Add the plugins in the proper place
echo "This activity has it's plugindir in $plugindir"
cp $plugindir/*.so $activity_dir
rm -f $activity_dir/libmenu.so

# Add the python plugins
if [ -f $pythonplugindir/*.py ]; then
  cp $pythonplugindir/*.py $activity_dir
  # Add the GCompris binding
  rm -f $activity_dir/gcompris
  ln -s ../boards/python/gcompris -t $activity_dir
fi

# Add the runit.sh script
cp $activity_dir/../runit.sh $activity_dir

tar -cjf $activity_dir.tar.bz2 -h \
    --exclude ".svn" \
    --exclude "resources/skins/babytoy" \
    $draw \
    $with_clock \
    --exclude ".deps" \
    --exclude "Makefile*" \
    --exclude "*.c" \
    --exclude "*.h" \
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