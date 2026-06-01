SLS Detector Package {{RELEASE_TYPE}} Release {{VERSION}} released on {{DATE}}
===============================================================

This document describes the differences between v{{VERSION}} and v{{PREVIOUS_VERSION}} 



    CONTENTS
    --------
    1           New, Changed or Resolved Features
        1.1     Compilation
        1.2     Callback
        1.3     Python
        1.4     Client
        1.5     Detector Server
        1.6     Simulator
        1.7     Receiver
        1.8     Gui
    2           On-board Detector Server Compatibility
    3           Firmware Requirements
    4           Kernel Requirements
    5           Download, Documentation & Support



1 New, Changed or Resolved Features
=====================================

Building shared libraries is disabled by default. If you need to link 
against any of the libSls*.so libraries, you can enable this by passing
 -DSLS_BUILD_SHARED_LIBRARIES=ON to CMake.
 
Added SLS_USE_SYSTEM_ZMQ option (default OFF) to use the libzmq of the host
instead of the one included in our repo. 

Experimental support for building the detector client (including python bindings) on macOS

``rx_dbitlist`` keeps the order of the passed bit list 

Detector.pattern (python) accepts also a pattern object, not only a pattern file

added patternstart to python (ctb, xilinx_ctb , mythen3), only the detector class api was exposed (startPattern())

documentation for all branches and developer now online and build and pushed automatically

gui: mouse zooms not reset at start of acquisition

m3: fixed patwaittime in intervals, which is hardly used. patwaittime in clocks stays the same and working.

m3: getPatternFileName typo fixed

support for building rpms

removed unused function readDataFile/writeDataFile from file_utils.h

added rx_streamdummyheader to send the zmq dummy header any time. Allows pre-configuring zmq processing before acq begins.


2  On-board Detector Server Compatibility
==========================================


    Eiger       10.0.0
    Jungfrau    10.0.0
    Mythen3     10.0.0
    Gotthard2   10.0.0
    Moench      10.0.0
  

    On-board Detector Server Upgrade
    --------------------------------

    From v6.1.0 (without tftp):
        update only on-board detector server
            Using command 'updatedetectorserver'


        udpate both on-board detector server and firmware simultaneously
            Using command 'update'

    Instructions available at
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/serverupgrade.html




3 Firmware Requirements
========================

    Eiger       02.10.2023 (v32)                    (updated in 7.0.3)
    
    Jungfrau    09.02.2025 (v1.6, HW v1.0)          (updated in 9.1.0)
                08.02.2025 (v2.6, HW v2.0)          (updated in 9.1.0)

    Mythen3     13.11.2024 (v2.0)                   (updated in 9.0.0)

    Gotthard2   03.10.2024 (v1.0)                   (updated in 9.0.0)

    Moench      26.10.2023 (v2.0)                   (updated in 8.0.2)


    Detector Upgrade
    ----------------

    The following can be upgraded remotely:

    Eiger      via bit files
    Jungfrau   via command <.pof>
    Mythen3    via command <.rbf>
    Gotthard2  via command <.rbf>
    Moench     via command <.pof>

    Except Eiger, 
        upgrade 
            Using command 'programfpga' or

        udpate both on-board detector server and firmware simultaneously
            Using command 'update'


    Instructions available at
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/firmware.html




4 Kernel Requirements
======================


    Blackfin 
    --------
    Latest version: Fri Oct 29 00:00:00 2021
    
    Older ones will work, but might have issues with programming firmware via
    the package.


    Nios
    -----
    Compatible version: Mon May 10 18:00:21 CEST 2021


    Kernel Upgrade
    ---------------
    Eiger   via bit files
    Others  via command

    Commands: udpatekernel, kernelversion
    Instructions available at
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/commandline.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/detector.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/pydetector.html




5 Download, Documentation & Support
====================================

    Download
    --------
    
    The Source Code:
         https://github.com/slsdetectorgroup/slsDetectorPackage
            
    Documentation
    -------------
    
    Installation:
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/installation.html

    Quick Start Guide:
        https://slsdetectorgroup.github.io//{{VERSION}}/quick_start_guide.html

    Firmware Upgrade:
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/firmware.html

    Detector Server upgrade:
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/serverupgrade.html

    Detector Simulators:
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/virtualserver.html

    Consuming slsDetectorPackage:
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/consuming.html
        
    Software Architecture
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/softwarearchitecture.html

    Set up commands in config file
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/configcommands.html
        
    Image Size and Output Characteristics
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/dataformat.html

    API Examples:
        https://github.com/slsdetectorgroup/api-examples

    Command Line Documentation:
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/commandline.html

    C++ API Documentation:
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/detector.html
       
    C++ API Example:
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/examples.html#
        
    Python API Documentation:
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/pygettingstarted.html

    Python API Example:
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/pyexamples.html

    Receivers (including custom receiver):
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/receivers.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/slsreceiver.html

    Detector UDP Header:
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/udpheader.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/udpdetspec.html

    Output Data:
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/dataformat.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/fileformat.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/slsreceiverheaderformat.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/masterfileattributes.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/binaryfileformat.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/hdf5fileformat.html

    slsReceiver Zmq Format:
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/slsreceiver.html#zmq-json-header-format

    TroubleShooting:
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/troubleshooting.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/troubleshooting.html#receiver-pc-tuning-options
        
    Further Documentation:
        https://www.psi.ch/en/detectors/documentation
        
    Info on Releases:
        https://slsdetectorgroup.github.io/slsDetectorPackage/index.html


    Support
    -------

        dhanya.thattil@psi.ch
        erik.frojdh@psi.ch
        alice.mazzoleni@psi.ch
