#####! /bin/awk -f


if [ $# -lt 3 ] 
then
    echo "wrong usage"
    exit -1
fi


fin=$1
ftmp=$2
fout=$3


#dat=echo "date '+%Y%m%d'"

echo "Updating $fout"
#echo "in: $fin tmp: $ftmp out: $fout"

#awk 'NR==FNR {if ($3=="Date:") {l[FNR]=$4; gsub("-","",l[FNR]);} else { if (match($0,"Rev")) {l[FNR]=$(NF);} else {l[FNR]="\""$(NF)"\"";};};next} {$0=$1" "$2" "l[FNR]}1' $fin $ftmp > $fout

awk 'BEGIN {l[0]=0; "date +%Y%m%d" | getline l[1]; l[2]="\"/\""; l[3]="\"nobody\""; l[3]="\"nobody\""; l[4]="\"0000-0000-0000\"";} \
NR==FNR {if (match($0,"Rev")) {l[0]="0x"$(NF);} else if (match($0,"Date")) {l[1]="0x"$4; gsub("-","",l[1]);} else if (match($0,"URL")) {l[2]="\""$(NF)"\"";} else if (match($0,"Author")) {l[3]="\""$(NF)"\"";} else if (match($0,"UUID")) {l[4]="\""$(NF)"\"";} else if (match($0,"Branch")) {l[5]="\""$(NF)"\"";};next;} 
{if (match($2,"REV")) {$0=$1" "$2" "l[0];} else if (match($2,"DATE")) {$0=$1" "$2" "l[1];} else if (match($2,"URL")) {$0=$1" "$2" "l[2];} else if (match($2,"AUTH")) {$0=$1" "$2" "l[3];} else if (match($2,"UUID")) {$0=$1" "$2" "l[4];} else if (match($2,"BRANCH")) {$0=$1" "$2" "l[5];}}1' $fin $ftmp > $fout
