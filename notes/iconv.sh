#!/bin/bash

if [ $# != 1 ]
then
    echo "bad arg"
    exit 1
fi

iconv -c -f gb2312 -t utf-8 $1 > $1.bak
if [ $? = 0 ]
then
    mv $1.bak $1
    rm -f $1.bak
    echo "done"
elif
    echo "error"
fi
