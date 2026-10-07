.. _firmware upgrade:

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

#. Copy new servers to the board. See :ref:`how to upgrade detector servers<Detector Server Upgrade>` for more detals. A reboot should have started the new linked servers automatically. For Eiger, do not reboot yet as we need to program the firmware via bit files.

    * This step is crucial when registers between firmwares change. Failure to do so will result in linux on boards to crash and boards can't be pinged anymore.

#. Bring the board into programmable mode using either of the 2 ways. Both methods result in only the central LED blinking.
    
    * **Manual:**
    
        Do a hard reset for each half module on back panel boards, between the LEDs, closer to each of the 1G ethernet connectors. Push until all LEDs start to blink.
    
    * Software:  

        .. code-block:: bash

            # Option 1: if the old server is still running:
            sls_detector_put execcommand "./boot_recovery"

            # Option 2:
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

        #update kernel (only if required by us)
        bcp download.bit bebxxx:/kernel

#. Reboot the detector.

    .. code-block:: bash

        # In the first terminal where we saw "Succeess"
        # reconfig febX is necessary only if you have flashed a new feb firmware
        reconfig febl
        reconfig febr
        # will reboot controller
        reconfig fw0

.. note :: 

    If the detector servers did not start up automatically after reboot, you need to add scripts to do that. See :ref:`Automatic start<Automatic start servers>` for more details.

Jungfrau
-------------

Download 
^^^^^^^^^^^^^
- detector server corresponding to package in slsDetectorPackage/serverBin

- `pof files <https://github.com/slsdetectorgroup/slsDetectorFirmware>`__


Upgrade
^^^^^^^^

.. warning ::

    In case you have had issues in the past with programming via software:

    * 6.1.2 server has a fix for seamless fpga programming

    * We recommend first updating the on-board detector server to 6.1.2 (with client 6.1.x) using command `updatedetectorserver <commandline.html#term-updatedetectorserver-server_name-with-full-path>`_.

    * Then use command 'programfpga' to only update firmware or use command 'update' to update firmware and server to the latest release.



Check :ref:`firmware troubleshooting <blackfin firmware troubleshooting>` if you run into issues while programming firmware.




Program from console
    .. code-block:: bash

        # These instructions are for upgrades from v5.0.0. For earlier versions, please contact us.

        # Always ensure that the client and server software are of the same release.

        # copies server, links new server to jungfrauDetectorServer, 
        # removes old server from respawn, sets up new lnked server to respawn
        # programs fpga, reboots

        #  older versions: v5.0.0 - 6.0.0 using tftp from tftp folder of pc
        sls_detector_put update jungfrauDetectorServervxxx pcxxx xx.pof

        # v6.1.1 - present (copies server from the full path provided)
        sls_detector_put update jungfrauDetectorServervxxx xx.pof

        # Or only program firmware
        sls_detector_put programfpga xxx.pof



Mythen III
-----------

Download 
^^^^^^^^^^^^^

- detector server corresponding to package in slsDetectorPackage/serverBin

- `rbf files <https://github.com/slsdetectorgroup/slsDetectorFirmware>`__


Upgrade
^^^^^^^^

Program from console
    .. code-block:: bash

        # Always ensure that the client and server software are of the same release.

        # copies server, links new server to mythen3DetectorServer, 
        # removes old server from respawn, sets up new lnked server to respawn
        # programs fpga, reboots

        #  older versions: v5.0.0 - 6.0.0 using tftp from tftp folder of pc
        sls_detector_put update mythen3DetectorServervxxx pcxxx xxx.rbf

        # v6.1.1 - present (copies server from the full path provided)
        sls_detector_put update mythen3DetectorServervxxx xxx.rbf

        # Or only program firmware
        sls_detector_put programfpga xxx.rbf

.. note :: 

    If the detector servers did not start up automatically after reboot, you need to add scripts to do that. See :ref:`Automatic start<Automatic start servers>` for more details.

Gotthard II
-------------

Download 
^^^^^^^^^^^^^
- detector server corresponding to package in slsDetectorPackage/serverBin

- `rbf files <https://github.com/slsdetectorgroup/slsDetectorFirmware>`__

Upgrade
^^^^^^^^

Program from console
    .. code-block:: bash

        # Always ensure that the client and server software are of the same release.
        
        # copies server, links new server to gotthard2DetectorServer, 
        # removes old server from respawn, sets up new lnked server to respawn
        # programs fpga, reboots

        #  older versions: v5.0.0 - 6.0.0 using tftp from tftp folder of pc
        sls_detector_put update gotthard2DetectorServervxxx pcxxx xxx.rbf

        # v6.1.1 - present (copies server from the full path provided)
        sls_detector_put update gotthard2DetectorServervxxx xxx.rbf

        # Or only program firmware
        sls_detector_put programfpga xxx.rbf

.. note :: 

    If the detector servers did not start up automatically after reboot, you need to add scripts to do that. See :ref:`Automatic start<Automatic start servers>` for more details.

Moench
-------

Download 
^^^^^^^^^^^^^
- detector server corresponding to package in slsDetectorPackage/serverBin

- `pof files <https://github.com/slsdetectorgroup/slsDetectorFirmware>`__



Upgrade
^^^^^^^^

.. warning ::

    In case you have had issues in the past with programming via software:

    * 6.1.2 server has a fix for seamless fpga programming

    * We recommend first updating the on-board detector server to 6.1.2 (with client 6.1.x) using command `updatedetectorserver <commandline.html#term-updatedetectorserver-server_name-with-full-path>`_.

    * Then use command 'programfpga' to only update firmware or use command 'update' to update firmware and server to the latest release.



Check :ref:`firmware troubleshooting <blackfin firmware troubleshooting>` if you run into issues while programming firmware.


Program from console
    .. code-block:: bash

        # Always ensure that the client and server software are of the same release.

        # copies server, links new server to moenchDetectorServer, 
        # removes old server from respawn, sets up new lnked server to respawn
        # programs fpga, reboots

        #  older versions: v5.0.0 - 6.0.0 using tftp from tftp folder of pc
        sls_detector_put update moenchDetectorServervxxx pcxxx xx.pof

        # v6.1.1 - present (copies server from the full path provided)
        sls_detector_put update moenchDetectorServervxxx xx.pof

        # Or only program firmware
        sls_detector_put programfpga xxx.pof

Ctb
----

Download 
^^^^^^^^^^^^^
- detector server corresponding to package in slsDetectorPackage/serverBin

- `pof files <https://github.com/slsdetectorgroup/slsDetectorFirmware>`__



Upgrade
^^^^^^^^

Check :ref:`firmware troubleshooting <blackfin firmware troubleshooting>` if you run into issues while programming firmware.


Program from console
    .. code-block:: bash

        # Always ensure that the client and server software are of the same release.
        
        # copies server, links new server to ctbDetectorServer, 
        # removes old server from respawn, sets up new lnked server to respawn
        # programs fpga, reboots

        #  older versions: v5.0.0 - 6.0.0 using tftp from tftp folder of pc
        sls_detector_put update ctbDetectorServervxxx pcxxx xx.pof

        # v6.1.1 - present (copies server from the full path provided)
        sls_detector_put update ctbDetectorServervxxx xx.pof

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

How to get back mtd3 drive remotely (udpating kernel)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    You have 2 alternatives to update the kernel.

    1. Commands via software (>= v6.0.0)

        .. code-block:: bash

            sls_detector_put updatekernel /home/...path-to-kernel-image


    2. or command line 
    
        .. code-block:: bash
            
            # step 1: get the kernel image (uImage.lzma) from slsdetectorgroup
            # and copy it to pc's tftp folder

            # step 2: connect to the board
            telnet bchipxxx

            #step 3: go to directory for space
            cd /var/tmp/

            # step 3: copy kernel to board
            tftp pcxxx -r uImage.lzma -g

            # step 4: verify kernel copied properly
            ls -lrt
            
            # step 5: erase flash
            flash_eraseall /dev/mtd1
            
            # step 6: copy new image to kernel drive
            cat uImage.lzma > /dev/mtd1
            
            # step 7:
            sync
            
            # step 8:
            reboot
            
            # step 9: verification
            telnet bchipxxx
            uname -a # verify kernel date
            more /proc/mtd # verify mtd3 is listed
            

