#!/bin/bash
for file in $(find . -name "*.h"); do
  echo Processing $file

  cat notice_to_add_for_every_file $file > $file.modified

  mv $file.modified $file

done
