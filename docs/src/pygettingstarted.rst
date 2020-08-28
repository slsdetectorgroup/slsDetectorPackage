Getting Started 
==================

--------------------------------------
Which detector class should I use? 
--------------------------------------

We provide a generic class called Detector and detector specific 
versions like, Eiger, Jungfrau etc. The most or all functionality 
is there in the base class except the convenient access to dacs
and temperatures. 

:: 

    from slsdet import Detector, Eiger

    d = Detector()
    e = Eiger()

    # Both classes can be used to control an Eiger detector
    d.exptime = 0.5
    e.period = 1

    # But Eiger gives a simpler interface to the dacs
    >>> e.dacs
    ========== DACS =========
    vsvp           :    0
    vtrim          : 2480
    vrpreamp       : 3300
    vrshaper       : 1400
    vsvn           : 4000
    vtgstv         : 2556
    vcmp_ll        : 1000
    vcmp_lr        : 1000
    vcal           :    0
    vcmp_rl        : 1000
    rxb_rb         : 1100
    rxb_lb         : 1100
    vcmp_rr        : 1000
    vcp            : 1000
    vcn            : 2000
    vishaper       : 1550
    iodelay        :  650


.. note ::

    Depending on user feedback we might move some detector specific
    functionality to the specialized classes.


----------------------------------
Hey, there seems to be two APIs?
----------------------------------

To make the Python approachable for both command line users and people 
used to the C++ API we provide both a property based API similar to the 
command line and a direct copy of the C++ API. There is also an underlying
design reason for the two APIs since we auto generate the bindings to
the C++ code. 

::  

    d = Detector()

    # C++ like API 
    d.setExptime(0.1)

    # or a bit more pythonic
    d.exptime = 0.1

The c++ style API offers a bit more control over custom access to modules
in a large detector.

:: 

    # Set exposure time for module 1, 5 and 7
    d.setExptime(0.1, [1,5,7])

--------------------
Finding functions 
--------------------

To find out which properties and methods that a Python object have you
can use dir()

::

    >>> from slsdet import Detector
    >>> d = Detector()
    >>> dir(d)
    ['__class__', '__delattr__', '__dict__', '__dir__', '__doc__', 
    '__eq__', '__format__', '__ge__', '__getattribute__', '__gt__', 
    '__hash__', '__init__', '__init_subclass__', '__le__', '__len__', 
    '__lt__', '__module__', '__ne__', '__new__', '__reduce__', 
    '__reduce_ex__', '__repr__', '__setattr__', '__sizeof__', 
    '__str__', '__subclasshook__', '_adc_register', '_frozen', 
    '_register', 'acquire', 'adcclk', 'adcphase', 'adcpipeline', 
    'adcreg', 'asamples', 'auto_comp_disable', 'clearAcquiringFlag', 
    'clearBit', 'clearROI', 'client_version', 'config', 'copyDetectorServer', 
    'counters', 'daclist', 'dacvalues', 'dbitclk', 'dbitphase' ...

Since the list for Detector is rather long it's an good idea to filter it. 
This list comprehension gives you properties and methods containing time in 
their name.

:: 

    >>> [item for item in dir(d) if 'time' in item]
    ['exptime', 'getExptime', 'getExptimeForAllGates', 'getExptimeLeft', 
    'getSubExptime', 'patwaittime0', 'patwaittime1', 'patwaittime2', 
    'setExptime', 'setSubExptime', 'subdeadtime', 'subexptime']

The above method works on any Python object but for convenience we also 
included two functions to find names. View prints the names one per line
while find returns a list of names. 

::

    from slsdet.lookup import view, find

    >>> view('exptime')
    exptime
    getExptime
    getExptimeForAllGates
    getExptimeLeft
    getSubExptime
    setExptime
    setSubExptime
    subexptime

    >>> find('exptime')
    ['exptime', 'getExptime', 'getExptimeForAllGates', 'getExptimeLeft', 
    'getSubExptime', 'setExptime', 'setSubExptime', 'subexptime']


