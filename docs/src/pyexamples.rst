Examples
================

Some short examples on how to use slsdet. If something is missing don't hesitate to
open an issue in our our  `github repo
<https://github.com/slsdetectorgroup/slsDetectorPackage>`_. 


------------------------------------
Setting exposure time 
------------------------------------

Setting and reading back exposure time can be done either using a Python datetime.timedelta
or by setting the time in seconds. 

::

    # Set exposure time to 1.2 seconds
    >>> d.exptime = 1.2

    # Setting exposure time using timedelta
    import datetime as dt
    >>> d.exptime = dt.timedelta(seconds = 1.2)

    # With timedelta any arbitrary combination of units can be used
    >>> t = dt.timedelta(microseconds = 100, seconds = 5.3, minutes = .3)

    # To set exposure time for individual detector one have to resort
    # to the C++ style API.
    # Sets exposure time to 1.2 seconds for module 0, 6 and 12
    >>> d.setExptime(1.2, [0, 6, 12]) 
    >>> d.setExptime(dt.timedelta(seconds = 1.2), [0, 6, 12]) 



------------------------------------
Converting numbers to hex
------------------------------------

Python support entering numbers in  format by using the 0x prefix. However, when reading 
back you will get a normal integer. This can then be converted to a hex string representation
using the built in hex() function. 

.. code-block :: python

    from slsdet import Detector
    >>> d = Detector()
    >>> d.patwait0 = 0xaa
    >>> d.patwait0
    170

    # Convert to  string
    >>> hex(d.patwait0)
    '0xaa'

For multiple values one can use a list comprehension to loop over the values. 

.. code-block :: python

    >>> values = [1,2,3,4,5]
    >>> [(v) for v in values]
    ['0x1', '0x2', '0x3', '0x4', '0x5']

    # or to a single string by passing the list to .join
    >>> ', '.join([hex(v) for v in values])
    '0x1, 0x2, 0x3, 0x4, 0x5'



    

------------------------
Simple threshold scan
------------------------

Assuming you have set up your detector with exposure time, period, enabled
file writing etc.

.. code-block:: python
 
    from slsdet import Eiger

    d = Eiger()
    threshold = range(0, 2000, 200)
    for th in threshold:
        d.vthreshold = th
        d.acquire()
    

If we want to control the shutter of for example, the big X-ray box we can add
this line in our code. It then opens the shutter just before the measurement
and closes is afterwards.
    
::

    with xrf_shutter_open(box, 'Fe'):
        for th in threshold:
            d.vthreshold = th
            d.acquire()
        
        
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



Instead launching d.acq() from a different process is a bit easier since the control of receiver and detector
is handled in the acq call. However, you need to join the process used otherwise a lot of zombie processes would
hang around until the main process exits.

::

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


------------------------------
Setting and getting times
------------------------------

::

    import datetime as dt
    from slsdet import Detector
    from slsdet.utils import element_if_equal

    d = Detector()

    # The simplest way is to set the exposure time in 
    # seconds by using the exptime property
    # This sets the exposure time for all modules
    d.exptime = 0.5

    # exptime also accepts a python datetime.timedelta
    # which can be used to set the time in almost any unit
    t = dt.timedelta(milliseconds = 2.3)
    d.exptime = t

    # or combination of units
    t = dt.timedelta(minutes = 3, seconds = 1.23)
    d.exptime = t

    # exptime however always returns the time in seconds
    >>> d.exptime
    181.23 

    # To get back the exposure time for each module 
    # it's possible to use getExptime, this also returns
    # the values as datetime.timedelta

    >>> d.getExptime()
    [datetime.timedelta(seconds=181, microseconds=230000), datetime.timedelta(seconds=181, microseconds=230000)]

    # In case the values are the same it's possible to use the
    # element_if_equal function to reduce the values to a single 
    # value

    >>> t = d.getExptime()
    >>> element_if_equal(t)
    datetime.timedelta(seconds=1)

--------------
Reading dacs
--------------

::

    from slsdet import Detector, Eiger, dacIndex

    #using the specialized class
    e = Eiger()
    >>> e.dacs
    ========== DACS =========
    vsvp      :    0    0
    vtrim     : 2480 2480
    vrpreamp  : 3300 3300
    vrshaper  : 1400 1400
    vsvn      : 4000 4000
    vtgstv    : 2556 2556
    vcmp_ll   : 1000 1000
    vcmp_lr   : 1000 1000
    vcal      :    0    0
    vcmp_rl   : 1000 1000
    rxb_rb    : 1100 1100
    rxb_lb    : 1100 1100
    vcmp_rr   : 1000 1000
    vcp       : 1000 1000
    vcn       : 2000 2000
    vishaper  : 1550 1550
    iodelay   :  650  650

    # or using the general class and the list
    d = Detector()
    for dac in d.daclist:
        r = d.getDAC(dac, False)
        print(f'{dac.name:10s} {r}')
