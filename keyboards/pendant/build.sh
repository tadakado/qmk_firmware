#!/bin/sh

K=`dirname $0`
K=`(cd $K; pwd)`
K=`basename $K`
H=`dirname $0`
H=`(cd $H/../..; pwd)`
if [ "$*" = "" ]; then
    TARGETS="default no_msc"
else
    TARGETS="$*"
fi
FOLDER=$H/keyboards/${K}/firm/`TZ='Asia/Tokyo' date +%Y%m%d_%H%M`
echo $FOLDER

cd $H

for target in $TARGETS; do
    echo $target
    if [ "$target" = "clean" ]; then
        make clean
    else
        make ${K}:${target}:uf2 && mkdir -p $FOLDER && cp .build/${K}_${target}.uf2 $FOLDER
    fi
done

