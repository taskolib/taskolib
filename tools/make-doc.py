#!/usr/bin/env python3

#
#  \file   make-doc.py
#  \author Lars Froehlich
#  \date   Created on 26 July 2021
#
#  \copyright Copyright 2021-2022 Deutsches Elektronen-Synchrotron (DESY), Hamburg
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as published
#  by the Free Software Foundation, either version 2.1 of the license, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

import os
import re
import stat
import subprocess
import sys

from pathlib import Path

script_file = Path(sys.argv[0]).resolve()
os.chdir(script_file.parent.parent)

file = open('LIBNO', mode='r')
entire_file = file.read()
file.close()

matches = re.match(r'^(.*=)?(.*)', entire_file, re.S)

if matches is None or not matches.group(2):
    sys.exit("Unable to find version in LIBNO")

version_str = matches.group(2).strip()
print(version_str)

file = open('data/Doxyfile', mode='r')

output_directory = None
str = ""

for line in file:
    # Empty line
    matches = re.match('^\s*$', line)
    if matches is not None:
        continue

    # Comment line
    matches = re.match('^\s*#', line)
    if matches is not None:
        continue

    # Project number
    matches = re.match('^\s*OUTPUT_DIRECTORY\s*=\s*(.*)\s*$', line)
    if matches is not None:
        output_directory = matches.group(1)

    # Project number
    matches = re.match('^(\s*PROJECT_NUMBER\s*=\s*)([0123456789.]+)(.*)$', line)
    if matches is not None:
        str = str + '{}{}{}\n'.format(matches.group(1), version_str, matches.group(3))
        continue

    str = str + line

subprocess.run(("doxygen", "-"), text=True, input=str)

print("\nAdjusting file and directory permissions...")

for dirpath, dirnames, filenames in os.walk(output_directory):
    try:
        os.chmod(dirpath,
                 stat.S_IRUSR | stat.S_IRGRP | stat.S_IROTH |
                 stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH |
                 stat.S_IWUSR | stat.S_IWGRP)
    except PermissionError as e:
        #print(e)
        pass

    for filename in filenames:
        try:
            os.chmod(os.path.join(dirpath, filename),
                     stat.S_IRUSR | stat.S_IRGRP | stat.S_IROTH |
                     stat.S_IWUSR | stat.S_IWGRP)
        except PermissionError as e:
            #print(e)
            pass
