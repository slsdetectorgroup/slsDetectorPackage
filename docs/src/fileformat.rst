File format
================================

If `fwrite <commandline.html#term-fwrite-0-1>`_ is enabled, the receiver will write data to files. 

Number of Files
----------------

Every acquisition will create a master file and data files. 

An acquisition can have multiple data files for a single frame. The number of files is determined by the number of UDP ports per module and the number of modules.

    * Every modules has its own receiver process. Every receiver process can have 1 or 2 UDP ports.
    * Each UDP port will create its own file. Therefore, each receiver can write 1 or 2 files.
    * So, for example a detector with 4 modules with 2 UDP ports each will create a total of 8 files with file names containing UDP port index **'_d0'** to **'_d7'**.

A new file containing **'_f[file_index]'** in file name is also created when reaching the maximum frames per file. Configured using `rx_framesperfile <commandline.html#term-rx_framesperfile-n_frames>`_.

.. _file name format:

Naming
-------
| Master File Name: [fpath]/[fname]_master_[findex].[ext]


| Data File Name: [fpath]/[fname]_dx_fy_[findex].[ext] 

   * fpath: file path set using command `fpath <commandline.html#term-fpath-path>`_. Default: '/'
   * fname: file name prefix using command `fname <commandline.html#term-fname-name>`_. Default: "run"
   * findex: acquisition index using command `findex <commandline.html#term-findex-n_value>`_. Automatically incremented for every acquisition with `sls_detector_acquire <commandline.html#term-acquire>`_ (if `fwrite <commandline.html#term-fwrite-0-1>`_ enabled).
   * x: unique udp port index. New file per UDP port.
   * y: file index. New file created after reaching max frames per file.
   * ext: file extension. Default: "raw"(data file) or "json"(master file)


Some file name examples:

    .. code-block:: bash

        # first file
        path-to-file/run_d0_f0_0.raw

        # first file for second UDP port
        path-to-file/run_d1_f0_0.raw

        # second file after reaching max frames in first file
        path-to-file/run_d0_f1_0.raw
        
        # second acquisition, first file
        path-to-file/run_d0_f0_1.raw


Formats
--------

There are 2 file formats supported by the receiver:

    * Binary - extension .json (master file) or .raw (data files)
    * HDF5 - extension .h5

The default is binary. HDF5 can be enabled by compiling the package with HDF5 option enabled. The file format is set using the command `fformat <commandline.html#term-fformat-binary-hdf5>`_.

