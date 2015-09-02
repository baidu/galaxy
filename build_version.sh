#!/bin/bash
VERSION_STR=`git log -n 1 | head -1 | cut -d" " -f2`
sed -i s/_TEST_VERSION_/$VERSION_STR/g COMAKE
