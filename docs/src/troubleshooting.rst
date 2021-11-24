Troubleshooting
=================

If something is missing, don't hesitate to
open an issue at our  `github repo issues
<https://github.com/slsdetectorgroup/slsDetectorPackage/issues>`_. 

Common
------

Missing Packets
^^^^^^^^^^^^^^^
Possible causes could be the following:

#. Receiver PC is not tuned for socket buffer size and input packet queue.
    * Refer to :ref:`Increase rmem_default, rmem_max and max_backlog<Receiver PC Tuning>`

#. Wiring
    * Faulty wiring or connecting cable to incorrect interface.

#. Link up and speed
    * Check to see if there is a blue LED on board to signal that the link is up. Check ethtool and find if Link Deteced:Yes and Speed is acceptable (>10k).

#. Detector is not acquiring (Not Eiger)
    * Take an acquisition with many images and using the following steps instead of acquire:
        .. code-block:: bash

            sls_detector_put status start
            # keep executing this command to see if the number of frames left keeps decreasing,
            # which means the detector is acquiring.
            sls_detector_get framesl 

    .. note ::
    
        If you are using multiple modules, the previous command can return -1 because each module will return different values. Then, check for a single module instead: sls_detector_get 0:framesl

#. Data cable plugged into the wrong interface on board (Jungfrau)
    * Please ensure that the data cable is plugged into the rightmost interface. The middle one is disabled for PCB v1.0 and must be selected via command for PCB v2.0.

#. Detector is not sending data
    * Check the board to see if the green LED is blinking next to the data cable, which means that the detector is sending data.

#. Firewall or security feature
    * A firewall or some security feature could be blocking the reception of data.

#. Ethernet interface not configured properly
    * Ensure that the interfaces used are configured properly with the right mask and ip. Eg. use ifconfig and route commands to verify.

#. Ethernet interface not configured for Jumbo frames (10Gb)
    * Ensure that the interfaces used in receiver pc have MTU 9000 (jumbo frames) enabled.

#. Detector IP (Not Eiger)
    * Ensure it is valid and does not end if 0 or 255. Also ensure that the detector ip is in the same subnet as rx_udpip and the masking in the interface configuration ensures this rule.

#. Tcpdump or wireshark
    * Use one of these to confirm that you receive packets (with the right filtering ie. source and destination ports, ip).

#. Check SFP modules
    * Check if the SFP modules on both sides of the fiber are of same type.

#. Pinging the subnet (receiving only a few number of packets each time)
    * If a switch is used between a receiver pc and detector instead of plugging the cables directly, one might have to ping any ip in the subnet of the Ethernet interface constantly so that it does not forget the ip during operation.
    * Eg. if rx_udpip is 10.2.3.100, then ping constantly 10.2.3.xxx, where xxx is any ip other than 100.



.. _Receiver PC Tuning:

Receiver PC Tuning Options
^^^^^^^^^^^^^^^^^^^^^^^^^^
#. Increase maximum receive socket buffer size and socket input packet queue. 
    * Temporarily (until shut down)
        .. code-block:: bash
            
            # check size
            sysctl -a | grep rmem
            sysctl -a | grep backlog

            # set max and default (use 1Gb for Jungfrau and 100Mb for others)
            sysctl net.core.rmem_max=$((100*1024*1024)) 
            sysctl net.core.rmem_default=$((100*1024*1024))
            sysctl net.core.netdev_max_backlog=250000


    * Permanently
            .. code-block:: bash

                # edit /etc/sysctl.conf file
                # set max and default (use 1Gb for Jungfrau and 100Mb for others)
                net.core.rmem_max = 104857600
                net.core.rmem_default= 104857600
                net.core.netdev_max_backlog = 250000

                # save file and run the following
                sysctl -p

    .. note ::
        This is the most basic setting, which is sometimes more than enough.

#. For 10Gb,
    * MTU must be set up to 9000 for jumbo frames on detector, switch and server NIC
    
    * Set up static MAC address tables with separated VLANs

#. Write to memory if not a large disk and pc not fast enough.
    .. code-block:: bash

        mount -t tmpfs none /ramdisk_folder
        # or
        mount -t tmpfs none /mnt/ramdisk -o size=10G
        # check how many GB memory you can allocate, to avoid swapping otherwise    


#. Modify ethtool settings. 
    * rx ring parameters
        .. code-block:: bash

            # check 
            ethtool -g xth1

            # set to max value in your pc settings
            ethtool -G xth1 rx 4096 

    * coalesce settings (might not always work)
        .. code-block:: bash

            # check 
            ethtool -c xth1

            # set to max value in your pc settings
            ethtool -C xth1 rx-usecs 100 

    * pause parameters
        .. code-block:: bash

            # check 
            ethtool -a xth1

            # set to max value in your pc settings
            ethtool -A xth1 rx on
 
    .. note ::

        | xth1 is example interface name. 
        | These settings are lost at pc reboot.

#. Give user speicific user scheduling privileges.
    .. code-block:: bash

        # edit /etc/security/limits.conf
        # add following line or similar depending on your distribution
        username rtprio 99

    .. note ::

        This is also set if slsReceiver is run as root user.
        
        
#. Disable power saving in CPU frequency 
    .. code-block:: bash

        # or similar command depending on your distribution
        cpupower frequency-info
        cpupower frequency-set -g performance

        # or
        cpufreq-info
        for i in ‘seq 0 7‘; do cpufreq-set -c $i -g performance; done


slsReceiver Tuning
^^^^^^^^^^^^^^^^^^

#. Starting receiver as root to have scheduling privileges.

#. For 10g, enable flow control
    .. code-block:: bash

        sls_detector_put flowcontrol10g 1

#. Increase slsReceiver fifo depth between listening and processing threads.
    .. code-block:: bash

        sls_detector_get rx_fifodepth
        # sets number of frames in fifo to 5000
        sls_detector_put rx_fifodepth 5000

#. Increase number of frames per file to reduce time taken to open and close files.
    .. code-block:: bash

        sls_detector_get rx_framesperfile
        sls_detector_put rx_framesperfile 20000
        # writes all frames into a single file
        sls_detector_put rx_framesperfile 0


Shared memory error
^^^^^^^^^^^^^^^^^^^
| For errors due to access or size, delete shared memory files nd try again.

To list all shared memory files of sls detector package.
    .. code-block:: bash
        
        ll /dev/shm/slsDetectorPackage*
        -rw-------. 1 l_d l_d  136 Oct  1 11:42 /dev/shm/slsDetectorPackage_multi_0
        -rw-------. 1 l_d l_d 3476 Oct  1 11:42 /dev/shm/slsDetectorPackage_multi_0_sls_0
        -rw-------. 1 l_d l_d 3476 Oct  1 11:42 /dev/shm/slsDetectorPackage_multi_0_sls_1

Cannot connect to detector
^^^^^^^^^^^^^^^^^^^^^^^^^^
Ensure both control and stop servers are running on the detector.
    .. code-block:: bash

        ps -ef | grep jungfrauDetectorServer*

Cannot connect to receiver
^^^^^^^^^^^^^^^^^^^^^^^^^^
Start receiver before running a client command that needs to communicate with receiver.

Receiver: cannot bind socket
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#. slsReceiver or slsMultiReceiver is already open somewhere.
    * Kill it and restart it.

#. Tcp port is in use by another application.
    * Start Receiver with a different tcp port and adjust it config file
        .. code-block:: bash

            # restart receiver with different port
            slsReceiver -t1980

            # adjust in config file
            rx_hostname pcxxxx:1980

.. _common troubleshooting multi module data:

Cannot get multi module data
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Possible causes could be the following:

#. Network
    * If you have a direct connection, check to see if the network cables are connected correctly to corresponding interfaces on the PC side. Check also the network configuration and that the detectors and receivers are in the corresponding subnet.

#. Power Supply
    * Check power supply current limit.
    * For Jungfrau, refer to :ref:`Jungfrau Power Supply Troubleshooting<Jungfrau Troubleshooting Power Supply>`.


Cannot ping module (Nios)
^^^^^^^^^^^^^^^^^^^^^^^^^

If you executed "reboot" command on the board, you cannot ping it anymore unless you power cycle. To reboot the controller, please use the software command ("rebootcontroller"), which talks to the microcontroller.

Gotthard2
---------

Cannot get data without a module attached
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You cannot get data without a module attached as a specific pin is floating. Attach module to get data.


Gotthard
----------


Missing first frame or next frame after a delay
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Connect the data link from the Module directly to receiver pc or to a private network.


Jungfrau
---------

Temperature event occured
^^^^^^^^^^^^^^^^^^^^^^^^^
This will occur only if:
* temp_threshold (threshold temperature) has been set to a value
* temp_control (temperature control) set to 1
* and the temperature overshooted the threshold temperature.

**Consequence**
* sls_detector_get temp_event will give 1 # temperature event occured
* the chip will be powered off

**Solution**
* Even after fixing the cooling, any subsequent powerchip command will fail unless the temperature event has been cleared.

* Clear the temperature event
    .. code-block:: bash
        
        # gives the current chip power status (zero currently as chip powered off)
        sls_detector_get powerchip 

        # clear temperature event
        sls_detector_put temp_event 0

        # power on the chip
        sls_detector_put powerchip 1 


.. _Jungfrau Troubleshooting Power Supply:

Cannot get multi module data
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#. Check :ref:`Common Multi Module Troubleshooting<common troubleshooting multi module data>`
#. Power Supply
    * Comment out this line in the config file: powerchip 1
    * Powering on the chip increases the power consumption by a considerable amount. If commenting out this line aids in getting data (strange data due to powered off chip), then it could be the power supply current limit. Fix it (possibly to 8A current limit) and uncomment the powerchip line back in config file.

