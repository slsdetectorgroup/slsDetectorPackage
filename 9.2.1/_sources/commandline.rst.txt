Command line interface
==============================================

Usage
-------------

The syntax is *'[detector index]-[module index]:[command]'*, where the indices are by default '0', when not specified.

Module index
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Modules are indexed based on their order in the hostname command. They are used to configure a specific module within a detector and are followed by a ':' in syntax.

    .. code-block::

        # Applies to all modules of detector 0
        sls_detector_put exptime 5s

        # Applies to only the 4th module
        sls_detector_put 3:exptime 5s


Detector index
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
This index is useful when configuring multiple detectors from a single host. Each detector uses a unique shared memory identified by a detector index, derived again from the hostname command. It is followed by a '-'. 

    .. code-block::

        # For detector with index 2 in shared memory
        sls_detector_put 2-hostname bchip133+bchip123+bchip456
        
        # Without '-', the detector index defaults to 0
        sls_detector_put hostname bchip133+bchip123+bchip456

        # Accessing all modules with detector index 2
        sls_detector_put 2-exptime

        # Starting acquisition only for detector with index 2
        sls_detector_put 2-start

        # Applies only to the 2nd detector, 4th module
        sls_detector_put 1-3:exptime 5s


Command Execution
^^^^^^^^^^^^^^^^^^^^^^^
Commands can be executed using:

*   **sls_detector_put**: setting values
*   **sls_detector_get**: getting values
*   **sls_detector**: automatically infers based on the number of arguments.
*   **sls_detector_help**: gets help on the specific command
*   **sls_detector_acquire**: initiates acquisition with the detector. This command blocks until the entire acquisition process is completed.



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



