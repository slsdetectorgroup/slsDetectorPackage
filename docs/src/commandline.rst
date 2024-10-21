Command line interface
==============================================

Usage
-------------

Commands can be used either with sls_detector_get or sls_detector_put

.. code-block::

    sls_detector_get exptime

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

    # autocompletion
    # bash_autocomplete.sh or zsh_autocomplete.sh must be sourced from the 
    # main package folder to enable auto completion of commands and arguments 
    # for the command line on that shell.
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



