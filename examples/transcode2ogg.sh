#!/bin/sh

# An example post-processing script for VDR plugin webvideo.
#
# Copyright: Antti Ajanki <antti.ajanki@iki.fi>
# License: GPL3, see the file COPYING for the full license
#
# This script transcodes a video file using Ogg Theora and Vorbis
# codecs. The first parameter is the name of the video file.
#
# To setup this script to be called for every downloaded file, start
# the webvideo plugin with option -p. For example:
#
# vdr -P "webvideo -p /path/to/this/file/transcode2ogg.sh"

fullsrcname=$1
videodir=`dirname "$fullsrcname"`
srcfile=`basename "$fullsrcname"`
srcbasename=`echo "$srcfile" | sed 's/\.[^.]*$//'`
destname="$videodir/$srcbasename.ogg"

nice -n 19 ffmpeg -i "$fullsrcname" -qscale 7 -vcodec libtheora -acodec libvorbis -ac 2 -y "$destname"

if [ $? -eq 0 ]; then
    rm -f "$fullsrcname"
    exit 0
else
    exit 1
fi
