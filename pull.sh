#!/bin/bash
git pull
for i in */; do
	cd $i
	git pull
	cd ..
done

