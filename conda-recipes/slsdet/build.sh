# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package

echo "|<-------- starting python build"
echo $PWD
cd python
echo "folder: $PWD"
${PYTHON} setup.py install