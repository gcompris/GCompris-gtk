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

if test "$1" != "draw-activity" && \
    test "$1" != "anim-activity" && \
    test "$1" != "electric-activity" ; then
  draw="--exclude resources/skins/gartoon/draw"
else
  draw=""
fi

if test -f $1/init_path.sh; then 
  . $1/init_path.sh
else
  echo "ERROR: Cannot find $1/init_path.sh"
  exit 1
fi

# Create the Sugar specific startup scripts
activity_dir=${activity}.activity
if [ -d $activity_dir ]
then
  echo "The temporary directory '$activity_dir' already exists, delete it first"
  exit 1
fi

cp -a $1 $activity_dir
mkdir -p $activity_dir/activity
cp activity-gcompris.svg $activity_dir/activity
cp activity.info $activity_dir/activity
sed -i s/@ACTIVITY_NAME@/$activity/g $activity_dir/activity/activity.info
cp gcompris-instance $activity_dir/
cp gcompris-factory $activity_dir/
cp gcompris/gcompris $activity_dir/gcompris.bin
if [ -f $activity_dir/.libs/*.so ]; then
  mv $activity_dir/.libs/*.so $activity_dir
fi
rm -rf $activity_dir/.libs

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
mandatory_sound_dir=`grep mandatory_sound_dir $activity_dir/*.xml.in | cut -d= -f2 | sed s/\"//g`
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
  ln -s $resourcedir -t $activity_dir
fi

# Add the plugins in the proper place
echo "This activity has it's plugindir in $plugindir"
cp $plugindir/*.so $activity_dir
rm -f $activity_dir/menu.so

# Add the python plugins
if [ -f $pythonplugindir/*.py ]; then 
  cp $pythonplugindir/*.py $activity_dir
fi

# Add the runit.sh script
cp $activity_dir/../runit.sh $activity_dir

tar -cjf $activity_dir.tar.bz2 -h \
    --exclude ".svn" \
    --exclude "resources/skins/babytoy" \
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