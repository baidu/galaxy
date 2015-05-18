#! /bin/sh
export PYTHONPATH=`pwd`/src
nohup python src/manage.py runserver 127.0.0.1:8989  >log 2>&1 &

