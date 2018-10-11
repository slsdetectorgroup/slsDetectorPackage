if [ "$#" -eq 0 ]; then
    echo "Wrong number of arguments: usage should be $0 patname"
    exit 1
fi
infile=$1
outfile=$infile"at"
outfilebin=$infile"bin"
if [ "$#" -ge 2 ]; then
    outfile=$2
fi
exe=$infile"exe"
if [ "$#" -ge 4 ]; then
    exe=$4
fi

if [ "$#" -ge 3 ]; then
    outfilebin=$3
fi

if [ -f "$infile" ]
then
gcc -DINFILE="\"$infile\"" -DOUTFILE="\"$outfile\""  -DOUTFILEBIN="\"$outfilebin\"" -o $exe generator.c ; 
echo compiling
$exe ; 
echo cleaning
rm $exe
echo done
else
	echo "$infile not found."
fi
