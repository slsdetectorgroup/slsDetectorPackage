
import pytest
import config_test
import os
dir_path = os.path.dirname(os.path.realpath(__file__))

from fixtures import jungfrau, jungfrautest


def load_config_file_jungfrau_test(jungfrau):
    """Load a settings file and assert all settings"""

    print('\tStarting load_config_file_jungfrau_test test case')

    jungfrau.free_shared_memory
    jungfrau.load_config(os.path.join(dir_path, 'test.config'))

    assert jungfrau.lock == False
    assert jungfrau.rx_udpport == ['1754']
    assert jungfrau.hostname == ['bchip094']
    assert jungfrau.firmware_version == config_test.fw_version

    print('\tFinished load_config_file_jungfrau_test test case')

def load_parameters_file_jungfrau_test(jungfrau):
    """Load a parametes file and assert the settings in the file"""

    print('\tStarting load_parameters_file_jungfrau_test test case')

    jungfrau.load_parameters(os.path.join(dir_path, 'test.par'))
    assert jungfrau.high_voltage == 200

    print('\tFinished load_parameters_file_jungfrau_test test case')

@jungfrautest
def test_main(jungfrau):
    print('\nTesting configuration file loading')

    load_config_file_jungfrau_test(jungfrau)
    load_parameters_file_jungfrau_test(jungfrau)

    print('Tested configuration file loading')

