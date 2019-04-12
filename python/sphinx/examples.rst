Examples
================

Some short hints on how to use the detector

------------------------
Simple threshold scan
------------------------

Assuming you have set up your detector with exposure time, period, enabled
file writing etc.

.. code-block:: python
 
    from sls_detector import Eiger

    d = Eiger()
    threshold = range(0, 2000, 200)
    for th in threshold:
        d.vthreshold = th
        d.acq()
    

If we want to control the shutter of for example, the big X-ray box we can add
this line in our code. It then opens the shutter just before the measurement
and closes is afterwards.
    
::

    with xrf_shutter_open(box, 'Fe'):
        for th in threshold:
            d.vthreshold = th
            d.acq()
        
        
-----------------------
Reading temperatures
-----------------------       

::

    d.temp
    >>
    temp_fpga     :  43.19°C,  51.83°C
    temp_fpgaext  :  38.50°C,  38.50°C
    temp_10ge     :  39.50°C,  39.50°C
    temp_dcdc     :  42.50°C,  42.50°C
    temp_sodl     :  39.50°C,  40.50°C
    temp_sodr     :  39.50°C,  40.50°C
    temp_fpgafl   :  40.87°C,  37.61°C
    temp_fpgafr   :  34.51°C,  35.63°C
    
    d.temp.fpga
    >> temp_fpga     :  40.84°C,  39.31°C
    
    t = d.temp.fpga[0]
    t
    >> 40.551
    
    t = d.temp.fpga[:]
    t
    >> [40.566, 39.128]


-----------------------
Non blocking acquire
-----------------------

There are mainly two ways to achieve a non blocking acquire when calling from the Python API. One is to manually start
the detector and the second one is to launch the normal acquire from a different process. Depending on your measurement
it might also be better to run the other task in a seperate process and use acq in the main thread.
But lets start looking at the at the manual way:

::

    import time
    from sls_detector import Eiger
    d = Eiger()

    n = 10
    t = 1

    d.exposure_time = t
    d.n_frames = n
    d.reset_frames_caught()

    #Start the measurement
    t0 = time.time()
    d.start_receiver()
    d.start_detector()

    #Wait for the detector to be ready or do other important stuff
    time.sleep(t*n)

    #check if the detector is ready otherwise wait a bit longer
    while d.status != 'idle':
        time.sleep(0.1)

    #Stop the receiver after we got the frames
    #Detector is already idle so we don't need to stop it
    d.stop_receiver()

    lost = d.frames_caught - n
    print(f'{n} frames of {t}s took {time.time()-t0:{.3}}s with {lost} frames lost ')

    #Reset to not interfere with a potential next measurement
    d.reset_frames_caught()

Instead launching d.acq() from a different process is a bit easier since the control of receiver and detector
is handled in the acq call. However, you need to join the process used otherwise a lot of zombie processes would
hang around until the main process exits.

::

    import time
    from multiprocessing import Process
    from sls_detector import Eiger

    def acquire():
        """
        Create a new Eiger object that still referes to the same actual detector
        and same shared memory. Then launch acq.
        """
        detector = Eiger()
        detector.acq()

    #This is the detector we use throughout the session
    d = Eiger()

    #Process to run acquire
    p = Process(target=acquire)

    #Start the thread and short sleep to allow the acq to start
    p.start()
    time.sleep(0.01)

    #Do some other work
    while d.busy is True:
        print(d.busy)
        time.sleep(0.1)

    #Join the process
    p.join()