#!/bin/bash
git $1
for i in sls*/; do
	cd $i
	echo $i
	git diff
	cd ..
done
