import subprocess
import locale
out = subprocess.run(['g', 'list'], stdout = subprocess.PIPE, encoding=locale.getpreferredencoding())
cmd = out.stdout.splitlines()
cmd.pop(0)

from slsdet import Detector, Eiger, Ctb

pycmd = dir(Detector)+dir(Eiger)+dir(Ctb)

#Add commands that we should not expect as direct commands in python
pycmd += ['vrf', 'vtr', 'vrs', 'vtgstv', 'vsvn', 'vtrim',
'vsvp', 'vth1', 'vth2', 'vth3', 'vshaper', 'vshaperneg', 'rxb_rb',
'rxb_lb', 'vref_prech', 'vref_rstore', 'vref_cds',
'vpreamp', 'vref_comp', 'vref_comp_fe vref_ds', 'vref_h_adc', 
'vref_l_adc', 'iodelay', 'list', 'vref_ds', 'vis', 'vpl', 
'vref_comp_fe', 'vph', 'vout_cm', 'vcp', 'vcn', 'vcmp_ll', 'vcmp_lr'

]


# dacs are in general not included in the python commands and we expect to
# set them from the specialized class or using an enum 
dacs = [
    'adcvpp',
    'vicin', 
    'vcassh', 
    'vcal_n', 
    'vcal_p'
    'vipre_out', 
    'vipre_cds',
    'vdd_prot',
    'vcmp_rl', 
    'vcmp_rr', 
    'vcal', 'vcas', 
    'vipre',
    'vin_com', 
    'vin_cm', 
    'vrshaper', 
    'vrshaper_n', 
    'vrpreamp', 
    'vishaper',
    'vipre_out',
    'vcom_adc1',
    'vcom_adc2',
    'vcom_cds',
    'vdcsh',
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
    'vcal_p',
    'vcasc_out',
    'vcasc_sfp',
    'vcascn_pb',
    'vcascp_pb',
    'vchip_comp_adc',
    'vchip_comp_fe',
    'vchip_cs',
    'vchip_opa_1st',
    'vchip_opa_fd',
    'vchip_ref_comp_fe',

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
                        'trigger', #use sendSoftwareTrigger
                        'update', #use updateServerAndFirmare
                        'udp_validate', #use validateUdpConfiguration
                        'udp_reconfigure', #use reconfigureUdpDestination
                        'emin', #use rx_jsonpara
                        'pulse', # use pulseChip pulsePixel pulsePixelNmove
                        'pulsechip', 
                        'pulsenmove',
]

pycmd += intentionally_missing
pycmd += dacs
missing = []
for c in cmd:
    if c not in pycmd:
        print(c)
        missing.append(c)

print(f'Missing: {len(missing)} commands')