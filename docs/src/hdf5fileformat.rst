.. _hdf5 file format:

HDF5 File Format
================================

Compilation
-------------

#. Compile the package with HDF5 option enabled

    #. Using cmk script: ./cmk.sh -hj9 -d [path of hdf5 dir] (-d is optional and for custom installation folder)

    #. Enable using cmake option **-DSLS_USE_HDF5=ON** and **-DCMAKE_INSTALL_PREFIX=/path/to/custom/hdf/installation** (optional).


Setup
-------

#. Start Receiver process

#. Load config file

#. Set file format using command `fformat <commandline.html#term-fformat-binary-hdf5>`_.

    .. code-block:: bash

        sls_detector_put fformat hdf5


Filters (Compression)
----------------------

HDF5 filters (eg. compression) for the image dataset in the data files are set using command `fformatoptions <commandline.html#term-fformatoptions-filter-id-param-1-param-2-...-or-none>`_ after setting the file format to hdf5. The default is no filters.

* Each filter is the HDF5 filter id followed by its parameters (cd_values), all unsigned integers.

* Multiple filters are applied in the given order.

* Filters that are not part of the HDF5 library (eg. 32008 for bitshuffle) must be available to the receiver as plugins found in **HDF5_PLUGIN_PATH**. Setting a filter that is not available to the receiver will throw an error.

* The same filter plugins must be available to the application reading the data files.

* Changing the file format resets the filters.

* List of existing filter plugins and information on writing custom filters: `HDF5 Link <https://github.com/HDFGroup/hdf5_plugins/blob/master/docs/RegisteredFilterPlugins.md>`_.

.. code-block:: bash

    # gzip (filter id 1) with compression level 6
    sls_detector_put fformatoptions 1 6

    # shuffle (filter id 2) followed by gzip
    sls_detector_put fformatoptions [2] [1,6]

    # no filters
    sls_detector_put fformatoptions none

    # get filters
    sls_detector_get fformatoptions


Master File
-------------

* File Name: [fpath]/[fname]_master_[findex].h5 :ref:`Details here<file name format>`

* It contains  :ref:`attributes<master file attributes>` relevant to the acquisition. This can vary with detector type.

.. code-block:: text

    /                                               # Root level
    |---> entry                                     # entry group
    |    |---> data                                 # data group
    |         |---> column                          # dataset of each sls_receiver_header member
    |         |---> data
    |         |---> detector header version
    |         |---> detector specific 1
    |         |---> detector specific 2
    |         |---> detector specific 3
    |         |---> detector specific 4
    |         |---> detector type
    |         |---> exp length or sub exposure time
    |         |---> frame number
    |         |---> mod id
    |         |---> packets caught
    |         |---> packets caught bit mask
    |         |---> row
    |         |---> timestamp
    |    |---> instrument                           # instrument group
    |         |---> beam                            # beam group
    |         |---> detector                        # detector group
    |              |---> Master File Attribute 1    # dataset of each master file attribute
    |              |---> Master File Attribute 2
    |              |---> Master File Attribute 3
    |              |---> Master File Attribute ..
    |    |---> sample                               # sample group


If more than 1 data file per frame:
    * The dataset of each :ref:`**SLS Receiver Header** <sls receiver header format>` member is a virtual dataset.
    * **data** dataset is a virtual dataset.


More details regarding master file attributes can be found :ref:`here<master file attributes>`.

Data File
-----------

* File Name: [fpath]/[fname]_dx_fy_[findex].h5 :ref:`Details here<file name format>`

* More details on :ref:`slsReceiverHeader<sls receiver header format>` and the actual image data is described in the :ref:`Detector Image Size and Format <data format>` section.


Virtual Data File
------------------

* File Name: [fpath]/[fname]_virtual_[findex].h5 :ref:`Details here<file name format>`

* For multiple modules, a virtual file linking data from all the modules is created. The individual files are expected to be present.

* It is linked in the master file.


