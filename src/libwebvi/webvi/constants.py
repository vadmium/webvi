# constants.py - Python definitions for constants in libwebvi.h
#
# Copyright (c) 2009, 2010 Antti Ajanki <antti.ajanki@iki.fi>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

# Keep these in sync with libwebvi.h

class WebviRequestType:
    MENU = 0
    FILE = 1
    STREAMURL = 2

class WebviErr:
    OK = 0
    INVALID_HANDLE = 1
    INVALID_PARAMETER = 2
    INTERNAL_ERROR = -1

class WebviOpt:
    WRITEFUNC = 0
    READFUNC = 1
    WRITEDATA = 2
    READDATA = 3

class WebviInfo:
    URL = 0
    CONTENT_LENGTH = 1
    CONTENT_TYPE = 2
    STREAM_TITLE = 3

class WebviSelectBitmask:
    TIMEOUT = 0
    READ = 1
    WRITE = 2
    EXCEPTION = 4

class WebviConfig:
    TEMPLATE_PATH = 0
    DEBUG = 1
