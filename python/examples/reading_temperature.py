from slsdet import Detector, Eiger, dacIndex



#Using the general detector class and calling with an index
d = Detector()
fpga_temp = d.getTemperature(dacIndex.TEMPERATURE_FPGA)
print(f'fpga_temp: {fpga_temp}\n')

#Using the specialized detector class
e = Eiger()
print("All temperatures for Eiger\n")
print(e.temp)
