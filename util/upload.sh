#!/bin/sh
for FUNC in get_simple put_simple wordcount wordcount_manual_ndp thumbnailer
do
    inv func.compile ndp $FUNC
    inv func.upload ndp $FUNC
done

inv func.compile demo hello
inv func.upload demo hello
