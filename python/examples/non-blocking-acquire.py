# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
import time
from slsdet import Detector, runStatus


n_frames = 10
t_exp = 1

# Set exposure time and number of frames
d = Detector()
d.exptime = t_exp
d.frames = n_frames

# Start the measurement
t0 = time.time()
d.startDetector()
d.startReceiver()

# Wait for the detector to be ready or do other important stuff
time.sleep(t_exp * n_frames)

# check if the detector is ready otherwise wait a bit longer
while d.status != runStatus.IDLE:
    time.sleep(0.1)

# Stop the receiver after we got the frames
# Detector is already idle so we don't need to stop it
d.stopReceiver()

lost = d.rx_framescaught - n_frames
print(
    f"{n_frames} frames of {t_exp}s took {time.time()-t0:{.3}}s with {lost} frames lost "
)

