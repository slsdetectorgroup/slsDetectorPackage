SLS Detector Package Bug Fix Release 10.0.1 released on 2026-10-07
===============================================================

This document describes the differences between v10.0.1 and v10.0.0 



    CONTENTS
    --------
    1           New, Changed or Resolved Features
        1.1     Compilation
        1.2     Receiver
        1.3     Package Distribution
    2           On-board Detector Server Compatibility
    3           Firmware Requirements
    4           Kernel Requirements
    5           Download, Documentation & Support



1 New, Changed or Resolved Features
=====================================

1.1 Compilation Changes
========================

* Added CPack RPM packaging for CLI, receiver, Moench tools, GUI and zmq streaming.
* Compiler option SLS_INSTALL_VERSIONED_BINARIES=ON adds the project version to RPM
  package names and all installed executable names, including the GUI,
  Jungfrau tools and virtual detector servers, for side-by-side use.

1.2 Receiver
=============

* fixed race condition in slsReceiver potentially leading to corrupted data e.g. frame duplications, torn frames, header-only frames with missing data - leading to misalignments in file
    * Data corruption could have happened only if using discard policy `DISCARD_PARTIAL_FRAMES` or `DISCARD_EMPTY_FRAMES` 
    taken at high frame rates and if packet loss occured 
    * bug does not apply for receiver discard policy `NO_DISCARD` 


1.3 Package Distribution
=========================

RPM packages are for CLI, Receiver, Moench tools and GUI and zmq streaming provided for the folowing platforms

* RHEL9/Enterprise Linux 9 - x86_64
* RHEL8/Enterprise Linux 8 - x86_64

The RPM can be downloaded from https://gitea.psi.ch/detectors/-/packages 

2  On-board Detector Server Compatibility
==========================================

No need to update Detector Servers. Software changes are compatible with Detector Server version 10.0.0. 


3 Firmware Requirements
========================

No need to update on-board Detector-Server firmware. 

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
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/firmware.html




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
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/commandline.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/detector.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/pydetector.html




5 Download, Documentation & Support
====================================

    Download
    --------
    
    The Source Code:
         https://github.com/slsdetectorgroup/slsDetectorPackage

    RPM Packages: 
        https://gitea.psi.ch/detectors/-/packages
            
    Documentation
    -------------
    
    Installation:
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/installation.html

    Quick Start Guide:
        https://slsdetectorgroup.github.io//10.0.1/quick_start_guide.html

    Firmware Upgrade:
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/firmware.html

    Detector Server upgrade:
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/serverupgrade.html

    Detector Simulators:
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/virtualserver.html

    Consuming slsDetectorPackage:
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/consuming.html
        
    Software Architecture
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/softwarearchitecture.html

    Set up commands in config file
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/configcommands.html
        
    Image Size and Output Characteristics
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/dataformat.html

    API Examples:
        https://github.com/slsdetectorgroup/api-examples

    Command Line Documentation:
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/commandline.html

    C++ API Documentation:
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/detector.html
       
    C++ API Example:
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/examples.html#
        
    Python API Documentation:
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/pygettingstarted.html

    Python API Example:
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/pyexamples.html

    Receivers (including custom receiver):
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/receivers.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/slsreceiver.html

    Detector UDP Header:
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/udpheader.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/udpdetspec.html

    Output Data:
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/dataformat.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/fileformat.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/slsreceiverheaderformat.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/masterfileattributes.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/binaryfileformat.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/hdf5fileformat.html

    slsReceiver Zmq Format:
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/slsreceiver.html#zmq-json-header-format

    TroubleShooting:
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/troubleshooting.html
        https://slsdetectorgroup.github.io/slsDetectorPackage/10.0.1/troubleshooting.html#receiver-pc-tuning-options
        
    Further Documentation:
        https://www.psi.ch/en/detectors/documentation
        
    Info on Releases:
        https://slsdetectorgroup.github.io/slsDetectorPackage/index.html


    Support
    -------

        dhanya.thattil@psi.ch
        erik.frojdh@psi.ch
        alice.mazzoleni@psi.ch
