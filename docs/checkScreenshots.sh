#!/bin/sh
# Once gcompris2spip is run, run this one to check no screenshots
# are missing
files=$(grep screenshots/ all_article.spip | grep -v _small | cut -d= -f5 | cut -d \" -f2 | cut -d "&" -f1 |sort -u)

for f in $files
do
 ls $1/$f
done
