#!/bin/sh

# Plays a video on VDR. Give the video page address as parameter.
#
# Pre-requisites: svdrpsend.pl (from VDR)
#
# Example usage:
# watchonvdr.sh http://www.youtube.com/watch?v=n8qHOnlbvRY

# Put your SVDRP host and port here
VDR_HOST=127.0.0.1
VDR_PORT=2001

if [ "x$1" = "x" ]; then
    echo "video page URL expected"
    exit 1
fi

svdrpsend.pl -d $VDR_HOST -p $VDR_PORT "plug webvideo play $1"
