#! /bin/sh
export PYTHONPATH=`pwd`/src
nohup python src/manage.py runserver cq01-rdqa-pool056.cq01.baidu.com:8989  >log 2>&1 &

