Installation
=========================

The easiest way to install the Python API and the slsDetectorPackage is using conda. But other
methods are also available. 

---------------------
Install using conda
---------------------
If you don't have it installed get the latest version of `Miniconda`_

.. _Miniconda: https://conda.io/miniconda.html

::

    wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh
    sh Miniconda3-latest-Linux-x86_64.sh


Install sls_detector and sls_detector_lib using:

::

    #Add conda channels
    conda config --add channels conda-forge
    conda config --add channels slsdetectorgroup

    #Install latest version
    conda install sls_detector

    #Install specific version
    conda install sls_detector=3.0.1

------------------------------
Local build using conda-build
------------------------------

Needs the `sls_detector_lib`_ installed  in order to automatically find headers
and shared libraries. Make sure that the branch of sls_detector matches the lib 
version installed. 

.. _sls_detector_lib: https://github.com/slsdetectorgroup/sls_detector_lib

::

    #Clone source code
    git clone https://github.com/slsdetectorgroup/sls_detector.git

    #Checkout the branch needed
    git checkout 3.0.1

    #Build and install the local version
    conda-build sls_detector
    conda install --use-local sls_detector


-----------------------
Developer build
-----------------------

IF you if you are developing and are making constant changes to the code it's a bit cumbersome
to build with conda and install. Then an easier way is to build the C/C++ parts in the package
directory and temporary add this to the path

::

    #in path/to/sls_detector
    python setup.py build_ext --inplace

Then in your Python script

::

    import sys
    sys.path.append('/path/to/sls_detector')
    from sls_detector import Detector



--------------
Prerequisites
--------------

All dependencies are manged trough conda but for a stand alone build you would need

 * gcc 4.8+
 * Qwt 6
 * Qt 4.8
 * numpy
 * slsDetectorPackage
