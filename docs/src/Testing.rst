Testing
==========

We use ``catch2`` and ``pytest`` for unit testing the C++ and Python code. 

CATCH2 Tests
----------------

To build and run the catch2 tests use the following commands: 

.. code-block:: console

    cd build
    cmake -DSLS_USE_TESTS=ON ../
    bin/tests 

Note that this requires that you have catch2 installed on your system.

Naming Policy: 
-----------------

Per default all tests should be visible to catch2. 

If a test fails in the github or gitea actions hide the test by adding the tag ``[.]`` to the test name.

.. code-block:: cpp

    TEST_CASE("This test is hidden from default runs", "[.]") {
        REQUIRE(1 == 2);
    }

If a test requires virtual detector mark them with the hidden tag ``[.cmdcall]``.

If you want to run tests requiring detector simulators run them as follows: 

.. code-block:: console

    cd build 
    python bin/test_simulators.py 

This runs all tests marked with the tag ``[.cmdcall]`` for all detector simulators.
If you want to run them for a specific virtual detector or a specific test use the following command:

.. code-block:: console
    cd build
    python bin/test_simulators.py --servers <detector_name> --test <test_name>


Pytest Tests
-----------------

To run the python tests use the following commands:

.. code-block:: console

    cd build
    cmake ../ -DSLS_USE_PYTHON=ON
    export PYTHONPATH=$PWD/bin 
    python -m pytest ../python/tests/ 

If a test requires a virtual detector mark them with the pytest marker ``@pytest.mark.withdetectorsimulators``.

To run only tests requiring virtual detectors use the following command:

.. code-block:: console
    #in build
    python -m pytest -m withdetectorsimulators ../python/tests/

There is a helper test fixture in ``slsDetectorSoftware/python/tests/conftest.py`` called ``test_with_simulators`` that sets up virtual detectors and yields the test for all detectors. 

Example usage: 

.. code-block:: python

    import pytest

    @pytest.mark.withdetectorsimulators
    def test_example_with_simulator(test_with_simulators):
        # your test code here

If you want to run the test only for a specific test use the parametrized test fixture: 

Example usage: 

.. code-block:: python

    import pytest

    @pytest.mark.withdetectorsimulators
    @pytest.mark.parametrize("setup_parameters", [(["<my_detector>"], <num_modules>)], indirect=True)
    def test_example_with_specific_simulators(test_with_simulators, setup_parameters):
        # your test code here




