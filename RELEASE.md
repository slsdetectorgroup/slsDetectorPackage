SLS Detector Package Minor Release 10.1.0 released on 2026-11-30
================================================================

This document describes the differences between v10.1.0 and v10.0.1

**Table of Contents**

- [1 Build and Dependencies](#1-build-and-dependencies)
  - [1.1 General](#11-general)
  - [1.2 Detector Server](#12-detector-server)
  - [1.3 Gui](#13-gui)
- [2 Bug Fixes](#2-bug-fixes)
  - [2.1 Detector Server](#21-detector-server)
  - [2.2 Client](#22-client)
  - [2.3 Receiver](#23-receiver)
  - [2.4 Gui](#24-gui)
  - [2.5 Documentaion](#25-documentaion)
- [3 Changed Features](#3-changed-features)
    - [3.1 Detector Server](#31-detector-server)
    - [3.2 Client](#32-client)
    - [3.3 Receiver](#33-receiver)
- [4 New Features](#4-new-features)
    - [4.1 Detector Server](#41-detector-server)
    - [4.2 Client](#42-client)
    - [4.3 Receiver](#43-receiver)
    - [4.4 Calibration](#44-calibration)
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



## 1 Build and Dependencies

### 1.1 General

- Removed dependency on `libexpat` and redundant linking to Posix `librt` library
  
- Added `Threads::Threads` as a public dependency on `slsDetectorSupport` which means it will propagate to the other libraries and executables
  
- Shared Lib:<br>
  Building shared libraries is disabled by default and libraries are **compiled statically**. To build the `libSls*.so` libraries for dynamic linking, enable them by passing `-DSLS_BUILD_SHARED_LIBRARIES=ON` to CMake.



- Using `find_package(Threads)`instead of directly linking to `pthread` to be more portable.
(Still check that the found library is pthread compatible since we use direct calls to pthread internally.)

- Upgraded the bundled libzmq from version 4.3.4 to 4.3.5 (Version 4.3.4 has a bug that cause segfaults on platforms that use x86-64-v3)
  
- Option to use sytem libzmq:<br>
  Added `SLS_USE_SYSTEM_ZMQ` option (default `OFF`) to use the libzmq of the host instead of the one included in the repo.


### 1.2 Detector Server

- All servers are compiled with `-02` optimization level


### 1.3 Gui

- Removed dependency on `bzip2` by unpacking `qwt` using cmake bundled tools








****
## 2 Bug Fixes

### 2.1 Detector Server

- Mythen3
  - Kernel Version<br> 
    Downgrading required minimum kernel version for Nios to (Mon Jan 18 14:24:30 CET 2021) - as latest Nios kernel not required

  - Remove excessive debug printing (only log in debug mode) speeding up setting trimval

  - Fixed log info about bad channel when setting trimbits

  - Fixed `patwaittime` in intervals, which is hardly used. `patwaittime` in clocks stays the same and working.

- [Jungfrau] Maximum loops in `pedestalmode`:<br>
  Fixed a software limitation that restricted the maximum number of loops in pedestal mode to 8 bits, despite firmware v9.1.0 supporting 16-bit values.


### 2.2 Client

- Version comparison from v10.0.0<br>
  Fixed. Previously lexicographic comparison on semantic version string was used (e.g. 9.0.0 > 10.0.0)

- ZMQ<br>
  Setting `zmqhwm` while client streaming is enabled now recreates/rebinds the zmq sockets so that the new high water mark takes effect.

- `dacs` in `python`:<br>
  <mark>TODO: issue #1441 and PR #1446 into 10.1.0.rc!!!!!!!!!!!</mark><br>
  Fixed python error in setting dacs due to deprecated np.int.<br>
  dacs now return an int numpy array instead of a float array.

- m3: `getPatternFileName` typo fixed

- [Eiger] ROI and Disabled Port<br>
  Throws if user sets an ROI to a disabled port or disable a port that has a restrictive ROI (not complete detector).

- [Mythen3] Python `pattern` command:<br>
  Now accepts `Pattern` objects directly, rather than being restricted to pattern files.



### 2.3 Receiver


- Partial Readout Mode (`readnrows`):
  - Fixed `rx_framescaught`, which was reporting 0.
  - Fixed frame capturing with `rx_discardpolicy` set to discard partial frames, which previously caused all frames to be discarded.

- `fwrite` and `numinterfaces`<br>
  Fixed an issue where changing the number of UDP interfaces caused the file write setting to be lost.

- `rx_roi`: Fixed an issue where entire ports outside the ROI were also being written.

- Automatically resolve trailing ``/`` in user given filepaths `fpath`.

- [Mythen3] [Eiger] `threshold` metadata:<br>
  Field "Threshold Energies" in master file is also now updated to unknown (`-1`) when settings dacs directly including `vthreshold`, load custom trimbits or set trimbits etc. 

- The high water mark `rx_zmqhwm` is now set when the zmq socket is created (before bind) instead of setting it afterwards and rebinding, which was unreliable. If streaming is enabled, setting `rx_zmqhwm` recreates the streaming sockets, so connected zmq clients are briefly disconnected and have to reconnect. Getting `rx_zmqhwm` returns the value actually used by the socket.

- HDF5<br>
  Fixed potential deadlock if HDF5 creation throws 


### 2.4 Gui

- mouse zooms are not reset at start of acquisition

### 2.5 Documentaion

- Fixed docs. 
  - Fixed typos, mistakes in figures. Thanks to @lrlunin!
  - using svg instead of png images. Thanks to @lrlunin!
  - updated installation docs to be more readable. Suggestions are welcome.


****
## 3 Changed Features

#### 3.1 Detector Server
  
- [Mythen3] [Eiger] updating threshold energy can be done during an acquisition (only affects slsReceiver)

- [Mythen3] updating threshold energy `ALL` can be done during an acquisition (only affects slsReceiver)

- [Eiger] Setting ``vthreshold`` dac directly sets settings to undefind

- [Jungfrau] Chip v1.0 and v1.1:<br>
  - `collectionmode`: Electron collection mode also resets and bypass cds.
  - `filterresistor`: Default changed to the lower one.
  - `autocompdisable`: Previously, disabled upon detector power-on. Now, disabled already at every detector server start-up.
  - `compdisabletime`: Default changed [Chip v1.0: 5us, Chip v1.1: 2 us].
  - ASIC DS timer (Chip v1.1):<br>
    Previously, this was fixed to 0x7f. Changing filter resistor now updates the ASIC DS timer [low res: 0x1f, high res:0x5f].

#### 3.2 Client

- removed unused function `readDataFile`/`writeDataFile` from `file_utils.h`
- Error messages on TCP sockets explicitly state that its a socket errno, returns the error number and duration of how long the faulty call took.


#### 3.3 Receiver

- zmqsocket - allowing hwm to be set already at the constructor level. As before, can still be set after construction with socket reconnection. 









****
## 4 New Features

#### 4.1 Detector Server

- [Jungfrau] Added Chip v1.2 Features:
  - Enable reset step reduction
  - Same features as Chip v1.1, but does not have `filtercells`.
  - Cannot set extra storage cells at the moment.
  - Different default dac values for G0 and G1.
  - `extrastoragecells` disabled temporarily.




  - support for enabling High Dynamic Range (HDR) for Synchotron or FEL









 
#### 4.2 Client

- Python: 
- Added member function `patternstart` to python, only the detector class api was exposed (`startPattern()`)


- `python`: `rx_roi` and `rx_clearroi` commands added to set and clear ROI in receiver.

#### 4.3 Receiver

- Stream Dummy Header:<br>
  Thanks to @lrlunin!<br>
  One can stream out the dummy header from the receiver at any time. This allows pre-configuring zmq processing before acquisition begins.The dummy header does not include the detector header at this point.  

  - Command: `rx_streamdummyheader`
  - C++/python API: `streamRxDummyHeader`  

- allow disabling one UDP interface in the r[Mythen3] eceiver.??? In v11.0.0 i believe


#### 4.4 Calibration

-<mark> #1363 Added support for Jungfrau 2 modules in 1M configuration  for root display=> Needed??</mark>




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
