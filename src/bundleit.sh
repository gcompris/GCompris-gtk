#!/bin/bash
#set -x
if [ -z $1 ]; then
  echo "Usage: bundleit.sh [one or more GCompris activity directory]"
  exit 1
fi

activity=${1%/}

tmpdir=`mktemp -d`
echo "Temporary directory: $tmpdir"

for var in "$@"
do
    activ=${var%/}
    echo "Processing $activ"
    cd $activ
    if [ $? -ne 0 ]
    then
        echo "ERROR: $activ directory not found"
	rm -rf $tmpdir
	exit 1
    fi
    make -s install DESTDIR=$tmpdir
    if [ $? -ne 0 ]
    then
        echo "ERROR: Make failed"
	rm -rf $tmpdir
	exit 1
    fi
    cd ..
done

echo "Creating $activity.tgz"
tar -czf $activity.tgz -C $tmpdir/usr/local/share/gcompris/ boards python

rm -rf $tmpdir

echo "The bundle is created in $activity.tgz"
