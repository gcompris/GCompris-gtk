#!/bin/bash

# usage: tranlators.sh LAST_RELEASE_TAG

if [ ! $# -eq 1 ] ; then
	echo "usage: $0 LAST_RELEASE_TAG"
	exit 1;
fi

cvs diff -r $1 po/ChangeLog | \
  (awk  '/\.po/ { print gensub ("[:,]", "\n", "g", $3); }' | \
    while read file; do
  if [ -z "po/$file" ]; then continue; fi
  echo $(grep "Last-Translator" po/$file | sed -e 's/"Last-Translator:  *\(.*\)  *<.*/\1/') "(${file%%.po})"
done) | sort | uniq
