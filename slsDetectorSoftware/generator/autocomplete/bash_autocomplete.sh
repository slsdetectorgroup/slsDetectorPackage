# GENERATED FILE - DO NOT EDIT
# ANY CHANGES TO THIS FILE WILL BE OVERWRITTEN

_sd() {

  # Taken from https://github.com/scop/bash-completion/blob/15b74b1050333f425877a7cbd99af2998b95c476/bash_completion#L770C12-L770C12
  # Reassemble command line words, excluding specified characters from the
  # list of word completion separators (COMP_WORDBREAKS).
  # @param $1 chars  Characters out of $COMP_WORDBREAKS which should
  #     NOT be considered word breaks. This is useful for things like scp where
  #     we want to return host:path and not only path, so we would pass the
  #     colon (:) as $1 here.
  # @param $2 words  Name of variable to return words to
  # @param $3 cword  Name of variable to return cword to
  #
  _comp__reassemble_words()
{
    local exclude="" i j line ref
    # Exclude word separator characters?
    if [[ $1 ]]; then
        # Yes, exclude word separator characters;
        # Exclude only those characters, which were really included
        exclude="[${1//[^$COMP_WORDBREAKS]/}]"
    fi

    # Default to cword unchanged
    printf -v "$3" %s "$COMP_CWORD"
    # Are characters excluded which were former included?
    if [[ $exclude ]]; then
        # Yes, list of word completion separators has shrunk;
        line=$COMP_LINE
        # Re-assemble words to complete
        for ((i = 0, j = 0; i < ${#COMP_WORDS[@]}; i++, j++)); do
            # Is current word not word 0 (the command itself) and is word not
            # empty and is word made up of just word separator characters to
            # be excluded and is current word not preceded by whitespace in
            # original line?
            while [[ $i -gt 0 && ${COMP_WORDS[i]} == +($exclude) ]]; do
                # Is word separator not preceded by whitespace in original line
                # and are we not going to append to word 0 (the command
                # itself), then append to current word.
                [[ $line != [[:blank:]]* ]] && ((j >= 2)) && ((j--))
                # Append word separator to current or new word
                ref="$2[$j]"
                printf -v "$ref" %s "${!ref-}${COMP_WORDS[i]}"
                # Indicate new cword
                ((i == COMP_CWORD)) && printf -v "$3" %s "$j"
                # Remove optional whitespace + word separator from line copy
                line=${line#*"${COMP_WORDS[i]}"}
                # Indicate next word if available, else end *both* while and
                # for loop
                if ((i < ${#COMP_WORDS[@]} - 1)); then
                    ((i++))
                else
                    break 2
                fi
                # Start new word if word separator in original line is
                # followed by whitespace.
                [[ $line == [[:blank:]]* ]] && ((j++))
            done
            # Append word to current word
            ref="$2[$j]"
            printf -v "$ref" %s "${!ref-}${COMP_WORDS[i]}"
            # Remove optional whitespace + word from line copy
            line=${line#*"${COMP_WORDS[i]}"}
            # Indicate new cword
            ((i == COMP_CWORD)) && printf -v "$3" %s "$j"
        done
        ((i == COMP_CWORD)) && printf -v "$3" %s "$j"
    else
        # No, list of word completions separators hasn't changed;
        for i in "${!COMP_WORDS[@]}"; do
            printf -v "$2[i]" %s "${COMP_WORDS[i]}"
        done
    fi
}


  local FCN_RETURN=""
  local IS_PATH=0


local SLS_COMMANDS=" acquire activate adcclk adcenable adcenable10g adcindex adcinvert adclist adcname adcphase adcpipeline adcreg adcvpp apulse asamples autocompdisable badchannels blockingtrigger burstmode burstperiod bursts burstsl bustest cdsgain chipversion clearbit clearbusy clearroi clientversion clkdiv clkfreq clkphase collectionmode column compdisabletime confadc config configtransceiver counters currentsource dac dacindex daclist dacname dacvalues datastream dbitclk dbitphase dbitpipeline defaultdac defaultpattern delay delayl detectorserverversion detsize diodelay dpulse dr drlist dsamples execcommand exptime exptime1 exptime2 exptime3 exptimel extrastoragecells extsampling extsamplingsrc extsig fformat filtercells filterresistor findex firmwaretest firmwareversion fliprows flowcontrol10g fmaster fname foverwrite fpath framecounter frames framesl frametime free fwrite gaincaps gainmode gappixels gatedelay gatedelay1 gatedelay2 gatedelay3 gates getbit hardwareversion highvoltage hostname im_a im_b im_c im_d im_io imagetest initialchecks inj_ch interpolation interruptsubframe kernelversion lastclient led lock master maxadcphaseshift maxclkphaseshift maxdbitphaseshift measuredperiod measuredsubperiod moduleid nextframenumber nmod numinterfaces overflow packageversion parallel parameters partialreset patfname patioctrl patlimits patloop patloop0 patloop1 patloop2 patmask patnloop patnloop0 patnloop1 patnloop2 patsetbit patternX patternstart patwait patwait0 patwait1 patwait2 patwaittime patwaittime0 patwaittime1 patwaittime2 patword pedestalmode period periodl polarity port powerchip powerindex powerlist powername powervalues programfpga pulse pulsechip pulsenmove pumpprobe quad ratecorr readnrows readout readoutspeed readoutspeedlist rebootcontroller reg resetdacs resetfpga roi romode row runclk runtime rx_arping rx_clearroi rx_dbitlist rx_dbitoffset rx_discardpolicy rx_fifodepth rx_frameindex rx_framescaught rx_framesperfile rx_hostname rx_jsonaddheader rx_jsonpara rx_lastclient rx_lock rx_missingpackets rx_padding rx_printconfig rx_realudpsocksize rx_roi rx_silent rx_start rx_status rx_stop rx_tcpport rx_threads rx_udpsocksize rx_version rx_zmqfreq rx_zmqhwm rx_zmqip rx_zmqport rx_zmqstartfnum rx_zmqstream samples savepattern scan scanerrmsg selinterface serialnumber setbit settings settingslist settingspath signalindex signallist signalname sleep slowadc slowadcindex slowadclist slowadcname slowadcvalues start status stop stopport storagecell_delay storagecell_start subdeadtime subexptime sync syncclk temp_10ge temp_adc temp_control temp_dcdc temp_event temp_fpga temp_fpgaext temp_fpgafl temp_fpgafr temp_slowadc temp_sodl temp_sodr temp_threshold templist tempvalues tengiga threshold thresholdnotb timing timing_info_decoder timinglist timingsource top transceiverenable trigger triggers triggersl trimbits trimen trimval tsamples txdelay txdelay_frame txdelay_left txdelay_right type udp_cleardst udp_dstip udp_dstip2 udp_dstlist udp_dstmac udp_dstmac2 udp_dstport udp_dstport2 udp_firstdst udp_numdst udp_reconfigure udp_srcip udp_srcip2 udp_srcmac udp_srcmac2 udp_validate update updatedetectorserver updatekernel updatemode user v_a v_b v_c v_chip v_d v_io v_limit vchip_comp_adc vchip_comp_fe vchip_cs vchip_opa_1st vchip_opa_fd vchip_ref_comp_fe versions veto vetoalg vetofile vetophoton vetoref vetostream virtual vm_a vm_b vm_c vm_d vm_io zmqhwm zmqip zmqport "
__acquire() {
FCN_RETURN=""
return 0
}
__activate() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__adcclk() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__adcenable() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__adcenable10g() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__adcindex() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__adcinvert() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__adclist() {
FCN_RETURN=""
return 0
}
__adcname() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__adcphase() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="deg"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="deg"
fi
fi
return 0
}
__adcpipeline() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__adcreg() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__adcvpp() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="mV mv"
fi
fi
return 0
}
__apulse() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__asamples() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__autocompdisable() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__badchannels() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__blockingtrigger() {
FCN_RETURN=""
return 0
}
__burstmode() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="burst_external burst_internal cw_external cw_internal"
fi
fi
return 0
}
__burstperiod() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__bursts() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__burstsl() {
FCN_RETURN=""
return 0
}
__bustest() {
FCN_RETURN=""
return 0
}
__cdsgain() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__chipversion() {
FCN_RETURN=""
return 0
}
__clearbit() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN="--validate"
fi
fi
return 0
}
__clearbusy() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN="--validate"
fi
fi
return 0
}
__clearroi() {
FCN_RETURN=""
return 0
}
__clientversion() {
FCN_RETURN=""
return 0
}
__clkdiv() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__clkfreq() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__clkphase() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="deg"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN="deg"
fi
fi
return 0
}
__collectionmode() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="electron hole"
fi
fi
return 0
}
__column() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__compdisabletime() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__confadc() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__config() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
fi
return 0
}
__configtransceiver() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
fi
return 0
}
__counters() {
FCN_RETURN=""
return 0
}
__currentsource() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="fix nofix"
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "5" ]]; then
FCN_RETURN="low normal"
fi
fi
return 0
}
__dac() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="`sls_detector_get daclist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="mV mv"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="`sls_detector_get daclist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN="mV mv"
fi
fi
return 0
}
__dacindex() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__daclist() {
FCN_RETURN=""
return 0
}
__dacname() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="`sls_detector_get daclist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="`sls_detector_get daclist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__dacvalues() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="mV mv"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="mV mv"
fi
fi
return 0
}
__datastream() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="bottom left right top"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="bottom left right top"
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__dbitclk() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__dbitphase() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="deg"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="deg"
fi
fi
return 0
}
__dbitpipeline() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__defaultdac() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="`sls_detector_get daclist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="`sls_detector_get settingslist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="`sls_detector_get daclist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN="`sls_detector_get settingslist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
fi
return 0
}
__defaultpattern() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="`sls_detector_get daclist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN="`sls_detector_get settingslist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
fi
return 0
}
__delay() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__delayl() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__detectorserverversion() {
FCN_RETURN=""
return 0
}
__detsize() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__diodelay() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__dpulse() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__dr() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__drlist() {
FCN_RETURN=""
return 0
}
__dsamples() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__execcommand() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__exptime() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__exptime1() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__exptime2() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__exptime3() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__exptimel() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__extrastoragecells() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__extsampling() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__extsamplingsrc() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__extsig() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="inversion_off inversion_on trigger_in_falling_edge trigger_in_rising_edge"
fi
fi
return 0
}
__fformat() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="binary hdf5"
fi
fi
return 0
}
__filtercells() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__filterresistor() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__findex() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__firmwaretest() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__firmwareversion() {
FCN_RETURN=""
return 0
}
__fliprows() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__flowcontrol10g() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__fmaster() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__fname() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__foverwrite() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__fpath() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
fi
return 0
}
__framecounter() {
FCN_RETURN=""
return 0
}
__frames() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__framesl() {
FCN_RETURN=""
return 0
}
__frametime() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__free() {
FCN_RETURN=""
return 0
}
__fwrite() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__gaincaps() {
FCN_RETURN=""
return 0
}
__gainmode() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="dynamic fixg0 fixg1 fixg2 forceswitchg1 forceswitchg2"
fi
fi
return 0
}
__gappixels() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__gatedelay() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__gatedelay1() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__gatedelay2() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__gatedelay3() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__gates() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__getbit() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__hardwareversion() {
FCN_RETURN=""
return 0
}
__highvoltage() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__hostname() {
FCN_RETURN=""
return 0
}
__im_a() {
FCN_RETURN=""
return 0
}
__im_b() {
FCN_RETURN=""
return 0
}
__im_c() {
FCN_RETURN=""
return 0
}
__im_d() {
FCN_RETURN=""
return 0
}
__im_io() {
FCN_RETURN=""
return 0
}
__imagetest() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__initialchecks() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__inj_ch() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__interpolation() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__interruptsubframe() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__kernelversion() {
FCN_RETURN=""
return 0
}
__lastclient() {
FCN_RETURN=""
return 0
}
__led() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__lock() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__master() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__maxadcphaseshift() {
FCN_RETURN=""
return 0
}
__maxclkphaseshift() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__maxdbitphaseshift() {
FCN_RETURN=""
return 0
}
__measuredperiod() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__measuredsubperiod() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__moduleid() {
FCN_RETURN=""
return 0
}
__nextframenumber() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__nmod() {
FCN_RETURN=""
return 0
}
__numinterfaces() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__overflow() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__packageversion() {
FCN_RETURN=""
return 0
}
__parallel() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__parameters() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
fi
return 0
}
__partialreset() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__patfname() {
FCN_RETURN=""
return 0
}
__patioctrl() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__patlimits() {
FCN_RETURN=""
return 0
}
__patloop() {
FCN_RETURN=""
return 0
}
__patloop0() {
FCN_RETURN=""
return 0
}
__patloop1() {
FCN_RETURN=""
return 0
}
__patloop2() {
FCN_RETURN=""
return 0
}
__patmask() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__patnloop() {
FCN_RETURN=""
return 0
}
__patnloop0() {
FCN_RETURN=""
return 0
}
__patnloop1() {
FCN_RETURN=""
return 0
}
__patnloop2() {
FCN_RETURN=""
return 0
}
__patsetbit() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__patternX() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
fi
return 0
}
__patternstart() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
fi
return 0
}
__patwait() {
FCN_RETURN=""
return 0
}
__patwait0() {
FCN_RETURN=""
return 0
}
__patwait1() {
FCN_RETURN=""
return 0
}
__patwait2() {
FCN_RETURN=""
return 0
}
__patwaittime() {
FCN_RETURN=""
return 0
}
__patwaittime0() {
FCN_RETURN=""
return 0
}
__patwaittime1() {
FCN_RETURN=""
return 0
}
__patwaittime2() {
FCN_RETURN=""
return 0
}
__patword() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__pedestalmode() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=" 0"
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__period() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__periodl() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__polarity() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="neg pos"
fi
fi
return 0
}
__port() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__powerchip() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__powerindex() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__powerlist() {
FCN_RETURN=""
return 0
}
__powername() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="`sls_detector_get daclist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="`sls_detector_get daclist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__powervalues() {
FCN_RETURN=""
return 0
}
__programfpga() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="--force-delete-normal-file"
fi
fi
return 0
}
__pulse() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="--force-delete-normal-file"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__pulsechip() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__pulsenmove() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__pumpprobe() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__quad() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__ratecorr() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__readnrows() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__readout() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__readoutspeed() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1 108 144 2 full_speed half_speed quarter_speed"
fi
fi
return 0
}
__readoutspeedlist() {
FCN_RETURN=""
return 0
}
__rebootcontroller() {
FCN_RETURN=""
return 0
}
__reg() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN="--validate"
fi
fi
return 0
}
__resetdacs() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN="--validate"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="hard"
fi
fi
return 0
}
__resetfpga() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="hard"
fi
fi
return 0
}
__roi() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__romode() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="analog analog_digital digital digital_transceiver transceiver"
fi
fi
return 0
}
__row() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__runclk() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__runtime() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__rx_arping() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__rx_clearroi() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__rx_dbitlist() {
FCN_RETURN=""
return 0
}
__rx_dbitoffset() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__rx_discardpolicy() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="discardempty discardpartial nodiscard"
fi
fi
return 0
}
__rx_fifodepth() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__rx_frameindex() {
FCN_RETURN=""
return 0
}
__rx_framescaught() {
FCN_RETURN=""
return 0
}
__rx_framesperfile() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__rx_hostname() {
FCN_RETURN=""
return 0
}
__rx_jsonaddheader() {
FCN_RETURN=""
return 0
}
__rx_jsonpara() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__rx_lastclient() {
FCN_RETURN=""
return 0
}
__rx_lock() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__rx_missingpackets() {
FCN_RETURN=""
return 0
}
__rx_padding() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__rx_printconfig() {
FCN_RETURN=""
return 0
}
__rx_realudpsocksize() {
FCN_RETURN=""
return 0
}
__rx_roi() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "5" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__rx_silent() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__rx_start() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__rx_status() {
FCN_RETURN=""
return 0
}
__rx_stop() {
FCN_RETURN=""
return 0
}
__rx_tcpport() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__rx_threads() {
FCN_RETURN=""
return 0
}
__rx_udpsocksize() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__rx_version() {
FCN_RETURN=""
return 0
}
__rx_zmqfreq() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__rx_zmqhwm() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__rx_zmqip() {
FCN_RETURN=""
return 0
}
__rx_zmqport() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__rx_zmqstartfnum() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__rx_zmqstream() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__samples() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__savepattern() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
fi
return 0
}
__scan() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="`sls_detector_get daclist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "5" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "6" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__scanerrmsg() {
FCN_RETURN=""
return 0
}
__selinterface() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__serialnumber() {
FCN_RETURN=""
return 0
}
__setbit() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN="--validate"
fi
fi
return 0
}
__settings() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="`sls_detector_get settingslist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
fi
return 0
}
__settingslist() {
FCN_RETURN=""
return 0
}
__settingspath() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
fi
return 0
}
__signalindex() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__signallist() {
FCN_RETURN=""
return 0
}
__signalname() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__sleep() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__slowadc() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__slowadcindex() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__slowadclist() {
FCN_RETURN=""
return 0
}
__slowadcname() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="`sls_detector_get daclist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="`sls_detector_get daclist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__slowadcvalues() {
FCN_RETURN=""
return 0
}
__start() {
FCN_RETURN=""
return 0
}
__status() {
FCN_RETURN=""
return 0
}
__stop() {
FCN_RETURN=""
return 0
}
__stopport() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__storagecell_delay() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__storagecell_start() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__subdeadtime() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__subexptime() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="ms ns s us"
fi
fi
return 0
}
__sync() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__syncclk() {
FCN_RETURN=""
return 0
}
__temp_10ge() {
FCN_RETURN=""
return 0
}
__temp_adc() {
FCN_RETURN=""
return 0
}
__temp_control() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__temp_dcdc() {
FCN_RETURN=""
return 0
}
__temp_event() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__temp_fpga() {
FCN_RETURN=""
return 0
}
__temp_fpgaext() {
FCN_RETURN=""
return 0
}
__temp_fpgafl() {
FCN_RETURN=""
return 0
}
__temp_fpgafr() {
FCN_RETURN=""
return 0
}
__temp_slowadc() {
FCN_RETURN=""
return 0
}
__temp_sodl() {
FCN_RETURN=""
return 0
}
__temp_sodr() {
FCN_RETURN=""
return 0
}
__temp_threshold() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__templist() {
FCN_RETURN=""
return 0
}
__tempvalues() {
FCN_RETURN=""
return 0
}
__tengiga() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__threshold() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="`sls_detector_get settingslist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "5" ]]; then
FCN_RETURN="`sls_detector_get settingslist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
fi
return 0
}
__thresholdnotb() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="`sls_detector_get settingslist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "5" ]]; then
FCN_RETURN="`sls_detector_get settingslist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
fi
return 0
}
__timing() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="`sls_detector_get timinglist | sed -e 's/.*\[\(.*\)\].*/\1/' | sed 's/,//g'`"
fi
fi
return 0
}
__timing_info_decoder() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="shine swissfel"
fi
fi
return 0
}
__timinglist() {
FCN_RETURN=""
return 0
}
__timingsource() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="external internal"
fi
fi
return 0
}
__top() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__transceiverenable() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__trigger() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__triggers() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__triggersl() {
FCN_RETURN=""
return 0
}
__trimbits() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
fi
return 0
}
__trimen() {
FCN_RETURN=""
return 0
}
__trimval() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__tsamples() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__txdelay() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__txdelay_frame() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__txdelay_left() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__txdelay_right() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__type() {
FCN_RETURN=""
return 0
}
__udp_cleardst() {
FCN_RETURN=""
return 0
}
__udp_dstip() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__udp_dstip2() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__udp_dstlist() {
FCN_RETURN=""
return 0
}
__udp_dstmac() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__udp_dstmac2() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__udp_dstport() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__udp_dstport2() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__udp_firstdst() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__udp_numdst() {
FCN_RETURN=""
return 0
}
__udp_reconfigure() {
FCN_RETURN=""
return 0
}
__udp_srcip() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__udp_srcip2() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__udp_srcmac() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__udp_srcmac2() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__udp_validate() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__update() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
fi
return 0
}
__updatedetectorserver() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
fi
return 0
}
__updatekernel() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
IS_PATH=1
fi
fi
return 0
}
__updatemode() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__user() {
FCN_RETURN=""
return 0
}
__v_a() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__v_b() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__v_c() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__v_chip() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__v_d() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__v_io() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__v_limit() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__vchip_comp_adc() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__vchip_comp_fe() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__vchip_cs() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__vchip_opa_1st() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__vchip_opa_fd() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__vchip_ref_comp_fe() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__versions() {
FCN_RETURN=""
return 0
}
__veto() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="0 1"
fi
fi
return 0
}
__vetoalg() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="10gbe lll none"
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN="hits raw"
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN="10gbe lll none"
fi
fi
return 0
}
__vetofile() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__vetophoton() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 1 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "4" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "5" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__vetoref() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__vetostream() {
FCN_RETURN=""
return 0
}
__virtual() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
if [[ "${cword}" == "3" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__vm_a() {
FCN_RETURN=""
return 0
}
__vm_b() {
FCN_RETURN=""
return 0
}
__vm_c() {
FCN_RETURN=""
return 0
}
__vm_d() {
FCN_RETURN=""
return 0
}
__vm_io() {
FCN_RETURN=""
return 0
}
__zmqhwm() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__zmqip() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}
__zmqport() {
FCN_RETURN=""
if [[ ${IS_GET} -eq 0 ]]; then
if [[ "${cword}" == "2" ]]; then
FCN_RETURN=""
fi
fi
return 0
}


  COMPREPLY=()
  local OPTIONS_NEW=""

  # check if bash or zsh
  # _get_comp_words_by_ref is a bash built-in function, we check if it exists
  declare -Ff _get_comp_words_by_ref > /dev/null && IS_BASH=1 || IS_BASH=0


  # bash interprets the colon character : as a special character and splits the argument in two
  # different than what zsh does
  # https://stackoverflow.com/a/3224910
  # https://stackoverflow.com/a/12495727
  local cur
  local cword words=()
  _comp__reassemble_words ":" words cword
  cur=${words[cword]}

  #  check the action (get or put)
  case "${words[0]}" in
    "sls_detector_get" | "g" | "detg")
      local IS_GET=1
      ;;
    *)
      local IS_GET=0
      ;;
  esac

  # if no command is written, autocomplete with the commands
  if [[ ${cword} -eq 1 ]]; then
#    local SLS_COMMANDS="trimbits exptime"
    local SLS_COMMANDS_NEW=""

    case "$cur" in
    	[0-9]*:*)
				local suggestions=($(compgen -W "${SLS_COMMANDS}" -- "${cur#*:}"))
				COMPREPLY=( ${suggestions[*]} )
        ;;
      [0-9]*)
        COMPREPLY=()
        ;;
      *)
				COMPREPLY=( $( compgen -W "$SLS_COMMANDS -h" -- "$cur" ) );;

		esac
    return 0
  fi

  if [[ ${cword} -eq 2 ]] && [[ ${words[1]} == "-h" ]]; then
    COMPREPLY=( $( compgen -W "$SLS_COMMANDS" -- "$cur" ) )
    return 0
  fi

  # if a command is written, autocomplete with the options
  # call the function for the command

  if [[ "$SLS_COMMANDS" == *"${words[1]##*:}"* ]]; then
      __"${words[1]##*:}"
  fi

  # if IS_PATH is activated, autocomplete with the path
  if [[ ${IS_PATH} -eq 1 ]]; then
    COMPREPLY=($(compgen -f -- "${cur}"))
    return 0
  fi

  # autocomplete with the options
  COMPREPLY=($(compgen -W "${FCN_RETURN}" -- "${cur}"))



}

complete -F _sd -o filenames sls_detector_get
complete -F _sd -o filenames g
complete -F _sd -o filenames detg

complete -F _sd -o filenames sls_detector_put
complete -F _sd -o filenames p
complete -F _sd -o filenames detp

complete -F _sd -o filenames sls_detector
complete -F _sd -o filenames det
