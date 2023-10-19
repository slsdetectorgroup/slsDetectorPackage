# GENERATED FILE - DO NOT EDIT
# ANY CHANGES TO THIS FILE WILL BE OVERWRITTEN

_sd() {
  local FCN_RETURN=""
  local IS_PATH=0


local SLS_COMMANDS="acquire activate adcclk adcenable adcenable10g adcindex adcinvert adclist adcname adcphase adcpipeline adcreg adcvpp apulse asamples autocompdisable badchannels blockingtrigger burstmode burstperiod bursts burstsl bustest cdsgain chipversion clearbit clearbusy clearroi clientversion clkdiv clkfreq clkphase column compdisabletime confadc config counters currentsource dac dacindex daclist dacname dacvalues datastream dbitclk dbitphase dbitpipeline defaultdac defaultpattern delay delayl detectorserverversion detsize diodelay dpulse dr drlist dsamples execcommand exptime exptime1 exptime2 exptime3 exptimel extrastoragecells extsampling extsamplingsrc extsig fformat filtercells filterresistor findex firmwaretest firmwareversion fliprows flowcontrol10g fmaster fname foverwrite fpath framecounter frames framesl frametime fwrite gaincaps gainmode gappixels gatedelay gatedelay1 gatedelay2 gatedelay3 gates getbit hardwareversion highvoltage hostname im_a im_b im_c im_d im_io imagetest initialchecks inj_ch interpolation interruptsubframe kernelversion lastclient led lock master maxadcphaseshift maxclkphaseshift maxdbitphaseshift measuredperiod measuredsubperiod moduleid nextframenumber nmod numinterfaces overflow packageversion parallel parameters partialreset patfname patioctrl patlimits patloop patloop0 patloop1 patloop2 patmask patnloop patnloop0 patnloop1 patnloop2 patsetbit patternX patternstart patwait patwait0 patwait1 patwait2 patwaittime patwaittime0 patwaittime1 patwaittime2 patword period periodl polarity port powerchip programfpga pulse pulsechip pulsenmove pumpprobe quad ratecorr readnrows readout readoutspeed readoutspeedlist rebootcontroller reg resetdacs resetfpga roi romode row runclk runtime rx_arping rx_clearroi rx_dbitlist rx_dbitoffset rx_discardpolicy rx_fifodepth rx_frameindex rx_framescaught rx_framesperfile rx_hostname rx_jsonaddheader rx_jsonpara rx_lastclient rx_lock rx_missingpackets rx_padding rx_printconfig rx_realudpsocksize rx_roi rx_silent rx_start rx_status rx_stop rx_tcpport rx_threads rx_udpsocksize rx_version rx_zmqfreq rx_zmqhwm rx_zmqip rx_zmqport rx_zmqstartfnum rx_zmqstream samples savepattern scan scanerrmsg selinterface serialnumber setbit settings settingslist settingspath signalindex signallist signalname slowadc slowadcindex slowadclist slowadcname slowadcvalues start status stop stopport storagecell_delay storagecell_start subdeadtime subexptime sync syncclk temp_10ge temp_adc temp_control temp_dcdc temp_event temp_fpga temp_fpgaext temp_fpgafl temp_fpgafr temp_slowadc temp_sodl temp_sodr temp_threshold templist tempvalues tengiga threshold thresholdnotb timing timinglist timingsource top transceiverenable trigger triggers triggersl trimbits trimen trimval tsamples txdelay txdelay_frame txdelay_left txdelay_right type udp_cleardst udp_dstip udp_dstip2 udp_dstlist udp_dstmac udp_dstmac2 udp_dstport udp_dstport2 udp_firstdst udp_numdst udp_reconfigure udp_srcip udp_srcip2 udp_srcmac udp_srcmac2 udp_validate update updatedetectorserver updatekernel updatemode user v_a v_b v_c v_chip v_d v_io v_limit vchip_comp_adc vchip_comp_fe vchip_cs vchip_opa_1st vchip_opa_fd vchip_ref_comp_fe versions veto vetoalg vetofile vetophoton vetoref vetostream virtual vm_a vm_b vm_c vm_d vm_io voltageindex voltagelist voltagename voltagevalues zmqhwm zmqip zmqport"
__exptime() {
FCN_RETURN=""
if [[ IS_GET -eq 1 ]]; then
if [[ "${COMP_CWORD}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ IS_GET -eq 0 ]]; then
if [[ "${COMP_CWORD}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${COMP_CWORD}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}


  COMPREPLY=()
  local OPTIONS_NEW=""
  local cur=${COMP_WORDS[COMP_CWORD]}
  #  check the action (get or put)
  if [ "${COMP_WORDS[0]}" == "sls_detector_get" ]; then
    local IS_GET=1
  else
    local IS_GET=0
  fi

  # if no command is written, autocomplete with the commands
  if [[ ${COMP_CWORD} -eq 1 ]]; then

    case "$cur" in
			[0-9]*)
				for i in $SLS_COMMANDS; do
					SLS_COMMANDS_NEW="${SLS_COMMANDS_NEW} ${cur%%:*}:$i"
				done
				COMPREPLY=( $( compgen -W "${SLS_COMMANDS_NEW}" -- "$cur" ) );;
			*)
				COMPREPLY=( $( compgen -W "$SLS_COMMANDS -h" -- "$cur" ) );;
		esac
    return 0
  fi

  if [[ ${COMP_CWORD} -eq 2 ]] && [[ ${COMP_WORDS[1]} == "-h" ]]; then
    COMPREPLY=( $( compgen -W "$SLS_COMMANDS" -- "$cur" ) )
    return 0
  fi

  # if a command is written, autocomplete with the options
  # call the function for the command

  if [[ "$SLS_COMMANDS" == *"${COMP_WORDS[1]##*:}"* ]]; then
      __"${COMP_WORDS[1]##*:}"
  fi

  # if IS_PATH is activated, autocomplete with the path
  if [[ ${IS_PATH} -eq 1 ]]; then
    COMPREPLY=($(compgen -d -- "${cur}"))
    return 0
  fi

  # autocomplete with the options
  COMPREPLY=($(compgen -W "${FCN_RETURN}" -- "${cur}"))



}

complete -F _sd -o filenames sls_detector_get
complete -F _sd -o filenames sls_detector_put
