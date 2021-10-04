#!/bin/sh
for FUNC in get_simple put_simple wordcount grep substr sha256sum thumbnailer_shrink thumbnailer_decode
do
    inv func.compile ndp $FUNC
    inv func.upload ndp $FUNC
done

inv func.compile demo hello
inv func.upload demo hello
