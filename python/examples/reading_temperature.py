# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from slsdet import Detector, Eiger, dacIndex



#Using the general detector class and calling with an index
d = Detector()
fpga_temp = d.getTemperature(dacIndex.TEMPERATURE_FPGA)
print(f'fpga_temp: {fpga_temp}\n')

#Using the specialized detector class
e = Eiger()
print("All temperatures for Eiger\n")
print(e.temp)
# >>> e.temp
# temp_fpga      :   54°C   60°C
# temp_fpgaext   :   49°C   52°C
# temp_10ge      :   47°C   45°C
# temp_dcdc      :   52°C   53°C
# temp_sodl      :   51°C   53°C
# temp_sodl      :   51°C   51°C
# temp_fpgafl    :   45°C   49°C
# temp_fpgafr    :   39°C   42°C

# The temperatures can also be returned in a dictionary
t = e.temp.to_dict()
print(t)
# >>> e.temp.to_dict()
# {'fpga': array([55, 60]), 'fpgaext': array([49, 52]), 
# 't10ge': array([47, 45]), 'dcdc': array([52, 53]), 
# 'sodl': array([51, 53]), 'sodr': array([51, 51]), '
# temp_fpgafl': array([45, 49]), 
# 'temp_fpgafr': array([39, 42])}

# or in a numpy array
t = e.temp.to_array()
print(t)
# >>> e.temp.to_array()
# array([[55, 60],
#        [49, 52],
#        [47, 45],
#        [52, 53],
#        [51, 53],
#        [51, 51],
#        [45, 49],
#        [40, 43]])
