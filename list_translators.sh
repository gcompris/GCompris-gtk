#!/bin/sh

# usage: translators.sh <LAST_COMMIT_ID>

echo "UI translations:"
git log $1..HEAD --pretty=format:%an --name-only  -- po/*.po | sed -e :a -e '$!N;s|\npo/\(.*\)\.po| \(\1\)|;ta' | sort -u | sed '$!N;s/^\n//'
