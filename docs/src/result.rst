Result
==============================================

sls::Result is a thin wrapper around std::vector and used for returning values
from the detector. Since every module could have a different value, we need
to return a vector instead of just a single value. 

Easy conversions to single values are provided using the squash method.


.. doxygenfile:: Result.h
