Firmware Upgrade
=================



Eiger
-------------

Download 
^^^^^^^^^^^^^
- `bcp script <https://github.com/slsdetectorgroup/slsDetectorFirmware/blob/master/binaries/eiger/bcp>`__

- detector server corresponding to package in slsDetectorPackage/serverBin

- `bit files <https://github.com/slsdetectorgroup/slsDetectorFirmware>`__

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

- `pof files <https://github.com/slsdetectorgroup/slsDetectorFirmware>`__


Upgrade (from v4.x.x)
^^^^^^^^^^^^^^^^^^^^^^

Check :ref:`firmware troubleshooting <blackfin firmware troubleshooting>` if you run into issues while programming firmware.

#. Tftp must be installed on pc.

#. Update client package to the latest (5.x.x).

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


Upgrade (from v5.0.0)
^^^^^^^^^^^^^^^^^^^^^^^^^^

Check :ref:`firmware troubleshooting <blackfin firmware troubleshooting>` if you run into issues while programming firmware.


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

- `pof files <https://github.com/slsdetectorgroup/slsDetectorFirmware>`__


.. _firmware upgrade using blaster for blackfin:

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

.. note :: 

  As it is still in developement, the rbf files must be picked up from the SLS Detector Group.

Download 
^^^^^^^^^^^^^

- detector server corresponding to package in slsDetectorPackage/serverBin

- rbf files (in developement)


Upgrade (from v5.0.0)
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

.. note :: 

  As it is still in developement, the rbf files must be picked up from the SLS Detector Group.

Download 
^^^^^^^^^^^^^
- detector server corresponding to package in slsDetectorPackage/serverBin

- rbf files (in development)


Upgrade (from v5.0.0)
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

- `pof files <https://github.com/slsdetectorgroup/slsDetectorFirmware>`__


Upgrade (from v5.0.0)
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Check :ref:`firmware troubleshooting <blackfin firmware troubleshooting>` if you run into issues while programming firmware.

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

- `pof files <https://github.com/slsdetectorgroup/slsDetectorFirmware>`__


Upgrade (from v5.0.0)
^^^^^^^^^^^^^^^^^^^^^^^^^^

Check :ref:`firmware troubleshooting <blackfin firmware troubleshooting>` if you run into issues while programming firmware.

#. Program from console
    .. code-block:: bash

        # copies server from tftp folder of pc, programs fpga,
        # removes old server from respawn, sets up new server to respawn
        # and reboots
        sls_detector_put update ctbDetectorServervxxx pcxxx xx.pof

        # Or only program firmware
        sls_detector_put programfpga xxx.pof


.. _blackfin firmware troubleshooting:

Firmware Troubleshooting with blackfin
----------------------------------------

1. v4.x.x client after programming will most likely reboot the blackfin processor, regardless of error.

2. v5.x.x-rcx client after programming will not reboot the blackfin processor, if error occurred.

3. If a reboot occured with an incomplete firmware in flash, the blackfin will most likely not find the mtd3 drive. To see if this drive exists:

  .. code-block:: bash
    
    # connect to the board
    telnet bchipxxx

    # view of mtd3 existing
    root:/> more /proc/mtd
    dev:    size   erasesize  name
    mtd0: 00040000 00020000 "bootloader(nor)"
    mtd1: 00100000 00020000 "linux kernel(nor)"
    mtd2: 002c0000 00020000 "file system(nor)"
    mtd3: 01000000 00010000 "bitfile(spi)"

4. If one can see the mtd3 drive, one can already try to flash again using the **programfpga** command (without rebooting blackfin or detector). 

5. If one can't list it, read the next section to try to get the blackfin to list it.

How to get back mtd3 drive remotely
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
This might take a few reruns (maybe even 10) until the mtd drive is accessed by the blackfin upon linux startup.

  .. code-block:: bash
    
    # step 1: connect to the board
    telnet bchipxxx

    # step 2: check if mtd3 drive listed
    more /proc/mtd

    # step 3: tell fpga not to touch flash and reboot
    echo 9 > /sys/class/gpio/export; 
    echo out > /sys/class/gpio/gpio9/direction; 
    echo 0 > /sys/class/gpio/gpio9/value;
    reboot

    # step 4: repeat steps 1 - 3 until you see the mtd3 drive


Last Resort using USB Blaster
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If none of these steps work, the last resort might be physically upgrading the firmware using a USB blaster, which also requires opening up the detector. Instructions for all the blackfin detectors are the same as the one for :ref:`gotthard firmware upgrade <firmware upgrade using blaster for blackfin>`.