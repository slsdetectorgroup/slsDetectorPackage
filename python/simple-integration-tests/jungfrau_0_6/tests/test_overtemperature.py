
import pytest
import config_test
import time
from fixtures import jungfrau, jungfrautest

def powerchip_test(jungfrau, control):
    """

    Test the main overtemperature protection control

    """
    #Set test initial conditions
    print('\tStarting powerchip_test test case')

    jungfrau.power_chip = False
    jungfrau.temperature_control = control
    assert jungfrau.power_chip == False
    jungfrau.temperature_threshold = 35
    jungfrau.power_chip = True


    if jungfrau.temperature_control is True:
        if jungfrau.temperature_event is True:
            assert jungfrau.power_chip == False
            jungfrau.power_chip = True
            assert jungfrau.power_chip == False
            jungfrau.temperature_control = False
            assert jungfrau.power_chip == True
            jungfrau.temperature_control = True
            jungfrau.temperature_threshold = 50
            assert jungfrau.power_chip == False

            print('\t\tWaiting to cool down the board. This may take a while...')
            while jungfrau.temperature_threshold < jungfrau.temp.fpga[0]:
                time.sleep(5)
                print('\t\tJungfrau MCB temperature: {0:.2f} °C'.format(jungfrau.temp.fpga[0]))

            #Leave enough time to let the board cool down a bit more
            time.sleep(30)
            jungfrau.reset_temperature_event()

            assert jungfrau.temperature_event == False
            assert jungfrau.power_chip == True

        else:
            assert jungfrau.power_chip == True
    else:
        print('\t\tWaiting to warm up the board. This may take a while...')
        while jungfrau.temperature_threshold > jungfrau.temp.fpga[0]:
            time.sleep(5)
            print('\t\tJungfrau MCB temperature: {0:.2f} °C'.format(jungfrau.temp.fpga[0]))

        assert jungfrau.temperature_event == False
        assert jungfrau.power_chip == True

    print('\tFinished powerchip_test test case')


#@jungfrautest
def test_main(jungfrau):

    print('\nTesting overtemperature protection control')

    powerchip_test(jungfrau, False)
    powerchip_test(jungfrau, True)

    print('Tested overtemperature protection control')
