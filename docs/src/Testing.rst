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

You can run all tests requiring a detector by running the following command: 

.. code-block:: console

    tests "[.detectorintegration]"


.. note ::   

    This only works if a configured simulator (or an actual configured detector) and receiver are already set up to run the tests. There is a script that automatically sets up virtual detectors and runs all integration tests. See Section :ref:`Simulator Script. <python simulator script>`  below.


If you want to disable testing that involves a data file that require pc tuning optimizations, add the hidden tag ``[.disable_check_data_file]`` to the test case. Please note that only some specific disable tests have been implemented so far.

.. code-block:: console

    tests "[detectorintegration]~[disable_check_data_file]"

.. note ::   

    Ensure that there are no spaces between the tags and no '.', else hardly any test will be matched.

.. _python simulator script:

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
    python bin/test_simulators.py --servers jungfrau --test "[dacs]"

You can exclude specific tests by adding the option ``~[<disable_test_name>]``. Again, we assume that this marker is added to the tests that you want to exclude. 

.. code-block:: console

    cd build
    python bin/test_simulators.py --servers eiger jungfrau moench --test "[detectorintegration]~[disable_check_data_file]"

You can additionally run all the tests not requiring detectors using the script ``bin/test_simulators.py`` by passing the option ``--general-tests``. 

One can use ``--no-log-file`` if you dont want to create a log files and instead print to console. If you prefer to not print to console either, add ``--quiet``.


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

There is a helper test fixture in ``slsDetectorSoftware/python/tests/conftest.py`` called ``session_simulator`` that sets up virtual detectors and yields the test for all detectors. The set up is done for every test automatically. Note that the fixture persists over the entire session e.g. the fixture is setup one detector at a time and runs all tests using this fixture before cleaning up and moving on to the next detector. It saves time if the setup and cleanup is expensive.

Example usage: 

.. code-block:: python

    import pytest

    @pytest.mark.detectorintegration
    def test_example_with_simulator(session_simulator):
        # your test code here

.. Note:: 
    As the detector is set up only once makes sure to not change the state of the detector in a way that affects other tests. If you want to change the state of the detector make sure to reset it at the end of your test.

If you want to run the test only for a specific detector use the parametrized test fixture: 

Example usage: 

.. code-block:: python

    import pytest

    @pytest.mark.detectorintegration
    @pytest.mark.parametrize("session_simulator", [("<my_detector>", <num_interfaces>, <num_modules>), ("<another_detector>", <num_interfaces>, <num_modules>)], indirect=True)
    def test_example_with_specific_simulators(session_simulator):
        # your test code here

.. Note::
    The parametrized test fixture is setup per file and not for the entire session. 

