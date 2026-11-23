SLS Detector Package Minor Release 10.1.0 released on 2026-11-30
================================================================

This document describes the differences between v10.1.0 and v10.0.1

**Table of Contents**

- [1 New, Changed or Resolved Features](#1-new-changed-or-resolved-features)
  - [1.1 Compilation](#11-compilation)
  - [1.2 Python](#12-python)
  - [1.3 Client](#13-client)
  - [1.4 Detector Server](#14-detector-server)
    - [1.4.1 Jungfrau](#141-jungfrau)
    - [1.4.2 Mythen3](#142-mythen3)
    - [1.4.3 Eiger](#143-eiger)
  - [1.5 Receiver](#15-receiver)
- [2 BugFixes](#2-bugfixes)
  - [2.1 Python](#21-python)
  - [2.2 Client](#22-client)
  - [2.3 Detector Server](#23-detector-server)
    - [2.3.1 Mythen3](#231-mythen3)
  - [2.4 Receiver](#24-receiver)
  - [2.5 Gui](#25-gui)
- [3 On-board Detector Server](#3-on-board-detector-server)
  - [3.1 Version Compatibility](#31-version-compatibility)
  - [3.2 Upgrade](#32-upgrade)
- [4 Firmware](#4-firmware)
  - [4.1 Version Compatibility](#41-version-compatibility)
  - [4.2 Upgrade](#42-upgrade)
- [5 Kernel](#5-kernel)
  - [5.1 Build Compatibility](#51-build-compatibility)
  - [5.2 Upgrade](#52-upgrade)
- [6 Help](#6-help)
  - [6.1 Download](#61-download)
  - [6.2 Documentation](#62-documentation)
  - [6.3 Support](#63-support)


## 1 New, Changed or Resolved Features

### 1.1 Compilation

- Unpacking `qwt` using cmake bundled tools removing the dependency on `bzip2`

- `libexpat` is no longer a dependency of slsDetectorPackage

- All servers are compiled with `-02` optimization level

- Fixed path to updateAPIVersion.py in server MakeFiles (only relevant if manually compiling servers)

- Building shared libraries is disabled by default. If you need to link against any of the `libSls*.so` libraries, you can enable this by passing `-DSLS_BUILD_SHARED_LIBRARIES=ON` to CMake.

- Added `SLS_USE_SYSTEM_ZMQ` option (default `OFF`) to use the libzmq of the host instead of the one included in the repo.

- Using `find_package(Threads)`instead of directly linking to `pthread` to be more portable.
(Still check that the found library is pthread compatible since we use direct calls to pthread internally.)

- Added `Threads::Threads` as a public dependency on `slsDetectorSupport` which means it will propagate to the other libraries and executables

- Removed redundant linking to Posix `librt` library

- Upgraded the bundled libzmq from version 4.3.4 to 4.3.5 (Version 4.3.4 has a bug that cause segfaults on platforms that use x86-64-v3)

### 1.2 Python

- `Detector().pattern` (python) accepts also a `Pattern` object, not only a pattern file

- Added member function `patternstart` to python, only the detector class api was exposed (`startPattern()`)

### 1.3 Client

- removed unused function `readDataFile`/`writeDataFile` from `file_utils.h`

- Added command ``rx_streamdummyheader`` to send the zmq dummy header any time. Allows pre-configuring zmq processing before acq begins.

- Error messages on TCP sockets explicitly state that its a socket errno, returns the error number and duration of how long the faulty call took.


### 1.4 Detector Server

**Detector specific updates:**

#### 1.4.1 Jungfrau

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


#### 1.4.2 Mythen3

- updating threshold energy can be done during an acquisition (only affects slsReceiver)

- updating threshold energy `ALL` can be done during an acquisition (only affects slsReceiver)

#### 1.4.3 Eiger

- updating threshold energy can be done during an acquisition (only affects slsReceiver)

- Setting ``vthreshold`` dac directly sets settings to undefind

### 1.5 Receiver

- added `rx_streamdummyheader` to send the zmq dummy header any time. Allows pre-configuring zmq processing before acq begins.
- allow disabling one UDP interface in the receiver.
- zmqsocket - allowing hwm to be set already at the constructor level. As before, can still be set after construction with socket reconnection. 

- Automatically resolve trailing ``/`` in user given filepaths 

## 2 BugFixes

### 2.1 Python

- Receiver ROI can be set from Python using command ``Detector().rx_roi``(it supports any sequence of four or two (for mythen3 and gotthard) ints e.g. a tuple (xmin, xmax, ymin, ymax) or a sequence of such for multiple ROIS)
- one can clear all ROI's from Python using command ``Detector().rx_clearroi()``

### 2.2 Client

- Fixed version comparison - previously lexicographic comparison on semantic version string was used (e.g. 9.0.0 > 10.0.0)

- Throws if user sets an roi to a disabled port or disable a port that has a restrictive roi (not complete detector).

- Setting `zmqhwm` while client streaming is enabled now recreates/rebinds the zmq sockets so that the new high water mark takes effect.

- m3: `getPatternFileName` typo fixed

### 2.3 Detector Server

#### 2.3.1 Mythen3

- Downgrading required minimum kernel version for Nios to (Mon Jan 18 14:24:30 CET 2021) - as latest Nios kernel not required

- Remove excessive debug printing (only log in debug mode) speeding up setting trimval

- Fixed log info about bad channel when setting trimbits

- Fixed `patwaittime` in intervals, which is hardly used. `patwaittime` in clocks stays the same and working.

### 2.4 Receiver

- Fixed potential deadlock if HDF5 creation throws 

- Partial Readout Mode (`readnrows`):
  - Fixed `rx_framescaught`, which was reporting 0.
  - Fixed frame capturing with `rx_discardpolicy` set to discard partial frames, which previously caused all frames to be discarded.

- Fixed proper enabling of filewrite after changing number of Interfaces (previously no proper file writing setup after changing number of Interfaces)

- Setting ``rx_roi`` only writes/streams data to file/over ``zmq`` within the ROI (previously Ports or modules not overlapping with a user defined ROI was also written/streamed)

- Field "Threshold Energies" in master file always updated to ``[-1, -1, -1]`` whenever it is set in the detector server (setting dacs directly, custom trimbit file, set all trimbits etc.)

- The high water mark `rx_zmqhwm` is now set when the zmq socket is created (before bind) instead of setting it afterwards and rebinding, which was unreliable. If streaming is enabled, setting `rx_zmqhwm` recreates the streaming sockets, so connected zmq clients are briefly disconnected and have to reconnect. Getting `rx_zmqhwm` returns the value actually used by the socket.

### 2.5 Gui

- mouse zooms are not reset at start of acquisition

## 3 On-board Detector Server

### 3.1 Version Compatibility

| Detector  | Version |
|-----------|---------|
| Eiger     | 10.0.0  |
| Jungfrau  | 10.1.0  |
| Mythen3   | 10.0.0  |
| Gotthard2 | 10.0.0  |
| Moench    | 10.0.0  |

### 3.2 Upgrade

From v6.1.0 (without tftp):

- update only on-board detector server
    - Using command `updatedetectorserver`
- udpate both on-board detector server and firmware simultaneously
    - Using command `update`

Instructions available at
- https://slsdetectorgroup.github.io/slsDetectorPackage/{{VERSIO}}/serverupgrade.html

## 4 Firmware 

### 4.1 Version Compatibility

| Detector  | Firmware Date | Version          | Updated in |
|-----------|---------------|------------------|------------|
| Eiger     | 02.10.2023    | v32              | 7.0.3      |
| Jungfrau  | 09.02.2025    | v1.6, HW v1.0    | 9.1.0      |
| Jungfrau  | 08.02.2025    | v2.6, HW v2.0    | 9.1.0      |
| Mythen3   | 13.11.2024    | v2.0             | 9.0.0      |
| Gotthard2 | 03.10.2024    | v1.0             | 9.0.0      |
| Moench    | 26.10.2023    | v2.0             | 8.0.2      |

### 4.2 Upgrade

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
- https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/firmware.html

## 5 Kernel

### 5.1 Build Compatibility

| Detector  | Kernel                        |
| --------- | ----------------------------- |
| Jungfrau  | Fri Oct 29 00:00:00 2021 *    |
| Moench    | Fri Oct 29 00:00:00 2021 *    |
| Mythen3   | Mon Jan 18 14:24:30 CET 2021  |
| Gotthard2 | Mon May 10 18:00:21 CEST 2021 |
| Eiger     | Thu Oct 13 14:51:04 CEST 2016 |

\* Older ones will work, but might have issues with programming firmware via the package.

### 5.2 Upgrade

| Detector | Upgrade via |
|----------|-------------|
| Eiger    | bit files   |
| Others   | command     |

Commands: `udpatekernel`, `kernelversion`

Instructions available at
- https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/commandline.html
- https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/detector.html
- https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/pydetector.html

## 6 Help

### 6.1 Download

- **The Source Code:**
    - https://github.com/slsdetectorgroup/slsDetectorPackage
- **RPM Packages:**
    - https://gitea.psi.ch/detectors/-/packages

### 6.2 Documentation

- **Installation:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/installation.html
- **Quick Start Guide:**
    - https://slsdetectorgroup.github.io//10.1.0/quick_start_guide.html
- **Firmware Upgrade:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/firmware.html
- **Detector Server upgrade:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/serverupgrade.html
- **Detector Simulators:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/virtualserver.html
- **Consuming slsDetectorPackage:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/consuming.html
- **Software Architecture:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/softwarearchitecture.html
- **Set up commands in config file:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/configcommands.html
- **Image Size and Output Characteristics:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/dataformat.html
- **API Examples:**
    - https://github.com/slsdetectorgroup/api-examples
- **Command Line Documentation:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/commandline.html
- **C++ API Documentation:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/detector.html
- **C++ API Example:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/examples.html#
- **Python API Documentation:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/pygettingstarted.html
- **Python API Example:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/pyexamples.html
- **Receivers (including custom receiver):**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/receivers.html
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/slsreceiver.html
- **Detector UDP Header:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/udpheader.html
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/udpdetspec.html
- **Output Data:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/dataformat.html
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/fileformat.html
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/slsreceiverheaderformat.html
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/masterfileattributes.html
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/binaryfileformat.html
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/hdf5fileformat.html
- **slsReceiver Zmq Format:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/slsreceiver.html#zmq-json-header-format
- **TroubleShooting:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/troubleshooting.html
    - https://slsdetectorgroup.github.io/slsDetectorPackage/10.1.0/troubleshooting.html#receiver-pc-tuning-options
- **Further Documentation:**
    - https://www.psi.ch/en/detectors/documentation
- **Info on Releases:**
    - https://slsdetectorgroup.github.io/slsDetectorPackage/index.html

### 6.3 Support

- erik.frojdh@psi.ch
- dhanya.thattil@psi.ch
- alice.mazzoleni@psi.ch
