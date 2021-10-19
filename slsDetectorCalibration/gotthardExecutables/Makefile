# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
slsReceiver --rx_tcpport 1954 &
slsReceiver --rx_tcpport 1955 &

./gotthard25umZmq pc8829 30003 129.129.202.98 40003 &
slsDetectorGui -f examples/bchip2modules_pc8829.config &


sls_detector_put settings veryhighgain
sls_detector_put exptime 0.000005
sls_detector_put period 0.01


sls_detector_put highvoltage 90

