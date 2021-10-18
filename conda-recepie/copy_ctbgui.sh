# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
mkdir $PREFIX/lib
mkdir $PREFIX/bin
mkdir $PREFIX/include



cp build/bin/ctbGui $PREFIX/bin/.
cp build/bin/libctbRootLib.so $PREFIX/lib/.

