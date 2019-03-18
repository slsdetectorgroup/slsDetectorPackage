#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Jungfrau detector class and support functions.
Inherits from Detector.
"""
from .adcs import Adc, DetectorAdcs
from .decorators import error_handling
from .detector import Detector
from .dacs import DetectorDacs
from .utils import element_if_equal


class JungfrauDacs(DetectorDacs):
    _dacs = [('vb_comp',    0, 4000,    1220),
             ('vdd_prot',   0, 4000,    3000),
             ('vin_com',    0, 4000,    1053),
             ('vref_prech', 0, 4000,    1450),
             ('vb_pixbuff', 0, 4000,     750),
             ('vb_ds',      0, 4000,    1000),
             ('vref_ds',    0, 4000,     480),
             ('vref_comp',  0, 4000,     420),
            ]
    _dacnames = [_d[0] for _d in _dacs]

class Jungfrau(Detector):
    """
    Class used to control a Jungfrau detector. Inherits from the Detector class but a specialized
    class is needed to provide the correct dacs and unique functions.

    """
    _detector_dynamic_range = [4, 8, 16, 32]

    _settings = ['dynamichg0',
                 'dynamicgain',
                 'fixgain1',
                 'fixgain2',
                 'forceswitchg1',
                 'forceswitchg2']
    """Available settings for Jungfrau"""

    def __init__(self, multi_id=0):
        #Init on base calss
        super().__init__(multi_id)
        self._dacs = JungfrauDacs(self)

        #Jungfrau specific temps, this can be reduced to a single value?
        self._temp = DetectorAdcs()
        self._temp.fpga = Adc('temp_fpga', self)
        # self._register = Register(self)


    @property
    def dacs(self):
        """

        An instance of DetectorDacs used for accessing the dacs of a single
        or multi detector.

        Examples
        ---------

        ::

            #Jungfrau


        """
        return self._dacs

    @property
    @error_handling
    def power_chip(self):
        """Power on or off the ASICs, True for on False for off"""
        return self._api.isChipPowered()

    @power_chip.setter
    @error_handling
    def power_chip(self, value):
        self._api.powerChip(value)

    @property
    @error_handling
    def delay(self):
        """Delay after trigger [s]"""
        return self._api.getDelay()/1e9

    @delay.setter
    @error_handling
    def delay(self, t):
        ns_time = int(t * 1e9)
        self._api.setDelay(ns_time)

    @property
    @error_handling
    def n_gates(self):
        return self._api.getNumberOfGates()

    @n_gates.setter
    @error_handling
    def n_gates(self, n):
        self._api.setNumberOfGates(n)

    @property
    @error_handling
    def n_probes(self):
        return self._api.getNumberOfProbes()

    @n_probes.setter
    @error_handling
    def n_probes(self, n):
        self._api.setNumberOfProbes(n)

    @property
    @error_handling
    def storagecell_start(self):
        """
        First storage cell
        """
        return self._api.getStoragecellStart()

    @storagecell_start.setter
    @error_handling
    def storagecell_start(self, value):
        self._api.setStoragecellStart(value)


    @property
    @error_handling
    def n_storagecells(self):
        """
        number of storage cells used for the measurements
        """
        return self._api.getNumberOfStorageCells()

    @n_storagecells.setter
    @error_handling
    def n_storagecells(self, value):
        self._api.setNumberOfStorageCells(value)

    @property
    def temp(self):
        """
        An instance of DetectorAdcs used to read the temperature
        of different components

        Examples
        -----------

        ::

            detector.temp
            >>
            temp_fpga     :  36.90°C,  45.60°C

            a = detector.temp.fpga[:]
            a
            >> [36.568, 45.542]


        """
        return self._temp

    @property
    def temperature_threshold(self):
        """Threshold for switching of chips"""
        return self._api.getThresholdTemperature()

    @temperature_threshold.setter
    def temperature_threshold(self, t):
        self._api.setThresholdTemperature(t)

    @property
    def temperature_control(self):
        """
        Monitor the temperature of the detector and switch off chips if temperature_threshold is
        crossed


        Examples
        ---------

        ::

            #activate
            detector.temperature_control = True

            #deactivate
            detector.temperature_control = False


        """
        return self._api.getTemperatureControl()

    @temperature_control.setter
    def temperature_control(self, v):
        self._api.setTemperatureControl(v)

    @property
    def temperature_event(self):
        """Have the temperature threshold been crossed?

        Returns
        ---------

            :py:obj:`True` if the threshold have been crossed and temperature_control is active
            otherwise :py:obj:`False`

        """
        return self._api.getTemperatureEvent()

    def reset_temperature_event(self):
        """Reset the temperature_event. After reset temperature_event is False"""
        self._api.resetTemperatureEvent()

    @property
    @error_handling
    def rx_udpport(self):
        """
        UDP port for the receiver. Each module have one port.
        Note! Eiger has two ports

        ::

            [0:rx_udpport]

        Examples
        -----------

        ::

            d.rx_udpport
            >> [50010]

            d.rx_udpport = [50010]

        """
        return self._api.getNetworkParameter('rx_udpport')


    @rx_udpport.setter
    @error_handling
    def rx_udpport(self, ports):
        """Requires iterating over elements two and two for setting ports"""
        for i, p in enumerate(ports):
            self._api.setNetworkParameter('rx_udpport', str(p), i)

    @property
    def detector_mac(self):
        s = self._api.getNetworkParameter('detectormac')
        return element_if_equal(s)


    @detector_mac.setter
    def detector_mac(self, mac):
        if isinstance(mac, list):
            for i, m in enumerate(mac):
                self._api.setNetworkParameter('detectormac', m, i)
        else:
            self._api.setNetworkParameter('detectormac', mac, -1)


    @property
    @error_handling
    def detector_ip(self):
        s = self._api.getNetworkParameter('detectorip')
        return element_if_equal(s)

    @detector_ip.setter
    def detector_ip(self, ip):
        if isinstance(ip, list):
            for i, addr in enumerate(ip):
                self._api.setNetworkParameter('detectorip', addr, i)
        else:
            self._api.setNetworkParameter('detectorip', ip, -1)
