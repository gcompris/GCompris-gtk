#!/bin/bash
#
# generate_lang_rcc.sh
#
# Copyright (C) 2014 Holger Kaelberer
#
# Generates Qt binary resource files (.rcc) for lang images
#
# Usage:
# cd git/src/lang-activity/resources/lang
# generate_lang_rcc.sh
#
# Results will be written to $PWD/.rcc/ which is supposed be synced to the
# upstream location.
#

QRC_DIR="."
RCC_DIR=".rcc"
#RCC_DEFAULT=`which rcc 2>/dev/null`   # default, better take /usr/bin/rcc?
RCC_DEFAULT=$Qt5_DIR/bin/rcc
CONTENTS_FILE=Contents
MD5SUM=/usr/bin/md5sum


DATA_DIR=$1
if [ ! -d "${DATA_DIR}" ]
then
    echo "Usage: $0 directory"
    exit 1
fi

[ -z "${RCC}" ] && RCC=${RCC_DEFAULT}

[ -z "${RCC}" ] && {
    echo "No rcc command in PATH, can't continue. Try to set specify RCC in environment:"
    echo "RCC=/path/to/qt/bin/rcc $0"
    exit 1
}

echo "Generating binary resource files in ${RCC_DIR}/ folder:"

[ -d ${RCC_DIR} ] && rm -rf ${RCC_DIR}
mkdir  ${RCC_DIR}

QRC_FILE="${QRC_DIR}/${DATA_DIR}.qrc"
RCC_FILE="${RCC_DIR}/${DATA_DIR}.rcc"

# Generate QRC:
echo -n "  Generating ${QRC_FILE} ... "
# check for junk in the voices dirs:
if [ ! -z "`git status --porcelain ${DATA_DIR} | grep '^??'`" ]; then
    echo "Found untracked files in your git checkout below ${LANG}. Better "git clean -f" it first!";
    exit 1;
fi
[ -e ${QRC_FILE} ] && rm ${QRC_FILE}

#header:
(cat <<EOHEADER
<!DOCTYPE RCC><RCC version="1.0">
<qresource prefix="/gcompris">
EOHEADER
) >> $QRC_FILE
for i in `find ${DATA_DIR} -not -type d`; do
    echo "    <file>${i#./}</file>" >> $QRC_FILE
done
#footer:
(cat <<EOFOOTER
</qresource>
</RCC>
EOFOOTER
) >> $QRC_FILE

# Generate RCC
echo -n "${RCC_FILE} ... "
${RCC} -binary ${QRC_FILE} -o ${RCC_FILE}

echo "md5sum ... "
cd ${RCC_DIR}
${MD5SUM} `basename ${RCC_FILE}` >>${CONTENTS_FILE}
cd - &>/dev/null

#cleanup:
#rm *.qrc

echo "Finished! Now do something like:"
echo "rsync -avx ${RCC_DIR}/  www.gcompris.net:/var/www/data/${DATA_DIR}"
#EOF
