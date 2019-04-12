Error handling
=========================


Check input in Python
----------------------

As far as possible we try to check the input on the Python side
before calling the slsDeteectorsSoftware. Errors should not pass
silently but raise an exception

::

    #Trimbit range for Eiger is 0-63
    detector.trimbits = 98
    (...)
    ValueError: Trimbit setting 98 is  outside of range:0-63
    
Errors in slsDetectorsSoftware
-------------------------------

The slsDetectorsSoftware uses a mask to record errors from the different
detectors. If an error is found we raise a RuntimeError at the end of the 
call using the error message from slsDetectorsSoftware

.. todo ::

    Implement this for all functions

::

    detector.settings = 'bananas'
    (...)
    RuntimeError: Detector 0:
    Could not set settings.
    Detector 1:
    Could not set settings.
    Detector 2:
    Could not set settings.
    
    
Using decorators
-------------------

Using decorators we can reset the error mask before the command and then 
check it after the command

.. code-block:: python

    #add decorator to check the error mask
    @error_handling
    def some_function():
        a = 1+1
        return a

Communication with the detector is usually the biggest overhead so 
this does not impact performance. 

::

    %timeit d.exposure_time
    >> 1.52 ms ± 5.42 µs per loop (mean ± std. dev. of 7 runs, 1000 loops each)

    %timeit d.decorated_exposure_time
    >> 1.53 ms ± 3.18 µs per loop (mean ± std. dev. of 7 runs, 1000 loops each)


