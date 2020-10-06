Firmware Upgrade
=================



Eiger
-------------

Download 
^^^^^^^^^^^^^
- `bcp script <https://github.com/slsdetectorgroup/slsDetectorFirmware/blob/master/binaries/eiger/bcp>`__

- detector server corresponding to package in slsDetectorPackage/serverBin

- bit files
    .. list-table:: 
       :widths: 25 10 30 25 10
       :header-rows: 1

       * - Software
         - Hardware
         - Firmware Date
         - Firmware Link
         - Comments
       * - v5.0.0-rcx
         - 
         - 08.09.2020
         - `v27 <https://github.com/slsdetectorgroup/slsDetectorFirmware/blob/master/binaries/eiger/v27/>`__
         - 
       * - v4.0.0 - v4.2.0
         - 
         - 30.07.2019
         - `v24 <https://github.com/slsdetectorgroup/slsDetectorFirmware/blob/master/binaries/eiger/v24/>`__
         - 
       * - v3.1.0 - v3.1.5
         - 
         - 17.08.2017
         - `v20 <https://github.com/slsdetectorgroup/slsDetectorFirmware/blob/master/binaries/eiger/v20/>`__
         -


Upgrade
^^^^^^^^
#. Tftp must be already installed on your pc to use the bcp script.

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

Download 
^^^^^^^^^^^^^
- detector server corresponding to package in slsDetectorPackage/serverBin

- pof files
    .. list-table:: 
       :widths: 25 10 30 25 10
       :header-rows: 1

       * - Software
         - Hardware
         - Firmware Date
         - Firmware Link
         - Comments
       * - v5.0.0-rcx
         - 2.0
         - 21.07.2020
         - `v2.1 <https://github.com/slsdetectorgroup/slsDetectorFirmware/blob/master/binaries/jungfrau/v2_1/jungfrau_v2_1.pof>`__
         - 
       * - v5.0.0-rcx
         - 1.0
         - 24.07.2020
         - `v1.1 <https://github.com/slsdetectorgroup/slsDetectorFirmware/blob/master/binaries/jungfrau/v1_1/jungfrau_v1_1.pof>`__
         - 
       * - v4.0.1 - v4.2.0
         - 1.0
         - 06.12.2018
         - `v0.7 <https://github.com/slsdetectorgroup/slsDetectorFirmware/blob/master/binaries/jungfrau/v0_7/jungfrau_v0_7.pof>`__
         - 
       * - v3.1.0 - v3.1.5
         - 1.0
         - 13.11.2017
         - `v0.6 <https://github.com/slsdetectorgroup/slsDetectorFirmware/blob/master/binaries/jungfrau/v0_6/jungfrau_v0_6.pof>`__
         -



Upgrade (from v4.x.x)
^^^^^^^^^^^^^^^^^^^^^^
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


Upgrade (from v5.0.0-rcx)
^^^^^^^^^^^^^^^^^^^^^^^^^^

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

Download 
^^^^^^^^^^^^^
- detector server corresponding to package in slsDetectorPackage/serverBin

- pof files
    .. list-table:: 
       :widths: 15 15 15 15 5
       :header-rows: 1

       * - Software
         - Hardware
         - Firmware Date
         - Firmware Link
         - Comments
       * - All versions
         - 50um
         - 08.02.2018
         - `50um <https://github.com/slsdetectorgroup/slsDetectorFirmware/blob/master/binaries/gotthard_I/50um/gotthard_I_50um.pof>`__
         - 
       * - All versions
         - 25um (master)
         - 08.02.2018
         - `25um (master) <https://github.com/slsdetectorgroup/slsDetectorFirmware/blob/master/binaries/gotthard_I/25um/master/gotthard_I_25um_master.pof>`__
         - 
       * - All versions
         - 25um (slave)
         - 09.02.2018
         - `25um (slave) <https://github.com/slsdetectorgroup/slsDetectorFirmware/blob/master/binaries/gotthard_I/25um/slave/gotthard_I_25um_slave.pof>`__
         - 



Upgrade
^^^^^^^^
.. warning ::
    | Gotthard firmware cannot be upgraded remotely and requires the use of USB-Blaster.
    | It is generally updated by the SLS Detector group.

#. Download `Altera Quartus software or Quartus programmer <https://fpgasoftware.intel.com/20.1/?edition=standard&platform=linux&product=qprogrammer#tabs-4>`__.
   

#. Start Quartus programmer, click on Hardware Setup. In the "Currently selected hardware" window, select USB-Blaster.

#. In the Mode combo box, select "Active Serial Programming".

#. Plug the end of your USB-Blaster with the adaptor provided to the connector 'AS config' on the Gotthard board.

#. Click on 'Add file'. Select programming (pof) file provided by the SLS Detector group.

#. Check "Program/Configure" and "Verify". Push the start button. Wait until the programming process is finished.

#. In case of error messages, check the polarity of cable (that pin1 corresponds) and that the correct programming connector is selected.

#. Reboot the detector.


Mythen3
-------

Download 
^^^^^^^^^^^^^
- detector server corresponding to package in slsDetectorPackage/serverBin

- rbf files
    .. list-table:: 
       :widths: 25 10 30 25 10
       :header-rows: 1

       * - Software
         - Hardware
         - Firmware Date
         - Firmware Link
         - Comments
       * - v5.0.0-rcx
         - 
         - 25.09.2020
         - 
         - 


Upgrade (from v5.0.0-rcx)
^^^^^^^^^^^^^^^^^^^^^^^^^^^

#. Program from console
    .. code-block:: bash

        # copies server from tftp folder of pc, programs fpga,
        # and reboots (new server not respawned currently)
        sls_detector_put update mythen3DetectorServervxxx pcxxx xxx.rbf

        # Or only program firmware
        sls_detector_put programfpga xxx.rbf



Gotthard2
-------------

Download 
^^^^^^^^^^^^^
- detector server corresponding to package in slsDetectorPackage/serverBin

- rbf files
    .. list-table:: 
       :widths: 25 10 30 25 10
       :header-rows: 1

       * - Software
         - Hardware
         - Firmware Date
         - Firmware Link
         - Comments
       * - v5.0.0-rcx
         - 
         - 25.09.2020
         - 
         - 

Upgrade (from v5.0.0-rcx)
^^^^^^^^^^^^^^^^^^^^^^^^^^

#. Program from console
    .. code-block:: bash

        # copies server from tftp folder of pc, programs fpga,
        # and reboots (new server not respawned currently)
        sls_detector_put update gotthard2DetectorServervxxx pcxxx xxx.rbf

        # Or only program firmware
        sls_detector_put programfpga xxx.rbf



Moench
-------

Download 
^^^^^^^^^^^^^
- detector server corresponding to package in slsDetectorPackage/serverBin

- pof files
    .. list-table:: 
       :widths: 25 10 30 25 10
       :header-rows: 1

       * - Software
         - Hardware
         - Firmware Date
         - Firmware Link
         - Comments
       * - v5.0.0-rcx
         - EPCQ128
         - 05.10.2020
         - `v1.0 <https://github.com/slsdetectorgroup/slsDetectorFirmware/blob/master/binaries/moench/EPCQ128/v1_0/moench_v1_0_201005.pof>`__
         - 
       * - v5.0.0-rcx
         - EPCS128
         - 05.10.2020
         - `v1.0 <https://github.com/slsdetectorgroup/slsDetectorFirmware/blob/master/binaries/moench/EPCS128/v1_0/moench_v1_0_201005.pof>`__
         - 

Upgrade (from v5.0.0-rcx)
^^^^^^^^^^^^^^^^^^^^^^^^^^^

#. Program from console
    .. code-block:: bash

        # copies server from tftp folder of pc, programs fpga,
        # removes old server from respawn, sets up new server to respawn
        # and reboots
        sls_detector_put update moenchDetectorServervxxx pcxxx xx.pof

        # Or only program firmware
        sls_detector_put programfpga xxx.pof

Ctb
----

Download 
^^^^^^^^^^^^^
- detector server corresponding to package in slsDetectorPackage/serverBin

- pof files
    .. list-table:: 
       :widths: 25 10 30 25 10
       :header-rows: 1

       * - Software
         - Hardware
         - Firmware Date
         - Firmware Link
         - Comments
       * - v5.0.0-rcx
         - EPCQ128
         - 05.10.2020
         - `v1.0 <https://github.com/slsdetectorgroup/slsDetectorFirmware/blob/master/binaries/ctb/EPCQ128/v1_0/ctb_v1_0_201005.pof>`__
         - 
       * - v5.0.0-rcx
         - EPCS128
         - 05.10.2020
         - `v1.0 <https://github.com/slsdetectorgroup/slsDetectorFirmware/blob/master/binaries/ctb/EPCS128/v1_0/ctb_v1_0_201005.pof>`__
         - 

Upgrade (from v5.0.0-rcx)
^^^^^^^^^^^^^^^^^^^^^^^^^^

#. Program from console
    .. code-block:: bash

        # copies server from tftp folder of pc, programs fpga,
        # removes old server from respawn, sets up new server to respawn
        # and reboots
        sls_detector_put update ctbDetectorServervxxx pcxxx xx.pof

        # Or only program firmware
        sls_detector_put programfpga xxx.pof