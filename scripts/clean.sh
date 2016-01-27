#!/bin/bash
cur=`pwd`
dist="*~"

clean() {

	rm -rf $dist
	echo "`pwd` : $dist is clean."

	for i in `ls`
	do
		if [ -d $i ];then 
			cd $i
			clean
		fi
	done
	cd ..
}

clean
