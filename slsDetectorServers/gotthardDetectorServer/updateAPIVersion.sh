SRCFILE=gitInfoGotthard.h
DSTFILE=versionAPI.h

SRCPATTERN=GITDATE
DSTPATTERN=APIGOTTHARD

awk -v a="$SRCFILE" -v b="$DSTFILE" -v c="$SRCPATTERN" -v d="$DSTPATTERN" 'FNR==NR&&$2==c{x=$3} NR!=FNR{if($2==d){$3="0x"substr(x,5)}print > b}' $SRCFILE $DSTFILE