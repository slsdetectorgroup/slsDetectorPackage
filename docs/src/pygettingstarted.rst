Getting Started 
==================


--------------------
Which Python?  
--------------------

We require at least Python 3.8 and strongly recommended that you don't use the system
Python installation. The examples in this documentation uses `conda
<https://docs.conda.io/en/latest/miniconda.html>`_ since it provides good support
also for non Python packages but there are also other alternatives like, pyenv. 

Using something like conda also allows you to quickly switch beteen different Python 
environments. 

---------------------
Building from Source 
---------------------

If you are not installing slsdet binaries from conda, but instead building from 
source, please refer to  :ref:`the installation section<Installation>` for details.

Don't forget to compile with the option SLS_USE_PYTHON=ON to enable the Python 
bindings or if you use the cmk.sh script -p.

.. note ::

    Ensure that the sls det python lib compiled is for the expected python version.
    For example, build/bin/_slsdet.cpython-39-x86_64-linux-gnu.so for Python v3.9.x


---------------------
PYTHONPATH 
---------------------

If you install slsdet binaries using conda everything is set up and you can
directly start using the Python bindings. However, if you build 
from source you need to tell Python where to find slsdet to use it. This
can be done by adding your build/bin directory to PYTHONPATH. 

.. code-block:: bash

    export PYTHONPATH = /path/to/your/build/bin:$PYTHONPATH

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

To make the Python API approachable, both if you come from the command line 
or are using the C++ API, we provide two interfaces to the detector. 
One is property based and tries to stay as close to the command line syntax
as is possible, and the other one directly maps the C++ API found in Detector.h.
There is also an underlying design reason for the two APIs since we auto 
generate the bindings to the C++ code using a mix of pybind11 and clang-tools. 
The property based API covers most of the functionality but in some cases 
you have to reach for the C++ like interface. 


::  

    d = Detector()

    # C++ like API 
    d.setExptime(0.1)

    # or a bit more pythonic
    d.exptime = 0.1

The c++ style API offers more control over access to individual modules
in a large detector.

:: 

    # Set exposure time for module 1, 5 and 7
    d.setExptime(0.1, [1,5,7])



.. _py-module-index-label:

----------------------------------
Accessing individual modules
----------------------------------

Using the C++ like API you can access individual modules in a large detector
by passing in the module index as an argument to the function.

::
    
    # Read the UDP destination port for all modules
    >>> d.getDestinationUDPPort()
    [50001, 50002, 50003]


    # Read it for module 0 and 1
    >>> d.getDestinationUDPPort([0, 1])
    [50001, 50002]

    >>> d.setDestinationUDPPort(50010, 1)
    >>> d.getDestinationUDPPort()
    [50001, 50010, 50003]

From the more pythonic API there is no way to read from only one module but you can read 
and then use list slicing to get the values for the modules you are interested in.

::

    >>> d.udp_dstport
    [50001, 50010, 50003]
    >>> d.udp_dstport[0]
    50001

    #For some but not all properties you can also pass in a dictionary with module index as key
    >>> ip = IpAddr('127.0.0.1')
    >>> d.udp_dstip = {1:ip}


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
    'clearBit', 'clearROI', 'client_version', 'config',  
    'counters', 'daclist', 'dacvalues', 'dbitclk', 'dbitphase' ...

Since the list for Detector is rather long it's an good idea to filter it. 
The following example gives you properties and methods containing time in 
their name.

:: 

    >>> [item for item in dir(d) if 'time' in item]
    ['compdisabletime', 'exptime', 'exptimel', 'frametime', 'getExptime', 
    'getExptimeForAllGates', 'getExptimeLeft', 'getSubExptime', 'patwaittime', 
    'patwaittime0', 'patwaittime1', 'patwaittime2', 'runtime', 'setExptime', 
    'setSubExptime', 'subdeadtime', 'subexptime']


The above method works on any Python object but for convenience we also 
included two functions to find names. View prints the names one per line
while find returns a list of names. 

::

    from slsdet.lookup import view, find

    >>> view('exptime')
    exptime
    exptimel
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


------------------------------------
Finding out what the function does
------------------------------------

To access the documentation of a function directly from the Python prompt use help(). 

.. code-block :: python

    >>> help(Detector.period)
    Help on property:

        Period between frames, accepts either a value in seconds or datetime.timedelta
        
        Note
        -----
        :getter: always returns in seconds. To get in DurationWrapper, use getPeriod
        
        Example
        -----------
        >>> # setting directly in seconds
        >>> d.period = 1.05
        >>>
        >>> # setting directly in seconds
        >>> d.period = 5e-07
        >>> 
        >>> # using timedelta (up to microseconds precision)
        >>> from datatime import timedelta
        >>> d.period = timedelta(seconds = 1, microseconds = 3)
        >>> 
        >>> # using DurationWrapper to set in seconds
        >>> from slsdet import DurationWrapper
        >>> d.period = DurationWrapper(1.2)
        >>> 
        >>> # using DurationWrapper to set in ns
        >>> t = DurationWrapper()
        >>> t.set_count(500)
        >>> d.period = t
        >>>
        >>> # to get in seconds
        >>> d.period
        181.23
        >>> 
        >>> d.getExptime()
        [sls::DurationWrapper(total_seconds: 181.23 count: 181230000000)]



----------------------
Where are the ENUMs?
----------------------

To set some of the detector settings like file format you have
to pass in an enum. 

:: 

    >>> d.setFileFormat(fileFormat.BINARY)
    
The enums can be found in slsdet.enums 

::

    import slsdet
    >>> [e for e in dir(slsdet.enums) if not e.startswith('_')]
    ['M3_GainCaps', 'burstMode', 'clockIndex', 'cls', 'dacIndex', 'detectorSettings', 
    'detectorType', 'dimension', 'externalSignalFlag', 'fileFormat', 
    'frameDiscardPolicy', 'gainMode', 'name', 'polarity', 'portPosition', 
    'readoutMode', 'runStatus', 'speedLevel', 'streamingInterface', 'timingMode', 
    'timingSourceType', 'vetoAlgorithm']


    # Even though importing using * is not recommended one could
    # get all the enums like this: 
    >>> from slsdet.enums import *
