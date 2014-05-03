#!/bin/sh

find /home/arthur/download -name "*.avi" | while read video
do
    rm -f output
    echo $video
    ./get_subtitle "$video"
    if [ -f output ]; then
	echo "Success"
	cat output
    else
	echo "Failed"
    fi
done
