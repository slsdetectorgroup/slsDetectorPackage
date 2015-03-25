#!/bin/bash
git fetch
for i in */; do
	cd $i
	git fetch
	cd ..
done

