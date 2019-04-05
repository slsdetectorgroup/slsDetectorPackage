#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue May 22 14:13:48 2018

@author: l_frojdh
"""
import os
from sls_detector_tools.io import write_trimbit_file
from sls_detector_tools import mask

energy = [5000, 6000, 7000]
vrf =     [500, 1000, 1500]

for i,e in enumerate(energy):
    dacs = np.array(  [[   0.,    0.],   #vsvp
                       [4000., 4000.],  #vtr
                       [vrf[i], vrf[i]],  #vrf
                       [1400., 1400.],  #vrs
                       [4000., 4000.],  #vsvn
                       [2556., 2556.],  #vtgstv
                       [1400., 1400.],  #vcmp_ll
                       [1500., 1500.],  #vcmp_lr
                       [4000., 4000.],  #vcall
                       [1500., 1500.],  #vcmp_rl
                       [1100., 1100.],  #rxb_rb
                       [1100., 1100.],  #rxb_lb
                       [1500., 1500.],  #vcmp_rr
                       [1500., 1500.],  #vcp
                       [2000., 2000.],  #vcn
                       [1550., 1550.],  #vis
                       [ 660.,  660.],  #iodelay
                       [   0.,    0.],  #tau
                       ]) 
    
    tb = np.zeros((256,1024))
    
    for beb in [83,98]:
        write_trimbit_file(f'settingsdir/standard/{e}eV/noise.sn{beb:03d}', tb, dacs[:,0])
#print(os.getcwd())
    
#print( os.path.realpath(__file__))