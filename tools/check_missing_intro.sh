#! /bin/sh
#
# This scripts get the list of activities from the SQLite database
# assuming GCompris has already been started one on this user account.
# Given a locale name, it check that all the intro voices are present.
#

voicedir=$1
if [ -z "$1" ] || [ ! -d $voicedir ]; then
    echo "Usage: checkmissing_intro.sh <voice directory>"
    echo "  e.g. checkmissing_intro.sh boards/voices/en"
    exit 1
fi

request="select name from boards where type != 'menu' and section != '/experimental"\
"' and name != 'tuxpaint' and name !='administration' and name !='login';"

for name in `sqlite3 ~/.config/gcompris/gcompris_sqlite.db "$request"`; do
    file="$voicedir/intro/$name.ogg"
    if [ ! -f $file ]; then
	echo "$file"
    fi
done
