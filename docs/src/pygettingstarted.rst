Getting Started 
==================


--------------------
Which Python?  
--------------------

We require at lest Python 3.6 and strongly recommended that you don't use the system
Python installation. The examples in this documentation uses `conda
<https://docs.conda.io/en/latest/miniconda.html>`_ since it provides good support
also for non Python packages but there are also other alternatives like, pyenv. 

Using something like conda also allows you to quickly switch beteen different Python 
environments. 

.. note ::

    Ensure that the python lib compiled is for the expected python version.
    For example, build/bin/_slsdet.cpython-39-x86_64-linux-gnu.so for Python v3.9.x

---------------------
PYTHONPATH 
---------------------

If you install slsdet using conda everything is set up and you can
directly start using the Python bindings. However, if you build 
from source you need to tell Python where to find slsdet. This
is be done by adding your build/bin directory to PYTHONPATH. 

.. code-block:: bash

    export PYTHONPATH = /path/to/your/build/bin:$PYTHONPATH

.. note ::

    Don't forget to compile with the option SLS_USE_PYTHON=ON to enable
    the Python bindings or if you use the cmk.sh script -p.

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
        :getter: always returns in seconds. To get in datetime.delta, use getPeriod

        Examples
        -----------
        >>> d.period = 1.05
        >>> d.period = datetime.timedelta(minutes = 3, seconds = 1.23)
        >>> d.period
        181.23
        >>> d.getPeriod()
        [datetime.timedelta(seconds=181, microseconds=230000)]


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
    ['burstMode', 'clockIndex', 'dacIndex', 
    'detectorSettings', 'detectorType', 'dimension', 'externalSignalFlag', 
    'fileFormat', 'frameDiscardPolicy', 
    'readoutMode', 'runStatus', 'speedLevel', 'timingMode', 
    'timingSourceType']

    # Even though importing using * is not recommended one could
    # get all the enums like this: 
    >>> from slsdet.enums import *
