# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
import subprocess
import locale
out = subprocess.run(['sls_detector_get', 'list'], stdout = subprocess.PIPE, encoding=locale.getpreferredencoding())
cmd = out.stdout.splitlines()
cmd.pop(0)

from slsdet import Detector

pycmd = dir(Detector)


# dacs are in general not included in the python commands and we expect to
# set them from the specialized class or using an enum 
dacs = [
        'iodelay',
        'list',
        'rxb_lb',
        'rxb_rb',
        'v_chip',
        'vb_comp',
        'vb_comp_adc',
        'vb_comp_fe',
        'vb_cs',
        'vb_ds',
        'vb_opa_1st',
        'vb_opa_fd',
        'vb_pixbuf',
        'vb_sda',
        'vbp_colbuf',
        'vcal',
        'vcal_n',
        'vcal_p',
        'vipre_out',
        'vcas',
        'vcasc_out',
        'vcasc_sfp',
        'vcascn_pb',
        'vcascp_pb',
        'vcassh',
        'vchip_comp_adc',
        'vchip_comp_fe',
        'vchip_cs',
        'vchip_opa_1st',
        'vchip_opa_fd',
        'vchip_ref_comp_fe',
        'vcmp_ll',
        'vcmp_lr',
        'vcmp_rl',
        'vcmp_rr',
        'vcn',
        'vcom_adc1',
        'vcom_adc2',
        'vcom_cds',
        'vcp',
        'vdcsh',
        'vdd_prot',
        'vicin',
        'vin_cm',
        'vin_com',
        'vipre',
        'vipre_cds',
        'vipre_out',
        'vishaper',
        'vout_cm',
        'vref_cds',
        'vref_comp',
        'vref_comp_fe',
        'vref_ds',
        'vref_h_adc',
        'vref_l_adc',
        'vref_prech',
        'vref_rstore',
        'vrpreamp',
        'vrshaper',
        'vrshaper_n',
        'vsvn',
        'vsvp',
        'vtgstv',
        'vth1',
        'vth2',
        'vth3',
        'vtrim',
        'ib_test_c',
        'ibias_sfp',

]

intentionally_missing  = [
                        'activate', #use getActive and getRxPadDeactivatedMode syntax is not a good fit for python
                        'temp_10ge', #temperatures already available from enum or specialized class
                        'temp_adc',
                        'temp_dcdc',
                        'temp_fpga',
                        'temp_fpgaext', 
                        'temp_fpgafl', 
                        'temp_fpgafr',
                        'temp_slowadc',
                        'temp_sodl',
                        'temp_sodr',
                        'update', #use updateServerAndFirmare
                        'udp_validate', #use validateUdpConfiguration
                        'udp_reconfigure', #use reconfigureUdpDestination
                        'pulse', # use pulseChip pulsePixel pulsePixelNmove
                        'pulsechip', 
                        'pulsenmove',
                        'savepattern', #use savePattern()
                        'resetfpga', #use resetFPGA()
                        'rebootcontroller', #use rebootController()
                        'firmwaretest', #use executeFirmwareTest
                        'bustest', # executeBusTest
                        'programfpga', #programFPGA
                        'dac', #use setDAC or detector specific class
                        'clearroi', #clearROI
]

pycmd += intentionally_missing
pycmd += dacs
missing = []
for c in cmd:
    if c not in pycmd:
        print(c)
        missing.append(c)

print(f'\nMissing: {len(missing)} commands')
print(f'Excluded: {len(dacs)} dacs')
print(f'Excluded: {len(intentionally_missing)} other commands')



not_in_cmd = []
for c in pycmd:
    if c.islower() and not c.startswith('_'):
        if c not in cmd:
            not_in_cmd.append(c)
print(f'\nCommands in Python and NOT in command line: {len(not_in_cmd)}')
for c in not_in_cmd:
    print(c)



#  print(',\n'.join([f'\'{d}\'' for d in sorted(dacs)]))
