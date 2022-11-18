#!/bin/sh
for FUNC in get put wordcount grep substr sha256sum thumbnailer_shrink thumbnailer_decode pcakmm
do
    inv func.compile ndp $FUNC
    inv func.upload ndp $FUNC
done

inv func.compile demo hello
inv func.upload demo hello
