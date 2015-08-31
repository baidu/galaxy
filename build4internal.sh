#!/usr/bin/env sh
BUILD_HOME=`pwd`

if [ -e common ];then
    cd common && git pull && cd $BUILD_HOME
else
    git clone https://github.com/baidu/common
fi

echo "build common done"

cd $BUILD_HOME/common 
# for use the same boost header
cp Makefile Makefile_bak
sed -i 's/^INCLUDE_PATH.*$/INCLUDE_PATH=-Iinclude -I..\/..\/..\/..\/third-64\/boost\/include/' Makefile
make -j4
cd -

cd $BUILD_HOME
if [ -e ../ins ];then
    cd ../ins && comake2 -UB && comake2 && make -j6
else
    cd $BUILD_HOME && cd .. && git clone http://gitlab.baidu.com/baidups/ins.git && cd ins && comake2 -UB && comake2 && make -j6
fi
cd $BUILD_HOME && test -e ins && rm -rf ins
mkdir ins && cp -rf ../ins/*  ins
echo "build ins done"

cd $BUILD_HOME && comake2 -UB && comake2
echo "Configuration done! please run 'make -j6' to compile galaxy"

VERSION_STR=`git log -n 1 | head -1 | cut -d" " -f2`
sed -i s/_TEST_VERSION_/$VERSION_STR/g COMAKE

