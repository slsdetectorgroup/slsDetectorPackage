#!/bin/bash
git $@
for i in sls*/; do
     cd $i
     echo $i
     git $@
     cd ..
done
