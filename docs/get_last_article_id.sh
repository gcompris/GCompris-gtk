#!/bin/bash
file=$1
grep "<id_article>" $file | cut -c13- | sort -un | tail -1 | sed "s:</id_article>::"

