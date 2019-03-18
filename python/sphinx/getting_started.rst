Getting started
================


------------------------
Setting up the detector
------------------------
        
All configuration of the detector can either be done from the Python
API (including loading config file) or externally. The detector setup is
discovered from the shared memory when launching a new script. Because the
detector usually should remain online longer than a specific script it is
recommended to run the receivers seperate.
        
---------------------------------
Setting and getting attributes
---------------------------------        

Most of the detector and software setting are implemented as attributes
in the Detector class. When something is assigned it is also set 
in the detector and when the attribute is called using dot notation it
it looked up from the detector.

::

    #Currently Eiger and Jungfrau but Detector should work for all
    from sls_detector import Eiger()
    d = Eiger()
    
    d.file_write = True
    d.vthreshold = 1500
    
    d.frame_index
    >> 12
    
    d.file_name
    >> 'run'
    
---------------------------------
Working with DACs
---------------------------------  

The following examples assumes an Eiger500k detector. But the same syntax
works for other detector sizes and models.

::

    d.dacs
    >>
    ========== DACS =========
    vsvp      :     0,     0
    vtr       :  4000,  4000
    vrf       :  2000,  2300
    vrs       :  1400,  1400
    vsvn      :  4000,  4000
    vtgstv    :  2556,  2556
    vcmp_ll   :  1500,  1500
    vcmp_lr   :  1500,  1500
    vcall     :  3500,  3600
    vcmp_rl   :  1500,  1500
    rxb_rb    :  1100,  1100
    rxb_lb    :  1100,  1100
    vcmp_rr   :  1500,  1500
    vcp       :  1500,  1500
    vcn       :  2000,  2000
    vis       :  1550,  1550
    iodelay   :   660,   660
    
    #Read dac values to a variable
    vrf = d.dacs.vrf[:]
    
    #Set a dac in a module
    d.dacs.vrf[0] = 1500
    d.dacs.vrf[0]
    >> 1500
    
    #Set vrf to the same value in all moduels
    d.dacs.vrf = 1500
    
    #Set a dac using an iterable
    d.dacs.vrf = [1500, 1600]
    d.dacs.vrf
    >> vrf       :  1500,  1600
    
    #Set dacs iterating on index and values
    d.dacs.vrf[0,1] = 1300,1400


---------------------------------
Operating multiple detectors
---------------------------------

Operating multiple detectors is supported by assigning an id when creating the object. If no id is
set it defaults to 0.

::

    d0 = Eiger() #id is now 0
    d1 = Jungfrau(1)

    #Or explicitly
    d1 = Jungfrau(id = 0)

The detectors now operate independently of each other but can be synchronized using a hardware trigger.

::

    from sls_detector import Eiger

    d0 = Eiger(0)
    d1 = Eiger(1)

    d0.load_config('/some/path/T45.config')
    d1.load_config('/some/path/T62.config')

    d0.n_frames = 1
    d0.exposure_time = 1
    d0.timing_mode = 'trigger'

    d1.n_frames = 5
    d1.exposure_time = 0.2
    d1.timing_mode = 'trigger'