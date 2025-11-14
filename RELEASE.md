SLS Detector Package Minor Release 10.0.1 released on ?
================================================================

This document describes the differences between v10.0.1 and v10.0.0   



    CONTENTS
    --------
    1           Changes
    1.1         Compilation Changes
    1.2         New or Changed Features
        1.2.1   Breaking API
        1.2.2   Resolved or Changed Features
        1.2.3   New Features
    3           On-board Detector Server Compatibility
    4           Firmware Requirements
    5           Kernel Requirements
    6           Download, Documentation & Support



1 Changes
==========


1.1 Compilation Changes
========================


1.2 New or Changed Features
============================

    Python
    -------

    * receiver ROI can be set from Python using command ``rx_roi``(it supports any sequence of four ints e.g. a tuple (xmin, xmax, ymin, ymax) or a sequence of such for multiple ROIS)
    * one can clear all ROI's from Python using command ``clear_rx_roi`` 


1.2.1 Breaking API
===================


1.2.2 Resolved or Changed Features
===================================



1.2.3 New Features
===================


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
        https://slsdetectorgroup.github.io/devdoc/serverupgrade.html




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
        https://slsdetectorgroup.github.io/devdoc/firmware.html




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
        https://slsdetectorgroup.github.io/devdoc/commandline.html
        https://slsdetectorgroup.github.io/devdoc/detector.html
        https://slsdetectorgroup.github.io/devdoc/pydetector.html




5 Download, Documentation & Support
====================================

    Download
    --------
    
    The Source Code:
         https://github.com/slsdetectorgroup/slsDetectorPackage
            
    Documentation
    -------------
    
    Installation:
        https://slsdetectorgroup.github.io/devdoc/installation.html

    Quick Start Guide:
        https://slsdetectorgroup.github.io/devdoc/quick_start_guide.html

    Firmware Upgrade:
        https://slsdetectorgroup.github.io/devdoc/firmware.html

    Detector Server upgrade:
        https://slsdetectorgroup.github.io/devdoc/serverupgrade.html

    Detector Simulators:
        https://slsdetectorgroup.github.io/devdoc/virtualserver.html

    Consuming slsDetectorPackage:
        https://slsdetectorgroup.github.io/devdoc/consuming.html
        
    Software Architecture
        https://slsdetectorgroup.github.io/devdoc/softwarearchitecture.html

    Set up commands in config file
        https://slsdetectorgroup.github.io/devdoc/configcommands.html
        
    Image Size and Output Characteristics
        https://slsdetectorgroup.github.io/devdoc/dataformat.html

    API Examples:
        https://github.com/slsdetectorgroup/api-examples

    Command Line Documentation:
        https://slsdetectorgroup.github.io/devdoc/commandline.html

    C++ API Documentation:
        https://slsdetectorgroup.github.io/devdoc/detector.html
       
    C++ API Example:
        https://slsdetectorgroup.github.io/devdoc/examples.html#
        
    Python API Documentation:
        https://slsdetectorgroup.github.io/devdoc/pygettingstarted.html

    Python API Example:
        https://slsdetectorgroup.github.io/devdoc/pyexamples.html

    Receivers (including custom receiver):
        https://slsdetectorgroup.github.io/devdoc/receivers.html
        https://slsdetectorgroup.github.io/devdoc/slsreceiver.html

    Detector UDP Header:
        https://slsdetectorgroup.github.io/devdoc/udpheader.html
        https://slsdetectorgroup.github.io/devdoc/udpdetspec.html

    Output Data:
        https://slsdetectorgroup.github.io/devdoc/dataformat.html
        https://slsdetectorgroup.github.io/devdoc/fileformat.html
        https://slsdetectorgroup.github.io/devdoc/slsreceiverheaderformat.html
        https://slsdetectorgroup.github.io/devdoc/masterfileattributes.html
        https://slsdetectorgroup.github.io/devdoc/binaryfileformat.html
        https://slsdetectorgroup.github.io/devdoc/hdf5fileformat.html

    slsReceiver Zmq Format:
        https://slsdetectorgroup.github.io/devdoc/slsreceiver.html#zmq-json-header-format

    TroubleShooting:
        https://slsdetectorgroup.github.io/devdoc/troubleshooting.html
        https://slsdetectorgroup.github.io/devdoc/troubleshooting.html#receiver-pc-tuning-options
        
    Further Documentation:
        https://www.psi.ch/en/detectors/documentation
        
    Info on Releases:
        https://www.psi.ch/en/detectors/software


    Support
    -------

        dhanya.thattil@psi.ch
        erik.frojdh@psi.ch
        alice.mazzoleni@psi.ch
