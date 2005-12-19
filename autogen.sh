#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="gcompris"

(test -f $srcdir/configure.in) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level directory"
    exit 1
}

REQUIRED_AUTOMAKE_VERSION=1.6 \
USE_GNOME2_MACROS=1 \
BUILD_PATH="$srcdir" \
srcdir="$srcdir" \
. $srcdir/gnome2-macros/gnome-autogen.sh
