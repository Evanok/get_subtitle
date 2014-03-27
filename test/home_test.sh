#!/bin/sh

find /home/arthur/download -name "*.avi" | while read video
do
    rm -f output
    echo $video
    ./video_lima "$video"
    if [ -f output ]; then
	echo "Success"
	cat output
    else
	echo "Failed"
    fi
done
