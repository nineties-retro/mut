#!/bin/sh
#
# Driver for Unix dependency generator

awk -f $1/depend.awk input=$2 dependency_file=$3 object_file=$4 include_path=$5 < /dev/null > $3
