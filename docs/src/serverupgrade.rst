.. _Detector Server Upgrade:
Detector Server Upgrade
=======================


**Location:** slsDetectorPackage/serverBin/ folder for every release.

.. note :: 

    For Mythen3, Gotthard2 and Eiger, you need to add scripts to automatically start detector server upon power on. See :ref:`Automatic start<Automatic start servers>` for more details.

 .. note :: 

    Eiger requires a manual reboot. Or killall the servers and restart the new linked one. If you are in the process of updating firmware, then don't reboot yet.


From 6.1.1 and above (no tftp required)
---------------------------------------

#. Program from console

    .. code-block:: bash

        # the following command copies new server, creates a soft link to xxxDetectorServerxxx
        # [Jungfrau][CTB][Moench] also deletes the old server binary and edits initttab to respawn server on reboot
        # Then, the detector controller will reboot (except Eiger)
        sls_detector_put updatedetectorserver /complete-path-to-binary/xxxDetectorServerxxx

#. Copy the detector server specific config files or any others required to the detector:

   .. code-block:: bash

        sls_detector_put execcommand "tftp pcxxx -r configxxx -g"

5.0.0 - 6.1.1
--------------

#. Install tftp and copy detector server binary to tftp folder
#. Program from console

    .. code-block:: bash

        # the following command copies new server from pc tftp folder, creates a soft link to xxxDetectorServerxxx
        # [Jungfrau][CTB][Moench] also edits initttab to respawn server on reboot
        # Then, the detector controller will reboot (except Eiger)
        sls_detector_put copydetectorserver xxxDetectorServerxxx pcxxx

#. Copy the detector server specific config files or any others required to the detector:

   .. code-block:: bash

        sls_detector_put execcommand "tftp pcxxx -r configxxx -g"


Troubleshooting with tftp
^^^^^^^^^^^^^^^^^^^^^^^^^

#. tftp write error: There is no space left. Please delete some old binaries and try again.

#. text file busy: You are trying to copy the same server.


Older than 5.0.0
-----------------

Please contact us.