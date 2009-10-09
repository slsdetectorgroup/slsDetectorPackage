#!/bin/csh -f
#set l = `ipcs -m | grep "$USER"| cut -c12-19`
set l = `ipcs -m | cut -c12-19`
foreach s ( $l )
    echo $s
    ipcrm shm $s 
end
#if ($#l != 0 ) 
echo $#l shared memory\(s\) for $user removed

