# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package

echo "|<-------- starting python build"
cd python
cp ../VERSION slsdet/VERSION
${PYTHON} setup.py install 
