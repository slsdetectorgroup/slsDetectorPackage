Detector
==============================================

The sls::Detector is the new public API to control 
detectors from C++. This API is also used internally
for the Python bindings and the command line interface. 
If a receiver has been configured this is also controlled
through this class.

Most, if not all, functions are called in parallel
and the return value is a thin std::vector wrapper 
containing results from all modules. (Result<T>)

.. doxygenclass:: sls::Detector
   :members:
   :undoc-members: