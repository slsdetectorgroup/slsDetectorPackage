for i in seq `ipcs -m | cut -d ' ' -f1`; do ipcrm -M $i; done;
