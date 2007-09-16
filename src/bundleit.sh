#!/bin/sh
#
# A far from complete script that can create a binary tarball
# for the given activity.
#
if test -z "$1"; then
    echo "Usage: ./bundleit.sh directory-activity [locale code]"
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

# Create the Sugar specific startup scripts
activity_name=`basename $1 -activity`
activity_dir=${activity_name}.activity
if [ -d $activity_dir ]
then
  echo "The temporary directory already exists, delete it first"
  exit 1
fi

cp -a $1 $activity_dir
mkdir -p $activity_dir/activity
cp activity-gcompris.svg $activity_dir/activity
cp activity.info $activity_dir/activity
sed -i s/@ACTIVITY_NAME@/$activity_name/g $activity_dir/activity/activity.info
cp gcompris-instance $activity_dir/
cp gcompris-factory $activity_dir/
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

# Added the mandatory sounds of this activity
mandatory_sound_file=`grep mandatory_sound_file $activity_dir/*.xml.in | cut -d= -f2 | sed s/\"//g`
echo $mandatory_sound_file
if test -n $mandatory_sound_file
then
    mandatory_sound_file=`echo "$mandatory_sound_file" | sed 's/\$LOCALE/'$lang/`
    mandatory_sound_file=`dirname $mandatory_sound_file`
    mandatory_sound_file_up=`dirname $mandatory_sound_file`
    echo "Adding mandatory sound file directory: $mandatory_sound_file"
    mkdir -p $activity_dir/resources/$mandatory_sound_file_up
    dotdot=`echo $mandatory_sound_file_up | sed s/[^/]*/../g`
    ln -s $dotdot/../../../boards/$mandatory_sound_file -t $activity_dir/resources/$mandatory_sound_file_up
fi

# Add the python plugin if needed
py=`ls $1/*.py 2>/dev/null`
if test "$py" != ""; then
    cp $1/../boards/.libs/libpython.so $activity_dir
fi

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