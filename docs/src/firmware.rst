Firmware Upgrade
=================



Eiger
-------------
.. note ::
    | Eiger firmware can be upgraded remotely.
    | The programming executable (bcp) and corresponding bit files are provided by the SLS Detector group.


Compatibility 
^^^^^^^^^^^^^

**Release candidate 5.0.0-rc1**
    .. code-block:: bash

        Minimum compatible version  : 27
        Latest compatible version   : 27  
    
`Older versions <https://www.psi.ch/en/detectors/latest-installation>`_


Corrsponding Detector Server 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
| **Location:**
| 5.0.0-rc1: slsDetectorPackage/slsDetectorServer/eigerDetectorServer/bin/eigerDetectorServer_developer

Upgrade
^^^^^^^^
#. Tftp must be already installed on your pc to use the bcp executable.

#. Kill the on-board servers and copy new servers to the board. 

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


    * This is crucial when registers between firmwares change. Failure to do so will result in linux on boards to crash and boards can't be pinged anymore.

#. Bring the board into programmable mode using either of the 2 ways. Both methods result in only the central LED blinking.
    
    * **Manual:**
    
        Do a hard reset for each half module on back panel boards, between the LEDs, closer to each of the 1G ethernet connectors. Push until all LEDs start to blink.
    
    * Software:  
        .. code-block:: bash

            ssh root@bebxxx
            cd executables
            ./boot_recovery

#. Start a terminal for each half module and run the following to see progress.

    .. code-block:: bash
    
    	nc -p 3000 -u bebxxx 3000
        # Press enter twice to see prompt with board name.
        > bebxxx
        # After each bcp command, wait for this terminal to print "Success".


#. In another terminal, run the following to update firmware. Please update bit files with great caution as it could make your board inaccessible, if done incorrectly.

    .. code-block:: bash
    
        #update back end fpga
        bcp download.bit bebxxx:/fw0

        #update front left fpga
        bcp download.bit bebxxx:/febl

        #update front right fpga
        bcp download.bit bebxxx:/febr

        #update kernel (only if required by the SLS Detector Group)
        bcp download.bit bebxxx:/kernel

#. Reboot the detector.

Jungfrau
-------------
.. note ::
    | Jungfrau firmware can be upgraded remotely.
    | The corresponding programming file (pof) is provided by the SLS Detector group.


Compatibility 
^^^^^^^^^^^^^

**Release candidate 5.0.0-rc1**

    .. code-block:: bash

        # PCB v1.0
        Minimum compatible version : 24.07.2020 (v0.8)
        Latest compatible version  : 24.07.2020 (v0.8)
        # PCB v2.0
        Minimum compatible version : 21.07.2020 (v2.1)
        Latest compatible version  : 21.07.2020 (v2.1) 
    
`Older versions <https://www.psi.ch/en/detectors/latest-installation>`_


Corrsponding Detector Server 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
| **Location:**
| 5.0.0-rc1: slsDetectorPackage/slsDetectorServer/jungfrauDetectorServer/bin/jungfrauDetectorServer_developer


Upgrade (from v4.x.x)
^^^^^^^^^^^^^^^^^^^^
#. Tftp must be installed on pc.

#. Update client package to the latest (5.0.0-rc1).

#. Disable server respawning or kill old server
    .. code-block:: bash

        # Option 1: if respawning enabled
        telnet bchipxxx
        # edit /etc/inittab
        # comment out line #ttyS0::respawn:/jungfrauDetectorServervxxx
        reboot
        # ensure servers did not start up after reboot
        telnet bchipxxx
        ps

        #  Option 2: if respawning already disabled
        telnet bchipxxx
        killall jungfrauDetectorServerv*

#. Copy new server and start in update mode
    .. code-block:: bash

        tftp pcxxx -r jungfrauDetectorServervxxx -g
        chmod 777 jungfrauDetectorServervxxx
        ./jungfrauDetectorServervxxx -u

#. Program fpga from the client console
    .. code-block:: bash

        sls_detector_get free
        # Crucial that the next command executes without any errors
        sls_detector_put hostname bchipxxx
        sls_detector_put programfpga xxx.pof

#. After programming, kill 'update server' using Ctrl + C in server console.

#. Enable server respawning if needed
    .. code-block:: bash

        telnet bchipxxx
        # edit /etc/inittab
        # uncomment out line #ttyS0::respawn:/jungfrauDetectorServervxxx
        # ensure the line has the new server name
        reboot
        # ensure both servers are running using ps
        jungfrauDetectorServervxxx
        jungfrauDetectorServervxxx --stop-server 1953


Upgrade (from v5.0.0-rc1)
^^^^^^^^^^^^^^^^^^^^^^^^

#. Program from console
    .. code-block:: bash

        # copies server from tftp folder of pc, programs fpga,
        # removes old server from respawn, sets up new server to respawn
        # and reboots
        sls_detector_put update jungfrauDetectorServervxxx pcxxx xx.pof

        # Or only program firmware
        sls_detector_put programfpga xxx.pof


Gotthard
---------

.. warning ::
    | Gotthard firmware cannot be upgraded remotely and requires the use of USB-Blaster.
    | It is generally updated by the SLS Detector group.


Compatibility 
^^^^^^^^^^^^^

**Release candidate 5.0.0-rc1**

    .. code-block:: bash

        Minimum compatible version  : 11.01.2013
        Latest compatible version   : 08.02.2018 (50um and 25um Master)
                                      09.02.2018 (25 um Slave)  
    
`Older versions <https://www.psi.ch/en/detectors/latest-installation>`_

Corrsponding Detector Server 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
| **Location:**
| 5.0.0-rc1: slsDetectorPackage/slsDetectorServer/gotthardDetectorServer/bin/gotthardDetectorServer_developer

Upgrade
^^^^^^^^

#. Download `Altera Quartus software or Quartus programmer <https://fpgasoftware.intel.com/20.1/?edition=standard&platform=linux&product=qprogrammer#tabs-4>`_.
   

#. Start Quartus programmer, click on Hardware Setup. In the "Currently selected hardware" window, select USB-Blaster.

#. In the Mode combo box, select "Active Serial Programming".

#. Plug the end of your USB-Blaster with the adaptor provided to the connector 'AS config' on the Gotthard board.

#. Click on 'Add file'. Select programming (pof) file provided by the SLS Detector group.

#. Check "Program/Configure" and "Verify". Push the start button. Wait until the programming process is finished.

#. In case of error messages, check the polarity of cable (that pin1 corresponds) and that the correct programming connector is selected.

#. Reboot the detector.


Mythen3
-------
.. note ::
    | Mythen3 firmware can be upgraded remotely.
    | The corresponding programming file (rbf) is provided by the SLS Detector group.


Compatibility 
^^^^^^^^^^^^^

**Release candidate 5.0.0-rc1**

    .. code-block:: bash

        Minimum compatible version : 25.09.2020
        Latest compatible version  : 25.09.2020
    
Corrsponding Detector Server 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
| **Location:**
| 5.0.0-rc1: slsDetectorPackage/slsDetectorServer/mythen3DetectorServer/bin/mythen3DetectorServer_developer

Upgrade (from v5.0.0-rc1)
^^^^^^^^^^^^^^^^^^^^^^^^

#. Program from console
    .. code-block:: bash

        # copies server from tftp folder of pc, programs fpga,
        # and reboots (new server not respawned currently)
        sls_detector_put update mythen3DetectorServervxxx pcxxx xxx.rbf

        # Or only program firmware
        sls_detector_put programfpga xxx.rbf



Gotthard2
----------
.. note ::
    | Gotthard2 firmware can be upgraded remotely.
    | The corresponding programming file (rbf) is provided by the SLS Detector group.


Compatibility 
^^^^^^^^^^^^^

**Release candidate 5.0.0-rc1**

    .. code-block:: bash

        Minimum compatible version : 25.09.2020
        Latest compatible version  : 25.09.2020
    
Corrsponding Detector Server 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
| **Location:**
| 5.0.0-rc1: slsDetectorPackage/slsDetectorServer/gotthard2DetectorServer/bin/gotthard2DetectorServer_developer

Upgrade (from v5.0.0-rc1)
^^^^^^^^^^^^^^^^^^^^^^^^

#. Program from console
    .. code-block:: bash

        # copies server from tftp folder of pc, programs fpga,
        # and reboots (new server not respawned currently)
        sls_detector_put update gotthard2DetectorServervxxx pcxxx xxx.rbf

        # Or only program firmware
        sls_detector_put programfpga xxx.rbf



Moench
------
.. note ::
    | Moench firmware can be upgraded remotely.
    | The corresponding programming file (pof) is provided by the SLS Detector group.


Compatibility 
^^^^^^^^^^^^^

**Release candidate 5.0.0-rc1**

    .. code-block:: bash

        Minimum compatible version : 02.03.2020
        Latest compatible version  : 02.03.2020 
    
Corrsponding Detector Server 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
| **Location:**
| 5.0.0-rc1: slsDetectorPackage/slsDetectorServer/moenchDetectorServer/bin/moenchDetectorServer_developer

Upgrade (from v5.0.0-rc1)
^^^^^^^^^^^^^^^^^^^^^^^^

#. Program from console
    .. code-block:: bash

        # copies server from tftp folder of pc, programs fpga,
        # removes old server from respawn, sets up new server to respawn
        # and reboots
        sls_detector_put update moenchDetectorServervxxx pcxxx xx.pof

        # Or only program firmware
        sls_detector_put programfpga xxx.pof

Ctb
---
.. note ::
    | Ctb firmware can be upgraded remotely.
    | The corresponding programming file (pof) is provided by the SLS Detector group.


Compatibility 
^^^^^^^^^^^^^

**Release candidate 5.0.0-rc1**

    .. code-block:: bash

        Minimum compatible version : 27.11.2019
        Latest compatible version  : 27.11.2019
    
Corrsponding Detector Server 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
| **Location:**
| 5.0.0-rc1: slsDetectorPackage/slsDetectorServer/ctbDetectorServer/bin/ctbDetectorServer_developer

Upgrade (from v5.0.0-rc1)
^^^^^^^^^^^^^^^^^^^^^^^^

#. Program from console
    .. code-block:: bash

        # copies server from tftp folder of pc, programs fpga,
        # removes old server from respawn, sets up new server to respawn
        # and reboots
        sls_detector_put update ctbDetectorServervxxx pcxxx xx.pof

        # Or only program firmware
        sls_detector_put programfpga xxx.pof