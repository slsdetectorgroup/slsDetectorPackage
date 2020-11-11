Command line interface
==============================================

Usage
-------------

Commands can be used either with sls_detector_get or sls_detector_put

.. code-block::

    sls_detector_get vrf

Help
--------

.. code-block::

    # get list of commands
    sls_detector_get list

    # search for a particular command using a word
    sls_detector_get list | grep adc

    # get help for a particular command
    sls_detector_get -h fpath
    sls_detector_help fpath
  

Commands
-----------

.. include:: ../commands.rst


Depreciated commands
------------------------

.. note ::
	All the dac commands are preceded with the **dac** command. Use command **daclist** to get correct list of dac command arguments for current detector.

.. csv-table:: Depreciated commands
   :file: ../depreciated.csv
   :widths: 35, 35
   :header-rows: 1



