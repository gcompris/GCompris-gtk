#!/bin/sh
# Once gcompris2spip is run, run this one to check no screenshots
# are missing
grep screenshots/ all_article.spip | cut -d= -f5 | cut -d \" -f2 | cut -d "&" -f1 |sort -u | xargs ls
