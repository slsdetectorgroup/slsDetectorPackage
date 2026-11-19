SLS Detector Package Minor Release 10.1.0 released on 2026-08-06
===============================================================

This document describes the differences between v10.1.0 and v10.0.1

## Contents

1. [New, Changed or Resolved Features](#1-new-changed-or-resolved-features)
    1. [Compilation](#11-compilation)
    2. [Callback](#12-callback)
    3. [Python](#13-python)
    4. [Client](#14-client)
    5. [Detector Server](#15-detector-server)
    6. [Simulator](#16-simulator)
    7. [Receiver](#17-receiver)
    8. [Gui](#18-gui)
2. [BugFixes](#2-bugfixes)
    1. [Python](#21-python)
    2. [Client](#22-client)
    3. [Detector Server](#23-detector-server)
    4. [Simulator](#24-simulator)
    5. [Receiver](#25-receiver)
    6. [Gui](#26-gui)
3. [On-board Detector Server Compatibility](#2-on-board-detector-server-compatibility)
4. [Firmware Requirements](#3-firmware-requirements)
5. [Kernel Requirements](#4-kernel-requirements)
6. [Download, Documentation & Support](#5-download-documentation--support)

## 1 New, Changed or Resolved Features

- zmqsocket - allowing hwm to be set already at the constructor level. As before, can still be set after construction with socket reconnection.

### 1.1 Compilation

- Unpacking `qwt` using cmake bundled tools removing the dependency on `bzip2`

- `libexpat` is no longer a dependency of slsDetectorPackage

- All servers are compiled with `-02` optimization level

- Fixed path to updateAPIVersion.py in server MakeFiles (only relevant if manually compiling servers)

- Building shared libraries is disabled by default. If you need to link against any of the `libSls*.so` libraries, you can enable this by passing `-DSLS_BUILD_SHARED_LIBRARIES=ON` to CMake.

- Added `SLS_USE_SYSTEM_ZMQ` option (default `OFF`) to use the libzmq of the host instead of the one included in the repo.

### 1.3 Python

- `Detector().pattern` (python) accepts also a `Pattern` object, not only a pattern file

- Added member function `patternstart` to python, only the detector class api was exposed (`startPattern()`)

### 1.4 Client

- removed unused function `readDataFile`/`writeDataFile` from `file_utils.h`

- Added command ``rx_streamdummyheader`` to send the zmq dummy header any time. Allows pre-configuring zmq processing before acq begins.

### 1.5 Detector Server

**Detector specific updates:**

#### 1.5.1 Jungfrau

New features are supported by the new Jungfrau Chip v1.2

- support for enabling High Dynamic Range (HDR) for Synchotron or FEL
- Enable reset step reduction
- default special dac values for G0 and G1 are pointing to different values
- Electron collection mode => reset and bypass cds
- filter resistor: default value is the lower resistor
- auto comp disable: disabled by default (as before but now explicitly set)
- compdisabletime defaults to 2 us

> **Note:** The Chip version1.2 does not allow setting extrastoragecells.


New features supported by the new Jungfrau Chip v1.1

- Electron collection mode => reset and bypass cds
- filter resistor: default value is the lower resistor
- 1.1+: changing filter resistor updates asic ds timer (0x1f for lower, 0x5f for higher)
- auto comp disable: disabled by default (as before but now explicitly set)
- compdisabletime defaults (?)

New features supported by Jungfrau Chip v1.0

- compdisabletime defaults 5us

New features for all chips: 

- The maximum allowed value for loops in pedestal mode has been increased from 8 bit to 16 bit (compatible with firmware >v9.1.0)


#### 1.5.2 Mythen3

- updating threshold energy can be done during an acquisition (only affects slsReceiver)

- updating threshold energy `ALL` can be done during an acquisition (only affects slsReceiver)

#### 1.5.3 Eiger

- updating threshold energy can be done during an acquisition (only affects slsReceiver)

- Setting ``vthreshold`` dac directly sets settings to undefind

### 1.7 Receiver

- added `rx_streamdummyheader` to send the zmq dummy header any time. Allows pre-configuring zmq processing before acq begins.
- allow disabling one UDP interface in the receiver.
- zmqsocket - allowing hwm to be set already at the constructor level. As before, can still be set after construction with socket reconnection. 

- Automatically resolve trailing ``/`` in user given filepaths 

### 1.8 Gui

## 2 BugFixes

### 2.1 Python

- Receiver ROI can be set from Python using command ``Detector().rx_roi``(it supports any sequence of four or two (for mythen3 and gotthard) ints e.g. a tuple (xmin, xmax, ymin, ymax) or a sequence of such for multiple ROIS)
- one can clear all ROI's from Python using command ``Detector().rx_clearroi()``

### 2.2 Client

- Fixed version comparison - previously lexicographic comparison on semantic version string was used (e.g. 9.0.0 > 10.0.0)

- Throws if user sets an roi to a disabled port or disable a port that has a restrictive roi (not complete detector).

- m3: `getPatternFileName` typo fixed

### 2.3 Detector Server

#### 2.3.1 Mythen3

- Downgrading required minimum kernel version for Nios to (Mon Jan 18 14:24:30 CET 2021) - as latest Nios kernel not required

- Remove excessive debug printing (only log in debug mode) speeding up setting trimval

- Fixed log info about bad channel when setting trimbits

- Fixed `patwaittime` in intervals, which is hardly used. `patwaittime` in clocks stays the same and working.

### 2.4 Simulator

### 2.5 Receiver

- Fixed potential deadlock if HDF5 creation throws 

- Fixed caught frame number in partial readout mode. (Previously caught frames always 0)

- Fixed proper enabling of filewrite after changing number of Interfaces (previously no proper file writing setup after changing number of Interfaces)

- Setting ``rx_roi`` only writes/streams data to file/over ``zmq`` within the ROI (previously Ports or modules not overlapping with a user defined ROI was also written/streamed)

- Field "Threshold Energies" in master file always updated to ``[-1, -1, -1]`` whenever it is set in the detector server (setting dacs directly, custom trimbit file, set all trimbits etc.)


### 2.6 Gui

- mouse zooms are not reset at start of acquisition

## 3 On-board Detector Server Compatibility

| Detector  | Version |
|-----------|---------|
| Eiger     | 10.0.0  |
| Jungfrau  | 10.0.0  |
| Mythen3   | 10.0.0  |
| Gotthard2 | 10.0.0  |
| Moench    | 10.0.0  |

### On-board Detector Server Upgrade

From v6.1.0 (without tftp):

- update only on-board detector server
    - Using command `updatedetectorserver`
- udpate both on-board detector server and firmware simultaneously
    - Using command `update`

Instructions available at
- https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/serverupgrade.html

## 4 Firmware Requirements

| Detector  | Firmware Date | Version          | Updated in |
|-----------|---------------|------------------|------------|
| Eiger     | 02.10.2023    | v32              | 7.0.3      |
| Jungfrau  | 09.02.2025    | v1.6, HW v1.0    | 9.1.0      |
| Jungfrau  | 08.02.2025    | v2.6, HW v2.0    | 9.1.0      |
| Mythen3   | 13.11.2024    | v2.0             | 9.0.0      |
| Gotthard2 | 03.10.2024    | v1.0             | 9.0.0      |
| Moench    | 26.10.2023    | v2.0             | 8.0.2      |

### Detector Upgrade

The following can be upgraded remotely:

| Detector  | Upgrade via        |
|-----------|--------------------|
| Eiger     | bit files          |
| Jungfrau  | command `<.pof>`   |
| Mythen3   | command `<.rbf>`   |
| Gotthard2 | command `<.rbf>`   |
| Moench    | command `<.pof>`   |

Except Eiger,

- upgrade
    - Using command `programfpga` or
- udpate both on-board detector server and firmware simultaneously
    - Using command `update`

Instructions available at
- https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/firmware.html

## 5 Kernel Requirements

### Blackfin

Latest version: Fri Oct 29 00:00:00 2021

Older ones will work, but might have issues with programming firmware via the package.

### Nios

Compatible version: Mon Jan 18 14:24:30 CEST 2021

### Kernel Upgrade

| Detector | Upgrade via |
|----------|-------------|
| Eiger    | bit files   |
| Others   | command     |

Commands: `udpatekernel`, `kernelversion`

Instructions available at
- https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/commandline.html
- https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/detector.html
- https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/pydetector.html

## 6 Download, Documentation & Support

### Download

- **The Source Code:**
    - https://github.com/slsdetectorgroup/slsDetectorPackage
- **RPM Packages:**
    - https://gitea.psi.ch/detectors/-/packages

### Documentation

- **Installation:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/installation.html
- **Quick Start Guide:**
    - https://slsdetectorgroup.github.io//{{VERSION}}/quick_start_guide.html
- **Firmware Upgrade:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/firmware.html
- **Detector Server upgrade:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/serverupgrade.html
- **Detector Simulators:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/virtualserver.html
- **Consuming slsDetectorPackage:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/consuming.html
- **Software Architecture:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/softwarearchitecture.html
- **Set up commands in config file:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/configcommands.html
- **Image Size and Output Characteristics:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/dataformat.html
- **API Examples:**
    - https://github.com/slsdetectorgroup/api-examples
- **Command Line Documentation:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/commandline.html
- **C++ API Documentation:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/detector.html
- **C++ API Example:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/examples.html#
- **Python API Documentation:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/pygettingstarted.html
- **Python API Example:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/pyexamples.html
- **Receivers (including custom receiver):**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/receivers.html
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/slsreceiver.html
- **Detector UDP Header:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/udpheader.html
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/udpdetspec.html
- **Output Data:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/dataformat.html
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/fileformat.html
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/slsreceiverheaderformat.html
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/masterfileattributes.html
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/binaryfileformat.html
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/hdf5fileformat.html
- **slsReceiver Zmq Format:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/slsreceiver.html#zmq-json-header-format
- **TroubleShooting:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/troubleshooting.html
    - https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSION}}/troubleshooting.html#receiver-pc-tuning-options
- **Further Documentation:**
    - https://www.psi.ch/en/detectors/documentation
- **Info on Releases:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/index.html

### Support

- dhanya.thattil@psi.ch
- erik.frojdh@psi.ch
- alice.mazzoleni@psi.ch
