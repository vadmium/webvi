#!/usr/bin/python

import distutils.sysconfig
import os
import os.path

libdir = distutils.sysconfig.get_config_var('LIBDIR')
ldlibrary = distutils.sysconfig.get_config_var('LDLIBRARY')

libfile = os.readlink(os.path.join(libdir, ldlibrary))
if not os.path.isabs(libfile):
    libfile = os.path.join(libdir, libfile)

print libfile
