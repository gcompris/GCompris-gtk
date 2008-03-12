#!/bin/bash

# usage: tranlators.sh LAST_RELEASE_TAG

# Changed so that the list comes out sorted by .po file name
# Mariano Suárez-Alvarez, Tue Feb 17 16:59:28 ART 2004

if [ ! $# -eq 1 ] ; then
        echo "usage: $0 LAST_RELEASE_TAG"
        exit 1;
fi

svn diff -r $1 po/ChangeLog | \
  (awk  '/\+.*[a-z][a-zA-Z@_]*\.po/ { print gensub ("[:,]", "\n", "g", $3); }' | \
    while read file; do
  if [ -z "$file" ]; then continue; fi
  echo "(${file%%.po})" $(grep "Last-Translator" po/$file | sed -e 's/"Last-Translator:  *\(.*\)  *<.*/\1/')
done) | sort | uniq | sed -e 's/\((.*)\) \(.*\)/\2 \1/'
