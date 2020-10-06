Detector Server Upgrade
=======================



Eiger
-------------


**Location:** slsDetectorPackage/serverBin/ folder for every release.


#. Kill old server and copy new server
    .. code-block:: bash

        # Option 1: from detector console
        # kill old server
        ssh root@bebxxx
        killall eigerDetectorServer

        # copy new server
        cd executables
        scp user@pc:/path/eigerDetectorServerxxx .
        chmod 777 eigerDetectorServerxxx
        ln -sf eigerDetectorServerxxx eigerDetectorServer
        sync

        # Options 2: from client console for multiple modules
        for i in bebxxx bebyyy;
        do ssh root@$i killall eigerDetectorServer;
        scp eigerDetectorServerxxx root@$i:~/executables/eigerDetectorServer;
        ssh root@$i sync; done


#. Reboot the detector.


Jungfrau
-------------

**Location:** slsDetectorPackage/serverBin/ folder for every release.

#. Install tftp and copy detector server binary to tftp folder
#. Program from console (only from 5.0.0-rcx)
    .. code-block:: bash

        # copies new server from pc tftp folder, respawns and reboots
        sls_detector_put copydetectorserver jungfrauDetectorServerxxx pcxxx


Gotthard
---------

**Location:** slsDetectorPackage/serverBin/ folder for every release.

#. Install tftp and copy detector server binary to tftp folder
#. Program from console (only from 5.0.0-rcx)
    .. code-block:: bash

        # copies new server from pc tftp folder, respawns and reboots
        sls_detector_put copydetectorserver gotthardDetectorServerxxx pcxxx



Mythen3
-------

**Location:** slsDetectorPackage/serverBin/ folder for every release.

#. Install tftp and copy detector server binary to tftp folder
#. Program from console (only from 5.0.0-rcx)
    .. code-block:: bash

        # copies new server from pc tftp folder and reboots (does not respawn)
        sls_detector_put copydetectorserver mythen3DetectorServerxxx pcxxx


Gotthard2
----------

**Location:** slsDetectorPackage/serverBin/ folder for every release.

#. Install tftp and copy detector server binary to tftp folder
#. Program from console (only from 5.0.0-rcx)
    .. code-block:: bash

        # copies new server from pc tftp folder and reboots (does not respawn)
        sls_detector_put copydetectorserver gotthard2DetectorServerxxx pcxxx


Moench
------

**Location:** slsDetectorPackage/serverBin/ folder for every release.

#. Install tftp and copy detector server binary to tftp folder
#. Program from console (only from 5.0.0-rcx)
    .. code-block:: bash

        # copies new server from pc tftp folder, respawns and reboots
        sls_detector_put copydetectorserver moenchDetectorServerxxx pcxxx


Ctb
---

**Location:** slsDetectorPackage/serverBin/ folder for every release.

#. Install tftp and copy detector server binary to tftp folder
#. Program from console (only from 5.0.0-rcx)
    .. code-block:: bash

        # copies new server from pc tftp folder, respawns and reboots
        sls_detector_put copydetectorserver ctbDetectorServerxxx pcxxx
