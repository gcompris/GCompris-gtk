#!/bin/sh
# Run it in the topp level GCompris directory
set -x
if test ! -f gcompris.png; then
  echo "You must run this script in the top level GCompris directory"
  exit 1
fi
# To create a full GCompris bundle on the OLPC xo:
mkdir /tmp/gcompris.activity
sh autogen.sh --enable-dbus --disable-sqlite --prefix=/tmp/gcompris.activity
if test $? -ne 0; then
  echo "ERROR: Fix autogen first"
  exit 1
fi
make
make install
tar  -C /tmp/gcompris.activity -xzf tools/full-xo-bundle-add-on.tzf

# The full bundle follow the release found in src/activity.info
release=`grep activity_version src/activity.info | cut -d= -f2 | sed "s/ //"`
sed -i "s/@RELEASE@/$release/" /tmp/gcompris.activity/activity/activity.info

# Keep only some locale voices relevant to XO targets
cd /tmp/gcompris.activity/share/gcompris/boards/voices
rm -rf ar bg br cs da de el eu fi fr he hi HOWTO_ENCODE hu id it mr nb nl nn ru so sr sv tr ur

# Copy extra binaries
extra_bin=`which gnuchess`
if [ "$extra_bin" = "" ]; then
  extra_bin=`which gnome-gnuchess`
  if [ "$extra_bin" = "" ]; then
    ERROR "  ERROR: Cannot find gnuchess or gnome-gnuchess"
  fi
fi
cp $extra_bin /tmp/gcompris.activity/bin

extra_bin=`which gnucap`
if [ "$extra_bin" = "" ]; then
  ERROR "  ERROR: Cannot find gnucap"
fi
cp $extra_bin /tmp/gcompris.activity/bin

cd /tmp
rm gcompris.activity.xo
zip -r gcompris.activity.xo gcompris.activity
