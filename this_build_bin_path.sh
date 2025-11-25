#!/bin/bash

# Since this script could be sourced, $0 is not sufficent, BASH_SOURCE[0] is necessary
SCRIPT_LOCATION="$(realpath ${BASH_SOURCE[0]})"
SCRIPT_LOCATION="$(dirname ${SCRIPT_LOCATION})"
BUILDBIN_LOCATION="${SCRIPT_LOCATION}/build/bin"

if [ ! -d "${BUILDBIN_LOCATION}" ]; then
	echo Cannot find path ${BUILDBIN_LOCATION}
else
	echo Adding ${BUILDBIN_LOCATION} to PATH and PYTHONPATH
	export PATH=${BUILDBIN_LOCATION}:${PATH}
	export PYTHONPATH=${BUILDBIN_LOCATION}:${PYTHONPATH}
fi
