#!/bin/sh
#
# A far from complete script that can create a binary tarball
# for the given activity.
#

if test -z "$1"; then
    echo "Usage: bundleit.sh directory-activity [locale code]"
    echo "       If no locale is provided then bundle all locales."
    echo "Example (for french locale):"
    echo "./bundleit.sh crane-activity fr"
    exit 1
fi

ERROR()
{
  local CNONE="\033[0m"		# No color
  local CRED="\033[0;31m"	# Red color

  echo "${CRED}$1${CNONE}"
  exit 1
}

WARNING()
{
  local CNONE="\033[0m"		# No color
  local CMAG="\033[0;35m"	# Magenta color
  echo "${CMAG}$1${CNONE}"
}

# Sugar cleanup
cleanup()
{
    echo "Cleanup $activity_dir"
    rm -rf $activity_dir
    rm -f $activity_dir.tar.bz2
}

lang=
if test -n "$2"; then
    lang=$2
else
  WARNING "No locale specified => ALL LOCALES ARE PACKAGED"
fi

activitysrc=`basename $1`
echo "Processing activity $activitysrc"
if test "$activitysrc" != "draw-activity" && \
    test "$activitysrc" != "anim-activity" && \
    test "$activitysrc" != "electric-activity" ; then
  draw="--exclude resources/skins/gartoon/draw"
else
  draw=""
fi

if test "$activitysrc" = "administration-activity" || \
   test "$activitysrc" = "chat-activity" || \
   test "$activitysrc" = "tuxpaint-activity" || \
   test "$activitysrc" = "melody-activity" || \
   test "$activitysrc" = "login-activity" || \
   test "$activitysrc" = "gcompris-activity" || \
   test "$activitysrc" = "pythontest-activity" || \
   test "$activitysrc" = "pythontemplate-activity" || \
   test "$activitysrc" = "old-gcompris-activity" ; then
  echo "  Skipping it, not relevant for xo or not ready for it."
  exit 0
fi

extra_bin=""
if test "$activitysrc" = "chess_computer-activity" || \
   test "$activitysrc" = "chess_movelearn-activity" || \
   test "$activitysrc" = "chess_partyend-activity" ; then
  extra_bin=`which gnuchess`
  if [ "$extra_bin" = "" ]; then
    extra_bin=`which gnome-gnuchess`
    if [ "$extra_bin" = "" ]; then
      ERROR "  ERROR: Cannot find gnuchess or gnome-gnuchess"
    fi
  fi
fi

if test "$activitysrc" = "electric-activity" ; then
 extra_bin=`which gnucap`
 if [ "$extra_bin" = "" ]; then
   ERROR "  ERROR: Cannot find gnucap"
 fi
fi

if test -f $activitysrc/init_path.sh; then
  . $activitysrc/init_path.sh
else
  ERROR "  ERROR: Cannot find $activitysrc/init_path.sh"
fi

# Bundle names must be formated without underscores
# This translate chess_computer in chessComputer
bundle_id=`echo $activity | sed 's/_\([a-z]\)/\U\1/g'`
echo $bundle_id

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
sed -i s/@ACTIVITY_NAME@/$bundle_id/g $activity_dir/activity/activity.info
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
if [ "$lang" != "" ]; then
  dir=$activity_dir/locale/$lang/LC_MESSAGES
  mkdir -p $dir
  if test -r ../po/$lang.gmo; then
      cp ../po/$lang.gmo $dir/gcompris.mo
  else
      WARNING "  WARNING: No translation found in ../po/$lang.gmo"
  fi
else
  for gmo in ../po/*.gmo
  do
    lng=`echo $gmo | sed s/.gmo//`
    dir=$activity_dir/locale/$lng/LC_MESSAGES
    mkdir -p $dir
    cp ../po/$lng.gmo $dir/gcompris.mo
  done
fi

# Never keep the voices by default
rm -f $activity_dir/resources/voices

# Add the mandatory sounds of this activity
mandatory_sound_dir=`grep mandatory_sound_dir $activity_dir/*.xml | cut -d= -f2 | sed s/\"//g`
localized=`echo "$mandatory_sound_dir" | grep "\$LOCALE"`
# Is this a localized mandatory_sound_dir
if [ "$lang" != "" -a "$localized" != "" ]; then
  # Copying localized content
  mandatory_sound_dir=`echo "$mandatory_sound_dir" | sed 's/\$LOCALE/'$lang/`
  echo "  Adding mandatory sound dir directory: $mandatory_sound_dir"
  up=`dirname $mandatory_sound_dir`
  mkdir -p $activity_dir/resources/$up
  dotdot=`echo $up | sed s/[^/]*/../g`
  if [ -d ../boards/$mandatory_sound_dir ]
  then
    ln -s $dotdot/../../../boards/$mandatory_sound_dir -t $activity_dir/resources/$up
  else
    echo "ERROR: Resource dir not found: $dotdot/../../../boards/$mandatory_sound_dir"
    cleanup
    exit 1
  fi
fi

# Add the resources if they are in another activity
if [ ! -d $activity_dir/resources ]; then
  ln -s ../$resourcedir -t $activity_dir
fi

# Add the plugins in the proper place
cp $plugindir/*.so $activity_dir
rm -f $activity_dir/libmenu.so

# Add the python plugins
haspyfile=`ls $pythonplugindir/*.py 2>/dev/null`
if [ "$haspyfile" != "" ]; then
  cp $pythonplugindir/*.py $activity_dir
  # Add the GCompris binding
  rm -f $activity_dir/gcompris
  ln -s ../boards/python/gcompris -t $activity_dir
fi

# Add the timers skin if needed (python activity don't use it)
with_clock="--exclude resources/skins/gartoon/timers"
if [ "$haspyfile" = "" ]; then
  has_timer=`egrep "gc_timer_display|timers/clock" $plugindir/../*.c`
  if test "$has_timer" != ""; then
    echo "  Adding timers/clock files"
    with_clock=""
  fi
fi

# Add the runit.sh script
cp $activity_dir/../runit.sh $activity_dir

tar -cjf $activity_dir.tar.bz2 -h \
    --exclude ".gitignore" \
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
if test -z "$lang"; then
  suffix=""
else
  suffix="-$lang"
fi

rm -f $activity_dir.xo
tar -tjf $activity_dir.tar.bz2 | zip ${activity_dir}${suffix}.xo -@ > /dev/null

cleanup
