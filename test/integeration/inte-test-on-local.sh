#!/usr/bin/env sh
CUR_DIR=`pwd`
cd ../../
GALAXY_SRC_HOME=`pwd`
export MASTER_BIN_PATH=$GALAXY_SRC_HOME/output/bin/master
cd $GALAXY_SRC_HOME/console/backend && sh compile-proto.sh && export PYTHONPATH=`pwd`/src
cd $CUR_DIR && mkdir -p case_tmp
export GALAXY_CASE_FOLDER=$CUR_DIR/case_tmp

nosetests -vs
