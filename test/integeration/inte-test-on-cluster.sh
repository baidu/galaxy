#!/usr/bin/env sh
USER=$1
PASSWORD=$2
CUR_DIR=`pwd`
cd ../../ && make -j8 -s
GALAXY_SRC_HOME=`pwd`
#add python sdk to python
cd $GALAXY_SRC_HOME/console/backend && sh compile-proto.sh && export PYTHONPATH=`pwd`/src
#add galaxy package
cd $GALAXY_SRC_HOME/deploy && sh make-package.sh && cp galaxy.tar.gz /tmp

# migrate test cluster

cd $CUR_DIR && python $GALAXY_SRC_HOME/deploy/deployer.py migrate -c galaxy_ci.py -u$USER -p$PASSWORD

# start integration test

#cd $CUR_DIR && python runner.py -d .



