.. slsDetectorPackage documentation master file, created by
   sphinx-quickstart on Mon Jul 29 17:38:15 2019.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to slsDetectorPackage's documentation!
==============================================

.. note :: 

    This is the documentation for the latest development version of slsDetectorPackage.
    For further documentation, visit the official page: https://www.psi.ch/en/detectors/documentation

.. toctree::
    :maxdepth: 3
    :caption: Installation:

    installation
    dependencies
    consuming
   
.. toctree::
    :caption: C++ API
    :maxdepth: 2

    detector
    result
    receiver_api
    examples

.. toctree::
    :caption: Python API
    :maxdepth: 2

    pygettingstarted
    pydetector
    pyenums
    pyexamples

.. toctree::
    :caption: Command line
    :maxdepth: 2

    commandline  
    quick_start_guide

.. toctree::
    :caption: Developer

    container_utils
    type_traits
    ToString

.. toctree::
    :caption: Firmware
    :maxdepth: 2

    firmware 

.. toctree::
    :caption: Detector Server
    :maxdepth: 2

    servers
    serverupgrade
    virtualserver
    serverdefaults


.. toctree::
    :caption: Detector UDP Header
    :maxdepth: 2

    udpheader
    udpconfig
    udpdetspec

.. toctree::
    :caption: Receiver
    :maxdepth: 2

    receivers
    slsreceiver

.. toctree::
    :caption: Receiver Files
    :maxdepth: 3

    fileformat
    slsreceiverheaderformat
    masterfileattributes
    binaryfileformat
    hdf5fileformat

.. toctree::
    :caption: Receiver ZMQ Stream
    :maxdepth: 2

    zmqjsonheaderformat

.. toctree::
    :caption: Troubleshooting

    troubleshooting


.. Indices and tables
.. ==================

.. * :ref:`genindex`
.. * :ref:`modindex`
.. * :ref:`search`
