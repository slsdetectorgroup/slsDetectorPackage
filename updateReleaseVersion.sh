# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package

API_FILE=$PWD/slsSupportLib/include/sls/versionAPI.h
CURR_BRANCH=$(cat $API_FILE | grep RELEASE | awk '{print $3}' )

# default branch is developer
if [ $# -eq 0 ]; then
	declare -a NEW_BRANCH="developer"
else
	declare -a NEW_BRANCH=${1}
fi

# update branch
sed -i s/$CURR_BRANCH/\"$NEW_BRANCH\"/g $API_FILE
