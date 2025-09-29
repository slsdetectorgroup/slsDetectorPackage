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


local SLS_COMMANDS=" acquire activate adcclk adcenable adcenable10g adcindex adcinvert adclist adcname adcphase adcpipeline adcreg adcvpp apulse asamples autocompdisable badchannels blockingtrigger burstmode burstperiod bursts burstsl bustest cdsgain chipversion clearbit clearbusy clientversion clkdiv clkfreq clkphase collectionmode column compdisabletime confadc config configtransceiver counters currentsource dac dacindex daclist dacname dacvalues datastream dbitclk dbitphase dbitpipeline defaultdac defaultpattern define delay delayl detectorserverversion detsize diodelay dpulse dr drlist dsamples execcommand exptime exptime1 exptime2 exptime3 extrastoragecells extsampling extsamplingsrc extsig fformat filtercells filterresistor findex firmwaretest firmwareversion fliprows flowcontrol10g fmaster fname foverwrite fpath framecounter frames framesl frametime free fwrite gaincaps gainmode gappixels gatedelay gatedelay1 gatedelay2 gatedelay3 gates getbit hardwareversion highvoltage hostname im_a im_b im_c im_d im_io imagetest initialchecks inj_ch interpolation interruptsubframe kernelversion lastclient led lock master maxadcphaseshift maxclkphaseshift maxdbitphaseshift measuredperiod measuredsubperiod moduleid nextframenumber nmod numinterfaces overflow packageversion parallel parameters partialreset patfname patioctrl patlimits patloop patloop0 patloop1 patloop2 patmask patnloop patnloop0 patnloop1 patnloop2 patsetbit pattern patternstart patwait patwait0 patwait1 patwait2 patwaittime patwaittime0 patwaittime1 patwaittime2 patword pedestalmode period periodl polarity port powerchip powerindex powerlist powername powervalues programfpga pulse pulsechip pulsenmove pumpprobe quad ratecorr readnrows readout readoutspeed readoutspeedlist rebootcontroller reg resetdacs resetfpga romode row runclk runtime rx_arping rx_clearroi rx_dbitlist rx_dbitoffset rx_dbitreorder rx_discardpolicy rx_fifodepth rx_frameindex rx_framescaught rx_framesperfile rx_hostname rx_jsonaddheader rx_jsonpara rx_lastclient rx_lock rx_missingpackets rx_padding rx_printconfig rx_realudpsocksize rx_roi rx_silent rx_start rx_status rx_stop rx_tcpport rx_threads rx_udpsocksize rx_version rx_zmqfreq rx_zmqhwm rx_zmqip rx_zmqport rx_zmqstartfnum rx_zmqstream samples savepattern scan scanerrmsg selinterface serialnumber setbit settings settingslist settingspath signalindex signallist signalname sleep slowadc slowadcindex slowadclist slowadcname slowadcvalues start status stop stopport storagecell_delay storagecell_start subdeadtime subexptime sync syncclk temp_10ge temp_adc temp_control temp_dcdc temp_event temp_fpga temp_fpgaext temp_fpgafl temp_fpgafr temp_slowadc temp_sodl temp_sodr temp_threshold templist tempvalues tengiga threshold thresholdnotb timing timing_info_decoder timinglist timingsource top transceiverenable trigger triggers triggersl trimbits trimen trimval tsamples txdelay txdelay_frame txdelay_left txdelay_right type udp_cleardst udp_dstip udp_dstip2 udp_dstlist udp_dstmac udp_dstmac2 udp_dstport udp_dstport2 udp_firstdst udp_numdst udp_reconfigure udp_srcip udp_srcip2 udp_srcmac udp_srcmac2 udp_validate update updatedetectorserver updatekernel updatemode user v_a v_b v_c v_chip v_d v_io v_limit vchip_comp_adc vchip_comp_fe vchip_cs vchip_opa_1st vchip_opa_fd vchip_ref_comp_fe versions veto vetoalg vetofile vetophoton vetoref vetostream virtual vm_a vm_b vm_c vm_d vm_io zmqhwm zmqip zmqport "
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
__define() {
FCN_RETURN=""
}
