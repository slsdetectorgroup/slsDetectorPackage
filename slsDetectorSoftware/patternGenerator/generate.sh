if [ "$#" -eq 0 ]; then
    echo "Wrong number of arguments: usage should be $0 patname"
    exit 1
fi
infile=$1
outfile=$infile"at"
if [ "$#" -ge 2 ]; then
    outfile=$2
fi
exe=$infile"exe"
if [ "$#" -ge 3 ]; then
    exe=$3
fi

if [ -f "$infile" ]
then
gcc -DINFILE="\"$infile\"" -DOUTFILE="\"$outfile\"" -o $exe generator.c ; ./$exe ; rm $exe
else
	echo "$infile not found."
fi
