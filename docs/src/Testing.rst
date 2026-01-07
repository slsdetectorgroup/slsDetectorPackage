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

If a test requires a detector mark them with the hidden tag ``[.detectorintegration]``. This only works if a configured simulator (or an actual configured detector) and receiver are already set up to run the tests.

.. code-block:: console

    tests "[.detectorintegration]"


If you want to disable a specific test for a specific detector add the hidden tag ``[.disable_<detector_name>]`` to the test case. Please note that only some specific disable tests have been implemented so far.

.. code-block:: console

    tests "[.detectorintegration] ~[.disable_jungfrau]"

Simulator Script: 
-----------------

One can also just run the following script, which will run your tests for all the detector types (simulators only) with a pre-determined configuration.

.. code-block:: console

    cd build 
    python bin/test_simulators.py 

This runs all tests marked with the tag ``[.detectorintegration]`` for all detector simulators.
If you want to run them for a specific virtual detector or a specific test use the following command:

.. code-block:: console
    cd build
    python bin/test_simulators.py --servers jungfrau --test "[.rx]"

You can exclude a specific detector for tests that have this marker by adding the option ``~[.disable_<detector_name>]``. Again, we assume that this marker is added to the tests that you want to exclude. 

.. code-block:: console
    cd build
    python bin/test_simulators.py --servers eiger jungfrau moench --test "[.detectorintegration] ~[.disable_jungfrau]"

Note that this still runs all the tests for the virtual jungfrau detector except for the ones marked with the tag ``[.disable_jungfrau]``. 

You can additionally run all the tests not requiring detectors using the script ``bin/test_simulators.py`` by passing the option ``--general_tests``. 


Pytest Tests
-----------------

To run the python tests use the following commands:

.. code-block:: console

    cd build
    cmake ../ -DSLS_USE_PYTHON=ON
    export PYTHONPATH=$PWD/bin 
    python -m pytest ../python/tests/ 

If a test requires a detector mark them with the pytest marker ``@pytest.mark.detectorintegration``.

To run only tests requiring virtual detectors use the following command:

.. code-block:: console
    #in build
    python -m pytest -m detectorintegration ../python/tests/

There is a helper test fixture in ``slsDetectorSoftware/python/tests/conftest.py`` called ``test_with_simulators`` that sets up virtual detectors and yields the test for all detectors. 

Example usage: 

.. code-block:: python

    import pytest

    @pytest.mark.detectorintegration
    def test_example_with_simulator(test_with_simulators):
        # your test code here

If you want to run the test only for a specific test use the parametrized test fixture: 

Example usage: 

.. code-block:: python

    import pytest

    @pytest.mark.detectorintegration
    @pytest.mark.parametrize("setup_parameters", [(["<my_detector>"], <num_modules>)], indirect=True)
    def test_example_with_specific_simulators(test_with_simulators, setup_parameters):
        # your test code here




