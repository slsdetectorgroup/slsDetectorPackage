# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Dec  6 11:51:18 2017

@author: l_frojdh
"""

from .detector import Detector
from .temperature import Temperature, DetectorTemperature
from .dacs import DetectorDacs
from . import _slsdet
dacIndex = _slsdet.slsDetectorDefs.dacIndex
from .detector_property import DetectorProperty

class EigerVcmp:
    """
    Convenience class to be able to loop over vcmp for Eiger
    
    .. todo::
        
        Support single assignment and perhaps unify with Dac class
    
    """
    
    def __init__(self, detector):
        _dacs = [ dacIndex.VCMP_LL,
                  dacIndex.VCMP_LR,
                  dacIndex.VCMP_RL,
                  dacIndex.VCMP_RR]
        self.set = []
        self.get = []
        for i in range(detector.size()):
            if i % 2 == 0:
                dacs = _dacs
            else:
                dacs = _dacs[::-1]
            for d in dacs:
                self.set.append(lambda x, d=d, i=i : detector.setDAC(d, x, False, [i]))
                self.get.append(lambda d=d, i=i : detector.getDAC(d, False, [i])[0])
    
    def __getitem__(self, key):
        if key == slice(None, None, None):
            return [_d() for _d in self.get]
        return self.get[key]()
    
    def __setitem__(self, i, value):
        self.set[i](value)

    def __repr__(self):
        return 'vcmp: '+ str(self[:])


class EigerDacs(DetectorDacs):
    """
    Eiger specific dacs
    """
    _dacs = [('vsvp',     dacIndex.VSVP,0, 4000,    0),
             ('vtrim',    dacIndex.VTRIM,0, 4000, 2500),
             ('vrpreamp', dacIndex.VRPREAMP,0, 4000, 3300),
             ('vrshaper', dacIndex.VRSHAPER,0, 4000, 1400),
             ('vsvn',     dacIndex.VSVN,0, 4000, 4000),
             ('vtgstv',   dacIndex.VTGSTV,0, 4000, 2556),
             ('vcmp_ll',  dacIndex.VCMP_LL,0, 4000, 1500),
             ('vcmp_lr',  dacIndex.VCMP_LR,0, 4000, 1500),
             ('vcal',    dacIndex.VCAL,0, 4000, 4000),
             ('vcmp_rl',  dacIndex.VCMP_RL,0, 4000, 1500),
             ('rxb_rb',   dacIndex.RXB_RB,0, 4000, 1100),
             ('rxb_lb',   dacIndex.RXB_LB,0, 4000, 1100),
             ('vcmp_rr',  dacIndex.VCMP_RR,0, 4000, 1500),
             ('vcp',      dacIndex.VCP,0, 4000,  200),
             ('vcn',      dacIndex.VCN,0, 4000, 2000),
             ('vishaper', dacIndex.VISHAPER,0, 4000, 1550),
             ('iodelay',  dacIndex.IO_DELAY,0, 4000,  660)]
    _dacnames = [_d[0] for _d in _dacs]


from .detector import freeze

@freeze
class Eiger(Detector):
    """
    Subclassing Detector to set up correct dacs and detector specific 
    functions. 
    """
    _detector_dynamic_range = [4, 8, 16, 32]


    _settings = ['standard', 'highgain', 'lowgain', 'veryhighgain', 'verylowgain']
    """available settings for Eiger, note almost always standard"""

    def __init__(self, id=0):
        super().__init__(id)
        self._frozen = False 
        self._dacs = EigerDacs(self)
        self._vcmp = EigerVcmp(self)

        # Eiger specific adcs
        self._temp = DetectorTemperature()
        self._temp.fpga = Temperature('temp_fpga', dacIndex.TEMPERATURE_FPGA, self)
        self._temp.fpgaext = Temperature('temp_fpgaext', dacIndex.TEMPERATURE_FPGAEXT, self)
        self._temp.t10ge = Temperature('temp_10ge', dacIndex.TEMPERATURE_10GE, self)
        self._temp.dcdc = Temperature('temp_dcdc', dacIndex.TEMPERATURE_DCDC, self)
        self._temp.sodl = Temperature('temp_sodl', dacIndex.TEMPERATURE_SODL, self)
        self._temp.sodr = Temperature('temp_sodl', dacIndex.TEMPERATURE_SODR, self)
        self._temp.temp_fpgafl = Temperature('temp_fpgafl', dacIndex.TEMPERATURE_FPGA2, self)
        self._temp.temp_fpgafr = Temperature('temp_fpgafr', dacIndex.TEMPERATURE_FPGA3, self)



    @property
    def dacs(self):
        """

        An instance of DetectorDacs used for accessing the dacs of a single
        or multi detector.

        Examples
        ---------

        ::

            d = Eiger()

            #Set all vrf to 1500
            d.dacs.vrf = 1500

            #Check vrf
            d.dacs.vrf
            >> vrf       :  1500,  1500

            #Set a single vtr
            d.dacs.vtr[0] = 1800

            #Set vrf with multiple values
            d.dacs.vrf = [3500,3700]
            d.dacs.vrf
            >> vrf       :  3500,  3700

            #read into a variable
            var = d.dacs.vrf[:]

            #set multiple with multiple values, mostly used for large systems
            d.dacs.vcall[0,1] = [3500,3600]
            d.dacs.vcall
            >> vcall     :  3500,  3600

            d.dacs
            >>
            ========== DACS =========
            vsvp      :     0,     0
            vtr       :  4000,  4000
            vrf       :  1900,  1900
            vrs       :  1400,  1400
            vsvn      :  4000,  4000
            vtgstv    :  2556,  2556
            vcmp_ll   :  1500,  1500
            vcmp_lr   :  1500,  1500
            vcall     :  4000,  4000
            vcmp_rl   :  1500,  1500
            rxb_rb    :  1100,  1100
            rxb_lb    :  1100,  1100
            vcmp_rr   :  1500,  1500
            vcp       :  1500,  1500
            vcn       :  2000,  2000
            vis       :  1550,  1550
            iodelay   :   660,   660

        """
        return self._dacs


    @property
    def vcmp(self):
        """
        Convenience function to get and set the individual vcmp of chips
        Used mainly in the calibration code.

        Examples
        ---------

        ::

            #Reading
            d.vcmp[:]
            >> [500, 500, 500, 500, 500, 500, 500, 500]

            #Setting
            d.vcmp = [500, 500, 500, 500, 500, 500, 500, 500]


        """

        return self._vcmp
    
    @vcmp.setter
    def vcmp(self, values):
        if len(values) == len(self._vcmp.set):
            for i, v in enumerate(values):
                self._vcmp.set[i](v)
        else:
            raise ValueError('vcmp only compatible with setting all')

#     @property
#     def rx_udpport(self):
#         """
#         UDP port for the receiver. Each module has two ports referred to
#         as rx_udpport and rx_udpport2 in the command line interface
#         here they are grouped for each detector
        
#         ::
            
#             [0:rx_udpport, 0:rx_udpport2, 1:rx_udpport ...]
        
#         Examples
#         -----------
        
#         ::
        
#             d.rx_udpport
#             >> [50010, 50011, 50004, 50005]
            
#             d.rx_udpport = [50010, 50011, 50012, 50013]
        
#         """
#         p0 = self._api.getReceiverUDPPort()
#         p1 = self._api.getReceiverUDPPort2()
#         return [int(val) for pair in zip(p0, p1) for val in pair]
    
#     @rx_udpport.setter
#     def rx_udpport(self, ports):
#         """Requires iterating over elements two and two for setting ports"""
#         a = iter(ports)
#         for i, p in enumerate(zip(a, a)):
#             self._api.setReceiverUDPPort(p[0], i)
#             self._api.setReceiverUDPPort2(p[1], i)

    @property
    def rx_zmqport(self):
        """
        Return the receiver zmq ports. Note that Eiger has two ports per receiver!
        This functions therefore differ from the base class.

        ::

            e.rx_zmqport
            >> [30001, 30002, 30003, 30004]


        """
        ports = self.getRxZmqPort()
        return [p + i for p in ports for i in range(2)]


#     @rx_zmqport.setter
#     def rx_zmqport(self, port):
#         if isinstance(port, Iterable):
#             for i, p in enumerate(port):
#                 self._api.setReceiverStreamingPort(p, i)
#         else:
#             self._api.setReceiverStreamingPort(port, -1)

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
            temp_fpgaext  :  31.50°C,  32.50°C
            temp_10ge     :   0.00°C,   0.00°C
            temp_dcdc     :  36.00°C,  36.00°C
            temp_sodl     :  33.00°C,  34.50°C
            temp_sodr     :  33.50°C,  34.00°C
            temp_fpgafl   :  33.81°C,  30.93°C
            temp_fpgafr   :  27.88°C,  29.15°C

            a = detector.temp.fpga[:]
            a
            >> [36.568, 45.542]


        """
        return self._temp

