SRCFILE=gitInfoJungfrau.h
DSTFILE=../commonFiles/versionAPI.h

SRCPATTERN=GITDATE
DSTPATTERN=APIJUNGFRAU

awk -v a="$SRCFILE" -v b="$DSTFILE" -v c="$SRCPATTERN" -v d="$DSTPATTERN" 'FNR==NR&&$2==c{x=$3} NR!=FNR{if($2==d){$3=x}print > b}' $SRCFILE $DSTFILE