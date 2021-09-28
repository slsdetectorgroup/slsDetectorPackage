Detector Server in the Background
=================================



Eiger
-------------


.. code-block:: bash

    # step 1: connect to board
    ssh root@bebxxx

    # step 2: create file
    vi /etc/rc5.d/S50board_com.ssh

    # step 3: edit contents to start up server (and 10g_led script)
    #! /bin/sh
    ### BEGIN INIT INFO
    # Provides:          feb_com
    # Required-Start:    $remote_fs $all
    # Required-Stop: 
    # Default-Start:     2 3 4 5
    # Default-Stop:
    # Short-Description: Starts the beb_com and feb_com
    # Description:       Starts feb communication ...
    #                    
    ### END INIT INFO

    /home/root/executables/eigerDetectorServer &> /dev/null &
    /home/root/executables/10g_led &

    exit 0

    # step 4: save file and reboot
    reboot

    #step 5: verify server is running in background
    ps -ef | grep eigerDetectorServer


Jungfrau
-------------

Check :ref:`Blackfin detector servers in background<blackfin detector servers in background>`.


Gotthard
---------

Check :ref:`Blackfin detector servers in background<blackfin detector servers in background>`.


Mythen3
-------

Check :ref:`Nios detector servers in background<nios detector servers in background>`.


Gotthard2
----------

Check :ref:`Nios detector servers in background<nios detector servers in background>`.


Moench
------

Check :ref:`Blackfin detector servers in background<blackfin detector servers in background>`.

Ctb
---

Check :ref:`Blackfin detector servers in background<blackfin detector servers in background>`.

.. _blackfin detector servers in background:

Blackfin Detector Servers in Background (Respawning)
----------------------------------------------------

.. code-block:: bash

    # step 1: connect to board
    telnet bchipxxx

    # step 2: create a soft link connecting the right server
    ln -sf xxxDetectorServerxxxxx xxxDetectorServer

    # step 3: edit file to respawn server
    vi /etc/inittab

    # step 4: add to the end of file 
    ttyS0::respawn:/./xxxDetectorServer

    # step 5: save file and reboot
    reboot

    # step 6: verify server is running in background
    ps -ef | grep xxxDetectorServer


.. _nios detector servers in background:

Nios Detector Servers in Background
----------------------------------------

.. code-block:: bash

    # step 1: connect to board
    ssh root@bebxxx

    # step 2: create a soft link connecting the right server
    ln -sf xxxDetectorServerxxx xxxDetectorServer

    # step 3: create file
    vi /etc/init.d/S99DetServer.ssh

    # step 4: add contents 
    #! /bin/sh
    cd /root >> /dev/null
    /root/xxxDetectorServer >> /dev/null &

    # step 5: save file and reboot
    reboot

    # step 6: verify server is running in background
    ps -ef | grep xxxDetectorServer
