Troubleshooting
=================

If something is missing, don't hesitate to
open an issue at our  `github repo issues
<https://github.com/slsdetectorgroup/slsDetectorPackage/issues>`_. 

Common
------


1. Total Failure of Packet Delivery
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#. Data cable plugged into the wrong interface on board (Jungfrau)
    * Please ensure that the data cable is plugged into the rightmost interface (default for single interface). The inner one is disabled for PCB v1.0 and must be selected via command for PCB v2.0.

#. Link up and speed
    * Check ethtool and find if Link Deteced:Yes and Speed is acceptable (>10k).
    * Check to see if the 10G link is up (blue or red LED on board, close to SFP+). If not:

       * Check transeiver and fibers are compatible (all MMF 850nm or all SMF 1030nm)
       * Check fiber
       * Check fiber polarity (if short range, unplug the link anywhere, and look at the light/dark pattern: dark has to mate with light)
    * For Jungfrau, check if the blue sfp light is blinking rapidly (even when it is not sending data). If so, most likely the link is down and something is wrong with the board. If it connected to a switch, then you do not see it with the ethtool command if link is down. One option is to connect it directly to a pc to see if link is down.
    * With nc, try "nc -u -p 50001 -l" in receiving pc, and from another pc try "echo hallo | nc -u 10.1.2.172 50001" to send something to the recieving pc interface to see if the link is up and see if the other nc console receives the hallo.

#. Detector is not acquiring (Not Eiger)
    * Take an acquisition with many images and using the following steps instead of acquire:

        .. code-block:: bash

            sls_detector_put status start
            # keep executing this command to see if the number of frames left keeps decreasing,
            # which means the detector is acquiring.
            sls_detector_get framesl 

            # If you are using multiple modules, the previous command can return -1 because each module will return different values. Then, check for a single module instead: sls_detector_get 0:framesl


#. Detector is not sending data (Except Eiger)
    * Check the board to see if the green LED close to SFP is blinking (detector is sending data). If not, detector is not operated properly (period too short/long, no trigger in trigger mode) or misconfigured and needs reboot.

#. Power supply
    * Check if power supply has enough current. 
    * For Jungfrau, refer to :ref:`Jungfrau Power Supply Troubleshooting<Jungfrau Troubleshooting Power Supply>`.

#. Ethernet interface not configured for Jumbo frames (10Gb)
    * Ensure that the interfaces (on NIC and the switch) used in receiver pc have MTU 9000 (jumbo frames) enabled.


#. Check if 'rx packets' counter in 'ifconfig' do not increment for interface.
    * If no, check switch configuration if present. Port counters of switch can also help to identify problem.
    * If yes, but receiver software does not see it:

        * Check no firewall (eg. firewalld) is present or add rules

                .. code-block:: bash
                    
                    # Stop and disable firewall
                    service firewalld stop
                    systemctl disable firewalld
                    # Check status
                    service firewalld status

        * Check that selinux is disabled ( or add rules)
        
#. Source UDP IP in config file (Not Eiger)
    * Ensure it is valid and does not end if 0 or 255. Also ensure that the source ip 'udp_srcip' is in the same subnet as destination ip 'udp_dstip' and the masking in the interface configuration ensures this rule.
    * Eg. If interface IP is 102.10.10.110 and mask is 255.255.255.0, udp_srcip has to be 102.10.10.xxx (same subnet)
    * Use ifconfig and route commands to verify etheret interface configuration


#. Netstat and netcat
    * Try with netstat to see if its really listening to the right interface. Or netcat to see if you get packets.

#. Wireshark or Tcpdump
    * Use one of these to confirm that you receive packets (with the right filtering ie. source and destination ports, ip).



2. Partial or Random Packet Loss (Performance)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. note ::
    
    The following suggestions are for convenience. Please do not follow blindly, research each parameter and adapt it to your system.

#. Receiver PC is not tuned for socket buffer size and input packet queue or other parameters.
    * Refer to :ref:`Receiver PC Tuning<Receiver PC Tuning>`

#. Wiring
    * Faulty wiring or connecting cable to incorrect interface.


#. Pinging the subnet (receiving only a few number of packets each time)
    * If a switch is used between a receiver pc and detector instead of plugging the cables directly, one might have to ping any ip in the subnet of the Ethernet interface constantly so that it does not forget the ip during operation.
    * Eg. if rx_udpip is 10.2.3.100, then ping constantly 10.2.3.xxx, where xxx is any ip other than 100.
    * Using slsReceiver, you can use a command that does this for you:
        .. code-block:: bash
        
            # arping the interface in a separate thread every minute
            sls_detector_put rx_arping 1


#. Only the slaves get no data 
    * Check trigger cabling and trigger configuration
    * When you cannot stop Jungfrau slaves in sync mode, refer to :ref:`Cannot stop slaves<Jungfrau Troubleshooting Sync Slaves Cannot Stop>`.

.. _Receiver PC Tuning:

Receiver PC Tuning Options
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. note ::

    | xth1 is example interface name in the following examples. 
    | These settings are lost at pc reboot.

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

            # enable adaptive xoalescence parameters
            ethtool -C xth1 adaptive-rx on

            # set to max value in your pc settings
            ethtool -C xth1 rx-usecs 100 

    * pause parameters
        .. code-block:: bash

            # check 
            ethtool -a xth1

            # set to max value in your pc settings
            ethtool -A xth1 rx on
    
    * generic receiver offload (might not always work)
        .. code-block:: bash

            # check
            ethtool -k xth1

            # enable generic receiver offload
            ethtool -K xth1 gro
        

    * **permanent ethtool settings**

        There are two main methods.

        1. ``systemd service`` (recommended for Fedora and modern RHEL)

            This is the preferred method on systems using systemd and NetworkManager, such as Fedora and RHEL 8+.

            Create a systemd service template:
                .. code-block:: ini

                    # /etc/systemd/system/ethtool@.service

                    [Unit]
                    Description=Set RX buffer size with ethtool
                    After=network-online.target
                    Wants=network-online.target

                    [Service]
                    ExecStart=/usr/sbin/ethtool -G %i rx 8192
                    Type=oneshot

                    [Install]
                    WantedBy=multi-user.target


            Enable the service for a specific interface:
                .. code-block:: bash

                    sudo systemctl enable ethtool@xth1.service

            This ensures the setting is applied at every boot once the network is online.


        2. ``ETHTOOL_OPTS in ifcfg scripts`` (legacy method for RHEL 7 and earlier)

            This method applies only to systems using the legacy network-scripts backend, typically RHEL 7 and earlier. 

            .. code-block:: bash

                # edit the corresponding ``ifcfg-*`` file
                # file: /etc/sysconfig/network-scripts/ifcfg-eth0

                # add or modify the following line:
                ETHTOOL_OPTS="-K  gro on; -G  rx 4096; -L  combined 4; -C  rx-usecs 100; -C  adaptive-rx on"

                # restart the interface
                ifdown eth0 && ifup eth0


#. Disable power saving in CPU frequency scaling and set system to performance 
    * Check current policy (default might be powersave or schedutil)
        .. code-block:: bash
            
            # check current active governor and range of cpu freq policy
            cpupower frequency-info --policy
            # list all available governors for this kernel
            cpupower frequency-info --governors  

    * Temporarily (until shut down)
        .. code-block:: bash
            
            # set to performance
            sudo cpupower frequency-set -g performance

            # or
            cpufreq-info
            for i in ‘seq 0 7‘; do cpufreq-set -c $i -g performance; done
            
    * Permanently
        .. code-block:: bash
            
            # edit /etc/sysconfig/cpupower to preference

            # enable or disable permanently
            sudo systemctl enable cpupower

#. Give user speicific user scheduling privileges.
    .. code-block:: bash

        # edit /etc/security/limits.conf
        # add following line or similar depending on your distribution
        username rtprio 99

    .. note ::

        This is also set if slsReceiver is run as root user.
        
#. Some more advanced options: 
    .. warning ::
        
        Please do not try if you do not understand

    #. reduce the number of queue per NIC to the number of expected streams: ethtool -L xth0 combined 2 
    #. assign each queue to its stream:  ethtool -U xth0 flow-type tcp4 dst-port 50004 action 1
    #. assign to each queue (IRQ) one CPU on the right socket:  echo "3"> /proc/irq/47/smp_affinity_list    #change the numbers looking at /proc/interrupts
    #. disable irqbalance service 
    #. Be sure that the switch knows the receiver mac address. Most switches reset the mac lists every few minutes, and since the receiver only receives, there is not a periodic refresh of the mac list. In this case, one can set a fixed mac list in the switch, or setup some kind of script arping or pinging out from that interface (will be available in 7.0.0). 
    #. assign the receiver numa node (also with -m) to the socket where the NIC is attached. To know it, cat /sys/class/net/ethxxx/device/numa_node
    #. ensure file system performance can handle sustained high data rate:
        
        * One can use dd: 

            .. code-block:: bash
	            
                dd if=/dev/zero of=/testpath/testfile bs=1M count=100000
        * Or better fio (which needs to be installed) 

            .. code-block:: bash
	        
                fio --name=global –directory=/testpath/ --rw=write --ioengine=libaio --direct=0 --size=200G -- 	numjobs=2 --iodepth=1 --bs=1M –name=job

slsReceiver Tuning
^^^^^^^^^^^^^^^^^^

#. Starting receiver as root to have scheduling privileges.

#. For 10g, enable flow control

    .. code-block:: bash

        sls_detector_put flowcontrol10g 1

#. Increase slsReceiver ring buffer depth 
    This can be tuned depending on the number of receivers (modules) and memory available.

    .. code-block:: bash

        # sugggested not to use more than half memory of CPU socket in case of NUMA systems) for this

        sls_detector_get rx_fifodepth
        # sets number of frames in fifo to 1024 ~1GB per receiver. Default is 2500
        sls_detector_put rx_fifodepth 1024

#. Increase number of frames per file 
    This can reduce time taken to open and close files.

    .. code-block:: bash

        sls_detector_get rx_framesperfile
        sls_detector_put rx_framesperfile 20000
        # writes all frames into a single file
        sls_detector_put rx_framesperfile 0

#. Disable file write
    This can ensure it is not the file system performance hampering high date rate.

    .. code-block:: bash

        sls_detector_put fwrite 0


Shared memory error
^^^^^^^^^^^^^^^^^^^
For errors due to access or size, use any of the following suggestions.
    #. Delete shared memory files and try again
    #. Use environment variable to use a different shared memory ending in jfxx
        
        .. code-block:: bash

            # shared memory ending in jfxx
            export SLSDETNAME=jfxx

    #. USe a different multi shared memory ID
        .. code-block:: bash
    
            sls_detector_put 2-config xxxx.config
            # or
            sls_detector_put 2-hostname bchipxxx

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



Mythen3
--------

Detector status is waiting even in auto timing mode
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Check if the control board or the flat band cable is connected properly. If not, connect them properly and try again.


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
    * Jungfrau needs a ~4A per module for a short time at startup. If not, it reboots misconfigured.
    * Comment out this line in the config file: powerchip 1
    * Powering on the chip increases the power consumption by a considerable amount. If commenting out this line aids in getting data (strange data due to powered off chip), then it could be the power supply current limit. Fix it (possibly to 8A current limit) and uncomment the powerchip line back in config file.



.. _Jungfrau Troubleshooting Sync Slaves Cannot Stop:

Cannot stop slaves in sync mode
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#. If cabling is accessible, ensure termination board and flatband cable between the masters and the slaves are connnected properly. Then try to stop.
#. If cabling is inaccessible, unsync first so that the slaves can get the stop directly from the client using the command. Then, don't use sync mode until the cabling is fixed.

    .. code-block:: bash
        
        # unsync, slaves command will fail as it is still in waiting state
        sls_detector_put sync 0

        # stop should now be successful as master does not determine the stop anymore
        sls_detector_put stop
