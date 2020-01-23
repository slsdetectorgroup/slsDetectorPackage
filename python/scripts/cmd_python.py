import subprocess
import locale
out = subprocess.run(['g', 'list'], stdout = subprocess.PIPE, encoding=locale.getpreferredencoding())
cmd = out.stdout.splitlines()
cmd.pop(0)

from sls_detector import Detector, Eiger, Ctb

pycmd = dir(Detector)+dir(Eiger)+dir(Ctb)

pycmd += ['vrf', 'vtr', 'vrs', 'vtgstv', 'vsvn', 'vtrim',
'vsvp', 'vth1', 'vth2', 'vth3', 'vshaper', 'vshaperneg', 'rxb_rb',
'rxb_lb', 'vref_prech', 'vref_rstore', 'vref_cds',
'vpreamp', 'vref_comp', 'vref_comp_fe vref_ds', 'vref_h_adc', 
'vref_l_adc', 'iodelay', 'list', 'vref_ds', 'vis', 'vpl', 
'vref_comp_fe', 'vph', 'vout_cm', 'vcp', 'vcn', 'vcmp_ll', 'vcmp_lr'
, 'vcmp_rl', 'vcmp_rr']

missing = []
for c in cmd:
    if c not in pycmd:
        print(c)
        missing.append(c)

print(f'Missing: {len(missing)} commands')