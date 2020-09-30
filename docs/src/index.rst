.. slsDetectorPackage documentation master file, created by
   sphinx-quickstart on Mon Jul 29 17:38:15 2019.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to slsDetectorPackage's documentation!
==============================================

.. note :: 

    This is the documentation for the latest development version of slsDetectorPackage
    For documentation on current and previous releases visit the official page: https://www.psi.ch/en/detectors/documentation

.. toctree::
    :maxdepth: 1
    :caption: Installation:

    installation
    dependencies
    consuming
   
.. toctree::
    :caption: C++ API
    :maxdepth: 2

    detector
    result
    receiver
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

.. toctree::
    :caption: Developer

    container_utils
    type_traits
    ToString

.. toctree::
    :caption: Firmware

    firmware 

.. toctree::
    :caption: Detector Server

    servers
    serverupgrade
    virtualserver



.. Indices and tables
.. ==================

.. * :ref:`genindex`
.. * :ref:`modindex`
.. * :ref:`search`
