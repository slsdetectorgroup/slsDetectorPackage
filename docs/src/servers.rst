Detector Servers
=================

Location
---------
   slsDetectorPackage/serverBin/ folder in every release.


.. _Detector Server Arguments:

Arguments
---------

   .. code-block:: bash  

      Possible arguments are:
      -v, --version            : Software version
      -p, --port <port>        : TCP communication port with client. 
      -g, --nomodule           : [Mythen3][Gotthard2] Generic or No Module mode. 
                                 Skips detector type checks.
      -f, --phaseshift <value> : [Gotthard] only. Sets phase shift. 
      -d, --devel              : Developer mode. Skips firmware checks. 
      -u, --update             : Update mode. Skips firmware checks and initial detector setup. 
      -s, --stopserver         : Stop server. Do not use as it is created by control server 


Basics
------------

Detector Servers include:
   * Control server [default port: 1952]
      * Almost all client communication.
   * Stop server [default port: 1953]
      *  Client requests for detector status, stop acquisition, temperature, advanced read/write registers.

When using a blocking acquire command (sls_detector_acquire or Detector::acquire), the control server is blocked until end of acquisition. However, stop server commands could be used in parallel.
