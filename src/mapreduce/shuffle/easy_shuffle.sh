#!/bin/bash
set -o pipefail
export PS4='[  ${LINENO} - \A - $0 ] '

cur_script=`readlink -f $0`
cur_dir=`basename $cur_script`

function print_help() {
   echo "usage help:"
   echo "cat data.txt | ./easy_shuffle.sh -r [reducer total] -o [output dir]" >&2
}

valid_arg=0
output_dir=""
reducer_total=0

while getopts "r:o:h" opt
do
    case "$opt" in
        "h")
            print_help
            exit -1
        ;;

        "o")
            output_dir=$OPTARG 
            let valid_arg=$valid_arg+1
        ;;

        "r")
            reducer_total=$OPTARG
            let valid_arg=$valid_arg+1
        ;;
    esac
done

if [ $valid_arg -lt 2 ]; then
    print_help
    exit -2 
fi

if [ ! -d $output_dir ] ; then
    mkdir -p $output_dir
else
    rm -rf $output_dir/*
fi

./partitioner $reducer_total | awk -F"\1\1\1" -v output_dir=$output_dir '{
reducer_slot=$1; 
key_len=length($1)
file_name=output_dir"/"reducer_slot".tmp"
print substr($0,key_len+4) > file_name }'

if [ $? -ne 0 ]; then
    echo "some errors happen when splitting data" >&2
    exit -3
fi

exit 0


