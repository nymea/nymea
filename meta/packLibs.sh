#!/bin/bash

function packLib()
{
echo -e "\nCopy dependencies for ${1}"
deps=$(ldd $1 | awk 'BEGIN{ORS=" "}$1 ~/^\//{print $1}$3~/^\//{print $3}' | sed 's/,$/\n/')

#Copy the deps
for dep in $deps
do
    cp -v "$dep" "$2"
done
}

#################################################
if [ -d ./dependencies ]; then
    echo "Clean dependency folder"
    rm -rf ./dependencies/*
else
    mkdir -v ./dependencies
fi

packLib libguh/libguh.so ./dependencies
packLib server/guhd ./dependencies

# TODO: pack qt plugin sqlite driver lib
