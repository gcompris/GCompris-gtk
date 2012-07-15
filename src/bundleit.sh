#!/bin/bash
#set -x
if [ -z $1 ]; then
  echo "Usage: bundleitit.sh [GCompris activity directory]"
  exit 1
fi

activity=${1%/}

tmpdir=`mktemp -d`

cd $activity
make install DESTDIR=$tmpdir
cd ..

tar -czf $activity.tgz -C $tmpdir/usr/local/share/gcompris/ boards python

rm -rf $tmpdir

echo "The bundle is created in $activity.tgz"