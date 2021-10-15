# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
import time
from multiprocessing import Process
from slsdet import Detector, runStatus


d = Detector()

#Create a separate process to run acquire in
p = Process(target=d.acquire)

#Start the thread and short sleep to allow the acq to start
p.start()
time.sleep(0.01)

#Do some other work
while d.status != runStatus.IDLE:
    print("Working")
    time.sleep(0.1)

#Join the process
p.join()