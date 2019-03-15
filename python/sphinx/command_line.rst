Command line to Python
=========================

If you are already familiar with the command line interface to the
slsDetectorSoftware here is a quick reference translating to Python commands


 .. note ::

     Commands labeled Mythen only  or Gotthard only are currently not implemented in the
     Python class. If you need this functionallity please contact the SLS Detector Group

.. py:currentmodule:: sls_detector

.. |ro| replace:: *(read only)*
.. |free| replace:: :py:func:`Detector.free_shared_memory`
.. |sub| replace:: :py:attr:`Detector.sub_exposure_time`
.. |mg| replace:: Mythen and Gotthard only
.. |g| replace:: Gotthard only
.. |m| replace:: Mythen only
.. |msp| replace:: :py:attr:`Detector.measured_subperiod`
.. |new_chiptest| replace:: New chip test board only
.. |chiptest| replace:: Chip test board only
.. |dr| replace::  :py:attr:`Detector.dynamic_range`
.. |j| replace:: Jungfrau only
.. |te| replace:: :py:attr:`Detector.trimmed_energies`
.. |temp_fpgaext| replace:: :py:attr:`Detector.temp`.fpgaext
.. |epa| replace:: :py:func:`Eiger.pulse_all_pixels`
.. |rfc| replace:: :py:func:`Detector.reset_frames_caught`
.. |rfi| replace:: :py:attr:`Detector.receiver_frame_index`
.. |ron| replace:: :py:attr:`Detector.receiver_online`
.. |flipy| replace:: :py:attr:`Detector.flipped_data_y`
.. |flipx| replace:: :py:attr:`Detector.flipped_data_x`
.. |adcr| replace:: :py:func:`DetectorApi.writeAdcRegister`
.. |sb| replace:: :py:func:`DetectorApi.setBitInRegister`
.. |cb| replace:: :py:func:`DetectorApi.clearBitInRegister`
.. |tempth| replace:: :py:attr:`Jungfrau.temperature_threshold`
.. |tempev| replace:: :py:attr:`Jungfrau.temperature_event`
.. |tempco| replace:: :py:attr:`Jungfrau.temperature_control`
.. |depr| replace:: *Deprecated/Internal*
.. |nimp| replace:: *Not implemented*
.. |rudp| replace:: :py:attr:`Detector.rx_realudpsocksize`
.. |lci| replace:: :py:attr:`Detector.last_client_ip`
.. |rlci| replace:: :py:attr:`Detector.receiver_last_client_ip`
.. |fdp| replace:: :py:attr:`Detector.frame_discard_policy`
.. |apic| replace:: :py:attr:`Detector.api_compatibility`


------------------------
Commands
------------------------

===================== ===================================== ================== =========
Command               Python                                Implementation     Tests
===================== ===================================== ================== =========
sls_detector_acquire  :py:func:`Detector.acq`                OK                 OK
test                  |depr|                                 \-                 \-
help                  help(Detector.acq)                     \-                 \-
exitserver            |depr|                                 \-                 \-
exitreceiver          |depr|                                 \-                 \-
flippeddatay          |flipy|                                OK                 \-
digitest              |depr|                                 \-                 \-
bustest               |depr|                                 \-                 \-
digibittest           Which detector?                        \-                 \-
reg                   :py:attr:`Detector.register`           OK                 \-
adcreg                |adcr|                                 OK                 \-
setbit                |sb|                                   OK                 \-
clearbit              |cb|                                   OK                 \-
getbit                |nimp|                                 \-                 \-
r_compression         Not implemented in receiver            \-                 \-
acquire               :py:func:`Detector.acq`                OK                 \-
busy                  :py:attr:`Detector.busy`               OK                 Partial
status                :py:attr:`Detector.status`             OK |ro|            \-
status start          :py:func:`Detector.start_detector`     OK                 \-
status stop           :py:func:`Detector.stop_detector`      OK                 \-
data                  |depr|                                 \-                 \-
frame                 |depr|                                 \-                 \-
readctr               |g|                                    \-                 \-
resetctr              |g|                                    \-                 \-
resmat                :py:attr:`Eiger.eiger_matrix_reset`    OK                 OK
free                  |free|                                 OK                 \-
hostname              :py:attr:`Detector.hostname`           OK                 OK
add                   |nimp|                                 \-                 \-
replace               |nimp|                                 \-                 \-
user                  |nimp|                                 \-                 \-
master                |nimp|                                 \-                 \-
sync                  Which detector?                        \-                 \-
online                :py:attr:`Detector.online`             OK                 \-
checkonline           |nimp|                                 \-                 \-
activate              :py:attr:`Eiger.active`                \-                 \-
nmod                  :py:attr:`Detector.n_modules`          OK                 \-
maxmod                |depr|                                 \-                 \-
dr                    |dr|                                   OK                 OK
roi                   |g|                                    \-                 \-
detsizechan           :py:attr:`Detector.image_size`         OK                 \-
roimask               |nimp|                                 \-                 \-
flippeddatax          |flipx|                                OK                 \-
tengiga               :py:attr:`Eiger.tengiga`               OK                 \-
gappixels             :py:attr:`Eiger.add_gappixels`         OK                 \-
flags                 :py:attr:`Detector.flags`              OK                 \-
extsig                |mg|                                   \-                 \-
programfpga           |j|                                    \-                 \-
resetfpga             |j|                                    \-                 \-
powerchip             :py:attr:`Jungfrau.powerchip`          \-                 \-
led                   |nimp|                                 \-                 \-
auto_comp_disable     |j|                                    \-                 \-
pulse                 Used in |epa|                          OK                 \-
pulsenmove            Used in |epa|                          OK                 \- 
pulsechip             :py:func:`Eiger.pulse_chip`            OK                 \-
checkdetversion       |apic|                                 \-                 \-
checkrecversion       |apic|                                 \-                 \-
moduleversion         |m|                                    \-                 \-
detectornumber        :py:attr:`Detector.detector_number`    OK                 \-
modulenumber          |m|                                    \-                 \-
detectorversion       :py:attr:`Detector.firmware_version`   OK                 OK
softwareversion       :py:attr:`Detector.server_version`     \-                 \-
thisversion           :py:attr:`Detector.client_version`     Reads date         \-
receiverversion       :py:attr:`Detector.receiver_version`   Reads date         \-
timing                :py:attr:`Detector.timing_mode`        OK                 \-
exptime               :py:attr:`Detector.exposure_time`      OK                 OK
subexptime            |sub|                                  OK                 OK
period                :py:attr:`Detector.period`             OK                 OK
subdeadtime           :py:attr:`Eiger.sub_deadtime`          OK                 OK
delay                 :py:attr:`Jungfrau.delay`              OK                 \-
gates                 :py:attr:`Jungfrau.n_gates`            OK                 \-
frames                :py:attr:`Detector.n_frames`           OK                 OK
cycles                :py:attr:`Detector.n_cycles`           OK                 \-
probes                :py:attr:`Jungfrau.n_probes`           OK                 \-
measurements          :py:attr:`Detector.n_measurements`     OK                 \-
samples               Chip test board only (new?)            \-                 \-
storagecells          :py:attr:`Jungfrau.n_storagecells`     OK                 \-
storagecell_start     :py:attr:`Jungfrau.storagecell_start`  OK                 \-
exptimel              |mg|                                   \-                 \-
periodl               |mg|                                   \-                 \-
delayl                |mg|                                   \-                 \-
gatesl                |mg|                                   \-                 \-
framesl               |mg|                                   \-                 \-
cyclesl               |mg|                                   \-                 \-
probesl               |mg|                                   \-                 \-
now                   |nimp|                                 \-                 \-
timestamp             |m|                                    \-                 \-
nframes               |nimp|                                 \-                 \-
measuredperiod        :py:attr:`Detector.measured_period`    OK                 \-
measuredsubperiod     |msp|                                  \-                 \-
clkdivider            :py:attr:`Detector.readout_clock`      OK                 OK
setlength             |m|                                    \-                 \-
waitstates            |m|                                    \-                 \-
totdivider            |m|                                    \-                 \-
totdutycycle          |m|                                    \-                 \-
phasestep             |g|                                    \-                 \-
oversampling          |new_chiptest|                         \-                 \-
adcclk                |new_chiptest|                         \-                 \-
adcphase              |new_chiptest|                         \-                 \-
adcpipeline           |new_chiptest|                         \-                 \-
dbitclk               |new_chiptest|                         \-                 \-
dbitphase             |new_chiptest|                         \-                 \-
dbitpipeline          |new_chiptest|                         \-                 \-
config                :py:func:`Detector.load_config`        OK                 \-
rx_printconfig        |nimp|                                 \-                 \-
parameters            :py:func:`Detector.load_parameters`    OK                 \-
setup                 |nimp|                                 \-                 \-
flatfield             |nimp|                                 \-                 \-
ffdir                 |nimp|                                 \-                 \-
ratecorr              :py:attr:`Detector.rate_correction`    OK                 \-
badchannels           |nimp|                                 \-                 \-
angconv               |m|                                    \-                 \-
globaloff             |nimp|                                 \-                 \-
fineoff               |nimp|                                 \-                 \-
binsize               |nimp|                                 \-                 \-
angdir                |nimp|                                 \-                 \-
moveflag              |nimp|                                 \-                 \-
samplex               |nimp|                                 \-                 \-
sampley               |nimp|                                 \-                 \-
threaded              :py:attr:`Detector.threaded`           OK                 \-
darkimage             |nimp|                                 \-                 \-
gainimage             |nimp|                                 \-                 \-
settingsdir           :py:attr:`Detector.settings_path`      OK                 \-
trimdir               |nimp|                                 \-                 \-
caldir                |nimp|                                 \-                 \-
trimen                :py:attr:`Detector.trimmed_energies`   OK                 \-
settings              :py:attr:`Detector.settings`           OK                 \-
threshold             :py:attr:`Detector.threshold`          OK                 \-
thresholdnotb         |nimp|                                 \-                 \-
trimbits              :py:func:`Detector.load_trimbits`      OK                 \-
trim                  |nimp|                                 \-                 \-
trimval               :py:attr:`Detector.trimbits`           OK                 OK
pedestal              |nimp|                                 \-                 \-
vthreshold            :py:attr:`Detector.vthreshold`         OK                 \-
vcalibration          |nimp|                                 \-                 \-
vtrimbit              |nimp|                                 \-                 \-
vpreamp               |nimp|                                 \-                 \-
vshaper1              |nimp|                                 \-                 \-
vshaper2              |nimp|                                 \-                 \-
vhighvoltage          :py:attr:`Detector.high_voltage`       OK                 \-
vapower               |nimp|                                 \-                 \-
vddpower              |nimp|                                 \-                 \-
vshpower              |nimp|                                 \-                 \-
viopower              |nimp|                                 \-                 \-
vref_ds               :py:attr:`Jungfrau.dacs.vref_ds`       OK                 \-
vcascn_pb             |nimp|                                 \-                 \-
vcascp_pb             |nimp|                                 \-                 \-
vout_cm               |nimp|                                 \-                 \-
vcasc_out             |nimp|                                 \-                 \-
vin_cm                |nimp|                                 \-                 \-
vref_comp             |nimp|                                 \-                 \-
ib_test_c             |nimp|                                 \-                 \-
dac0                  |nimp|                                 \-                 \-
dac1                  |nimp|                                 \-                 \-
dac2                  |nimp|                                 \-                 \-
dac3                  |nimp|                                 \-                 \-
dac4                  |nimp|                                 \-                 \-
dac5                  |nimp|                                 \-                 \-
dac6                  |nimp|                                 \-                 \-
dac7                  |nimp|                                 \-                 \-
vsvp                  :py:attr:`Eiger.dacs.vsvp`             OK                 \-
vsvn                  :py:attr:`Eiger.dacs.vsvn`             OK                 \- 
vtr                   :py:attr:`Eiger.dacs.vtr`              OK                 \-
vrf                   :py:attr:`Eiger.dacs.vrf`              OK                 \-
vrs                   :py:attr:`Eiger.dacs.vrs`              OK                 \-
vtgstv                :py:attr:`Eiger.dacs.vtgstv`           OK                 \-
vcmp_ll               :py:attr:`Eiger.dacs.vcmp_ll`          OK                 \-
vcmp_ll               :py:attr:`Eiger.dacs.vcmp_ll`          OK                 \-
vcall                 :py:attr:`Eiger.dacs.vcall`            OK                 \-
vcmp_rl               :py:attr:`Eiger.dacs.vcmp_rl`          OK                 \-
vcmp_rr               :py:attr:`Eiger.dacs.vcmp_rr`          OK                 \-
rxb_rb                :py:attr:`Eiger.dacs.rxb_rb`           OK                 \-
rxb_lb                :py:attr:`Eiger.dacs.rxb_lb`           OK                 \-
vcp                   :py:attr:`Eiger.dacs.vcp`              OK                 \-
vcn                   :py:attr:`Eiger.dacs.vcn`              OK                 \-
vis                   :py:attr:`Eiger.dacs.vis`              OK                 \-
iodelay               :py:attr:`Eiger.dacs.iodelay`          OK                 \-
dac                   |nimp|                                 \-                 \-
adcvpp                |nimp|                                 \-                 \-
v_a                   |nimp|                                 \-                 \-
v_b                   |nimp|                                 \-                 \- 
v_c                   |nimp|                                 \-                 \-  
v_d                   |nimp|                                 \-                 \- 
v_io                  |nimp|                                 \-                 \- 
v_chip                |nimp|                                 \-                 \- 
v_limit               |nimp|                                 \-                 \- 
vIpre                 |nimp|                                 \-                 \- 
VcdSh                 |nimp|                                 \-                 \- 
Vth1                  |nimp|                                 \-                 \- 
Vth2                  |nimp|                                 \-                 \- 
Vth3                  |nimp|                                 \-                 \- 
VPL                   |nimp|                                 \-                 \- 
Vtrim                 |nimp|                                 \-                 \- 
vIbias                |nimp|                                 \-                 \- 
vIinSh                |nimp|                                 \-                 \- 
cas                   |nimp|                                 \-                 \- 
casSh                 |nimp|                                 \-                 \- 
vIbiasSh              |nimp|                                 \-                 \- 
vIcin                 |nimp|                                 \-                 \- 
vIpreOut              |nimp|                                 \-                 \- 
temp_adc              |nimp|                                 \-                 \- 
temp_fpga             :py:attr:`Detector.temp`.fpga          OK                 \- 
temp_fpgaext          |temp_fpgaext|                         OK                 \- 
temp_10ge             :py:attr:`Detector.temp`.t10ge         OK                 \- 
temp_dcdc             :py:attr:`Detector.temp`.dcdc          OK                 \- 
temp_sodl             :py:attr:`Detector.temp`.sodl          OK                 \- 
temp_sodr             :py:attr:`Detector.temp`.sodr          OK                 \- 
adc                   |nimp|                                 \-                 \- 
temp_fpgafl           :py:attr:`Detector.temp`.fpgafl        OK                 \- 
temp_fpgafr           :py:attr:`Detector.temp`.fpgafr        OK                 \- 
i_a                   |nimp|                                 \-                 \-        
i_b                   |nimp|                                 \-                 \-        
i_c                   |nimp|                                 \-                 \-        
i_d                   |nimp|                                 \-                 \-        
i_io                  |nimp|                                 \-                 \-        
vm_a                  |nimp|                                 \-                 \-        
vm_b                  |nimp|                                 \-                 \-        
vm_c                  |nimp|                                 \-                 \-        
vm_d                  |nimp|                                 \-                 \-        
vm_io                 |nimp|                                 \-                 \-        
temp_threshold        |tempth|                               \-                 \-                        
temp_control          |tempco|                               \-                 \-  
temp_event            |tempev|                               \-                 \-  
outdir                :py:attr:`Detector.file_path`          OK                 OK
fname                 :py:attr:`Detector.file_name`          OK                 OK
index                 :py:attr:`Detector.file_index`         OK                 OK
enablefwrite          :py:attr:`Detector.file_write`         OK                 OK
overwrite             :py:attr:`Detector.file_overwrite`     OK                 \-
currentfname          |nimp|                                 \-                 \-      
fileformat            :py:attr:`Detector.file_format`        OK                 \-
positions             |depr|                                 \-                 \-
startscript           |depr|                                 \-                 \-
startscriptpar        |depr|                                 \-                 \-
stopscript            |depr|                                 \-                 \-
stopscriptpar         |depr|                                 \-                 \-
scriptbefore          |depr|                                 \-                 \-
scriptbeforepar       |depr|                                 \-                 \-
scriptafter           |depr|                                 \-                 \-
scriptafterpar        |depr|                                 \-                 \-
headerafter           |depr|                                 \-                 \-
headerbefore          |depr|                                 \-                 \-
headerbeforepar       |depr|                                 \-                 \-
headerafterpar        |depr|                                 \-                 \-
encallog              |depr|                                 \-                 \-
angcallog             |depr|                                 \-                 \-
scan0script           |depr|                                 \-                 \-
scan0par              |depr|                                 \-                 \-
scan0prec             |depr|                                 \-                 \-
scan0steps            |depr|                                 \-                 \-
scan0range            |depr|                                 \-                 \-
scan1script           |depr|                                 \-                 \-
scan1par              |depr|                                 \-                 \-
scan1prec             |depr|                                 \-                 \-
scan1steps            |depr|                                 \-                 \-
scan1range            |depr|                                 \-                 \-
rx_hostname           :py:attr:`Detector.rx_hostname`        OK                 \-
rx_udpip              :py:attr:`Detector.rx_udpip`           OK                 \-
rx_udpmac             :py:attr:`Detector.rx_udpmac`          OK                 \-
rx_udpport            :py:attr:`Detector.rx_udpport`         OK                 \-
rx_udpport2           :py:attr:`Detector.rx_udpport`         OK                 \-
rx_udpsocksize        :py:attr:`Detector.rx_udpsocksize`     OK                 \-
rx_realudpsocksize    |rudp|                                 OK
detectormac           :py:attr:`Detector.detector_mac`       OK                 \-
detectorip            :py:attr:`Detector.detector_ip`        OK                 \-
txndelay_left         :py:attr:`Eiger.delay`.left            OK                 \-
txndelay_right        :py:attr:`Eiger.delay`.right           OK                 \-
txndelay_frame        :py:attr:`Eiger.delay`.frame           OK                 \-
flowcontrol_10g       :py:attr:`Eiger.flowcontrol_10g`       OK                 \-
zmqport               :py:attr:`Detector.client_zmqport`     Read               \- 
rx_zmqport            :py:attr:`Detector.rx_zmqport`         Read               \-
rx_datastream         :py:attr:`Detector.rx_datastream`      OK                 \-
zmqip                 :py:attr:`Detector.client_zmqip`       OK                 \- 
rx_zmqip              :py:attr:`Detector.rx_zmqip`           Read               \-
rx_jsonaddheader      :py:attr:`Detector.rx_jsonaddheader`   OK                 \- 
configuremac          :py:attr:`Detector.config_network`     OK                 \-
rx_tcpport            :py:attr:`Detector.rx_tcpport`    
port                  |nimp|                                 \-                 \- 
stopport              |nimp|                                 \-                 \- 
lock                  :py:attr:`Detector.lock`               OK                 \-
lastclient            :py:attr:`Detector.last_client_ip`     OK                 \-
receiver start        :py:func:`Detector.start_receiver`     OK                 \-   
receiver stop         :py:func:`Detector.stop_receiver`      \-                 \-  
r_online              |ron|                                  OK                 \-
r_checkonline         |nimp|                                 \-                 \-   
framescaught          :py:attr:`Detector.frames_caught`      OK                 \-
resetframescaught     |rfc|                                  OK                 \-
frameindex            |rfi|                                  OK                 \-
r_lock                :py:attr:`Detector.lock_receiver`      OK                 \-
r_lastclient          |rlci|                                 OK                 \-
r_readfreq            |nimp|                                 \-                 \- 
rx_fifodepth          :py:attr:`Detector.rx_fifodepth`       OK                 \-
r_silent              |nimp|                                 \-                 \- 
r_framesperfile       :py:attr:`Detector.n_frames_per_file`  OK                 \-
r_discardpolicy       |fdp|                                  OK                 \-
r_padding             :py:attr:`Detector.file_padding`       OK                 \-
adcinvert             |chiptest|                             \-                 \-
adcdisable            |chiptest|                             \-                 \-
pattern               |chiptest|                             \-                 \-
patword               |chiptest|                             \-                 \-
patioctrl             |chiptest|                             \-                 \-
patclkctrl            |chiptest|                             \-                 \-
patlimits             |chiptest|                             \-                 \-
patloop0              |chiptest|                             \-                 \-
patnloop0             |chiptest|                             \-                 \-
patwait0              |chiptest|                             \-                 \-
patwaittime0          |chiptest|                             \-                 \-
patloop1              |chiptest|                             \-                 \-
patnloop1             |chiptest|                             \-                 \-
patwait1              |chiptest|                             \-                 \-
patwaittime1          |chiptest|                             \-                 \-
patloop2              |chiptest|                             \-                 \-
patnloop2             |chiptest|                             \-                 \-
patwait2              |chiptest|                             \-                 \-
patwaittime2          |chiptest|                             \-                 \-
dut_clk               |chiptest|                             \-                 \-
===================== ===================================== ================== =========   