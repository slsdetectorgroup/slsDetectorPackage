#!/bin/bash
git $1
for i in sls*/; do
	cd $i
	echo $i
	git $1
	cd ..
done
