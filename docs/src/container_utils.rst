ContainerUtils
==================

Helper functions to handle standard container compliant 
containers. Supports array, vector, sls::Result etc.

While not a part of the public API we aim not to change this
interface too much. However, we don't give the same strong
guarantees as for Detector etc. 

Any reoccurring container operation should probably be added to
this file.  

.. doxygenfile:: container_utils.h