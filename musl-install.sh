#!/bin/sh -e

VERSION=1.2.2
NAME1=musl-$VERSION
NAME2=$NAME1.tar.gz


if ! [ $# -eq 3 ]; then
	exit 1
fi

if ! [ -e $2 ]; then
	mkdir -p "$2"
fi

if ! echo $1 | grep '^/' >/dev/null ||
	! echo $2 | grep '^/' >/dev/null; then
	exit 1
fi

if [ -f $2/bin/musl-gcc ]; then
	exit 0
fi

if ! [ -f $1/$NAME2 ]; then
	echo "Missing $NAME2"
	echo "You should download and put it in $1"
	exit 1
fi

gzip -dkc $1/$NAME2 | tar -x

cd $NAME1

./configure --prefix=$2 --disable-shared
make -j$3
make -j$3 install
