Detector Server Upgrade
=======================



Eiger
-------------


| **Location:**
| 5.0.0-rc1: slsDetectorPackage/slsDetectorServer/eigerDetectorServer/bin/eigerDetectorServer_developer


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

| **Location:**
| 5.0.0-rc1: slsDetectorPackage/slsDetectorServer/jungfrauDetectorServer/bin/jungfrauDetectorServer_developer

#. Install tftp and copy detector server binary to tftp folder
#. Program from console (only from 5.0.0-rc1)
    .. code-block:: bash

        # copies new server from pc tftp folder, respawns and reboots
        sls_detector_put copydetectorserver jungfrauDetectorServerxxx pcxxx


Gotthard
---------

| **Location:**
| 5.0.0-rc1: slsDetectorPackage/slsDetectorServer/gotthardDetectorServer/bin/gotthardDetectorServer_developer

#. Install tftp and copy detector server binary to tftp folder
#. Program from console (only from 5.0.0-rc1)
    .. code-block:: bash

        # copies new server from pc tftp folder, respawns and reboots
        sls_detector_put copydetectorserver gotthardDetectorServerxxx pcxxx



Mythen3
-------

| **Location:**
| 5.0.0-rc1: slsDetectorPackage/slsDetectorServer/mythen3DetectorServer/bin/mythen3DetectorServer_developer

#. Install tftp and copy detector server binary to tftp folder
#. Program from console (only from 5.0.0-rc1)
    .. code-block:: bash

        # copies new server from pc tftp folder and reboots (does not respawn)
        sls_detector_put copydetectorserver mythen3DetectorServerxxx pcxxx


Gotthard2
----------

| **Location:**
| 5.0.0-rc1: slsDetectorPackage/slsDetectorServer/gotthard2DetectorServer/bin/gotthard2DetectorServer_developer

#. Install tftp and copy detector server binary to tftp folder
#. Program from console (only from 5.0.0-rc1)
    .. code-block:: bash

        # copies new server from pc tftp folder and reboots (does not respawn)
        sls_detector_put copydetectorserver gotthard2DetectorServerxxx pcxxx


Moench
------

| **Location:**
| 5.0.0-rc1: slsDetectorPackage/slsDetectorServer/moenchDetectorServer/bin/moenchDetectorServer_developer

#. Install tftp and copy detector server binary to tftp folder
#. Program from console (only from 5.0.0-rc1)
    .. code-block:: bash

        # copies new server from pc tftp folder, respawns and reboots
        sls_detector_put copydetectorserver moenchDetectorServerxxx pcxxx


Ctb
---

| **Location:**
| 5.0.0-rc1: slsDetectorPackage/slsDetectorServer/ctbDetectorServer/bin/ctbDetectorServer_developer

#. Install tftp and copy detector server binary to tftp folder
#. Program from console (only from 5.0.0-rc1)
    .. code-block:: bash

        # copies new server from pc tftp folder, respawns and reboots
        sls_detector_put copydetectorserver ctbDetectorServerxxx pcxxx
