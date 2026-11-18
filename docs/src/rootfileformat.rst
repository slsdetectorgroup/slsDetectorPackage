.. _root file format:

ROOT File Format
================================

Compilation
-------------

#. Compile the package with ROOT option enabled using cmake option **-DSLS_USE_ROOT=ON**. ROOT must be found by cmake (eg. by sourcing thisroot.sh of the ROOT installation).


Setup
-------

#. Start Receiver process

#. Load config file

#. Set file format using command `fformat <commandline.html#term-fformat-binary-hdf5-root>`_.

    .. code-block:: bash

        sls_detector_put fformat root

#. Choose how the images are saved using command `fformatoptions <commandline.html#term-fformatoptions-filter-id-param-1-param-2-...-or-TTree-TH2I-or-none>`_. The default is TTree. Changing the file format resets it to the default.

    .. code-block:: bash

        # every image is an entry in a tree (default)
        sls_detector_put fformatoptions TTree

        # every image is a histogram
        sls_detector_put fformatoptions TH2I


Master File
-------------

* File Name: [fpath]/[fname]_master_[findex].json :ref:`Details here<file name format>`

* It is the same master file as for the :ref:`binary file format <binary file format>` and contains :ref:`attributes<master file attributes>` relevant to the acquisition.


Data File
-----------

* File Name: [fpath]/[fname]_dx_fy_[findex].root :ref:`Details here<file name format>`

* Every data file has a tree called **frames** with one entry per image. Every member of the :ref:`sls_detector_header<sls receiver header format>` of the image is a branch of this tree.

    =================  ===========================  =====================
    Branch             Type                         Content
    =================  ===========================  =====================
    frameNumber        64 bit unsigned integer      frame number
    expLength          32 bit unsigned integer      exposure length or sub exposure time
    packetNumber       32 bit unsigned integer      packets caught
    detSpec1           64 bit unsigned integer      detector specific 1
    timestamp          64 bit unsigned integer      timestamp
    modId              16 bit unsigned integer      module id
    row                16 bit unsigned integer      row
    column             16 bit unsigned integer      column
    detSpec2           16 bit unsigned integer      detector specific 2
    detSpec3           32 bit unsigned integer      detector specific 3
    detSpec4           16 bit unsigned integer      detector specific 4
    detType            8 bit unsigned integer       detector type
    version            8 bit unsigned integer       detector header version
    packetsMask        64 x 8 bit unsigned integer  packets caught bit mask
    =================  ===========================  =====================

* The image itself is saved depending on the file format option:

    * **TTree** (default): The image is the branch **data** of the tree **frames**. It is a two dimensional array [rows][columns] of unsigned integers of the size of the dynamic range. As in the HDF5 file format, 12 bit is saved as 16 bit and 4 bit is saved as it is sent by the detector (2 pixels in every 8 bit unsigned integer, so half the number of columns).

    * **TH2I**: The image is a histogram called **frame_[frame number]** with one bin per pixel (x axis: column, y axis: row). The bins are signed 32 bit integers, so larger values are saved as the maximum of a signed 32 bit integer (2147483647). The file has one histogram per image.


Reading
---------

.. code-block:: cpp

    TFile file("run_d0_f0_0.root");
    TTree *frames = (TTree *)file.Get("frames");
    ULong64_t frameNumber;
    frames->SetBranchAddress("frameNumber", &frameNumber);

    // TTree: [rows][columns] in the type of the dynamic range (here 16 bit)
    UShort_t data[512][1024];
    frames->SetBranchAddress("data", data);
    frames->GetEntry(0);

    // TH2I: histogram of the image with the frame number of the entry
    frames->GetEntry(0);
    TH2I *image = (TH2I *)file.Get(Form("frame_%llu", frameNumber));
    image->Draw("colz");
