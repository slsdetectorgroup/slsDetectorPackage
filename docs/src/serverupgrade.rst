.. _Detector Server Upgrade:
Detector Server Upgrade
=======================


**Location:** slsDetectorPackage/serverBin/ folder for every release.


#. Install tftp and copy detector server binary to tftp folder
#. Program from console (only from 5.0.0-rcx)
    .. code-block:: bash

        # copies new server from pc tftp folder, creates a soft link to xxxDetectorServerxxx
        # [Jungfrau][CTB][Moench] also edits initttab to respawn server on reboot
        # Then, the detector controller will reboot (except Eiger)
        sls_detector_put copydetectorserver xxxDetectorServerxxx pcxxx

#. Copy the detector server specific config files or any others required to the detector:

   .. code-block:: bash

        sls_detector_put execcommand "tftp pcxxx -r configxxx -g"


.. note :: 

    For Mythen3, Gotthard2 and Eiger, you need to add scripts to automatically start detector server upon power on. See :ref:`Automatic start<Automatic start servers>` for more details.

 .. note :: 

    Eiger requires a manual reboot. Or killall the servers and restart the new linked one. If you are in the process of updating firmware, then don't reboot yet.


Errors
------

#. tftp write error: There is no space left. Please delete some old binaries and try again.

#. text file busy: You are trying to copy the same server.