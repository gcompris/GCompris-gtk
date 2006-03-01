#!/bin/sh
#
# Copyright (C) 2006 Jose JORGE
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#######################################################################
#
# This script lists the sounds missing in some locale compared to english
# It much be run with the list of locale sound you want to compare
# list_missing_sounds fr en de
#
snd_path="boards/sounds/"
reference="en"

if test "$1" == ""
then
  echo "Usage: $0 <locale>"
  echo "With locale being a locale as found in boards/sounds like fr"
  echo "Run it in the GCompris root directory"
  exit 1
fi

for locale in $@
do
  if test -d boards/sounds/$locale; then 
	  find $snd_path$locale | grep .ogg > ~/tmp/sounds_$locale.lst
	  find $snd_path$reference | grep .ogg | sed s+/$reference/+/$locale/+ > ~/tmp/sounds_$reference.lst
	  echo "The sounds avaliable in $reference that miss in $locale are :"
	  grep -v -f ~/tmp/sounds_$locale.lst ~/tmp/sounds_$reference.lst
  fi
done
rm ~/tmp/sounds_$locale.lst ~/tmp/sounds_$reference.lst
