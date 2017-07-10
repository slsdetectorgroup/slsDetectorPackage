GITREPO1='git remote -v'
GITREPO2=" | grep \"fetch\" | cut -d' ' -f1"
BRANCH1='git branch -v'
BRANCH2=" | grep '*' | cut -d' ' -f2"
REPUID1='git log --pretty=format:"%H" -1'
AUTH1_1='git log --pretty=format:"%cn" -1'
AUTH1_2=" | cut -d' ' -f1"
AUTH2_1='git log --pretty=format:"%cn" -1'
AUTH2_2=" | cut -d' ' -f2"
FOLDERREV1='git log --oneline . '   #used for all the individual server folders
FOLDERREV2=" | wc -l"  #used for all the individual server folders
REV1='git log --oneline  '
REV2=" | wc -l"

GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
REV=`eval $REV1  $REV2`
FOLDERREV=`eval $FOLDERREV1  $FOLDERREV2`

