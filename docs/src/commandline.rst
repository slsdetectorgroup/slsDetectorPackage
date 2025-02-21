Command line interface
==============================================

Usage
-------------

| **Detectors and Shared Memory:**
| Each detector uses a unique shared memory identified by a detector index, derived from the hostname. If the hostname contains a '-', the number preceding it is the detector index.

    .. code-block::

        # For detector with index 2 in shared memory
        sls_detector_put 2-hostname bchip133+bchip123+bchip456
        
        # Without '-', the detector index defaults to 0
        sls_detector_put hostname bchip133+bchip123+bchip456

        # Accessing all modules with detector index 2
        sls_detector_put 2-exptime

        # Starting acquisition only for detector wiht index 2
        sls_detector_put 2-start

| **Modules within a Detector:**
| Modules are indexed based on their order in the hostname list. To configure a specific module, prefix the command with the module index and ':'.

    .. code-block::

        # Applies to all modules of detector 0
        p exptime 5s

        # Applies to only the 4th module
        p 3:exptime 5s

| **Command Execution:**
| Commands can be executed using:

*   sls_detector_put: setting values
*   sls_detector_get: getting values
*   sls_detector: automatically infers based on the number of arguments.
*   sls_detector_help: gets help on the specific command
*   sls_detector_acquire: initiates acquisition with the detector. This command blocks until the entire acquisition process is completed.



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

    # list of deprecated commands
    list deprecated


Autocompletion
---------------

bash_autocomplete.sh or zsh_autocomplete.sh must be sourced from the main package folder to enable auto completion of commands and arguments for the command line on that shell.

.. code-block::

    source bash_autocomplete.sh
  

Commands
-----------

.. include:: ../commands.rst


Deprecated commands
------------------------

.. note ::
	All the dac commands are preceded with the **dac** command. Use command **daclist** to get correct list of dac command arguments for current detector.

.. csv-table:: Deprecated commands
   :file: ../deprecated.csv
   :widths: 35, 35
   :header-rows: 1



