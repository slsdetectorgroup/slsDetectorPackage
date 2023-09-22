#!/bin/bash
_sd ()
{
	OPTIONS=`sls_detector_get list | sed -e "1d"`
	local cur
	COMPREPLY=()
	OPTIONS_NEW=""
	cur=${COMP_WORDS[COMP_CWORD]}

	if [ "${COMP_WORDS[0]}" == "sls_detector_put" ]; then
		PUT=1
	else
		PUT=0
	fi

	# First argument is the command
	if [ "$COMP_CWORD" == "1" ]; then
		case "$cur" in
			[0-9]*)
				for i in $OPTIONS; do
					OPTIONS_NEW="${OPTIONS_NEW} ${cur%%:*}:$i"
				done
				COMPREPLY=( $( compgen -W "${OPTIONS_NEW}" -- "$cur" ) );;
			*)
				COMPREPLY=( $( compgen -W "$OPTIONS -h" -- "$cur" ) );;
		esac
	elif [ "$COMP_CWORD" == "2" ]; then

		# Help
		if [ "${COMP_WORDS[1]}" == "-h" ]; then
			COMPREPLY=( $( compgen -W "$OPTIONS" -- "$cur" ) )

		# All commands with [0, 1] in help text  ## TODO: Doesn't work for datastream
		elif ( echo $OPTIONS | grep -sq -- ${COMP_WORDS[1]}) && (sls_detector_get -h ${COMP_WORDS[1]} | grep -sq "\[0, 1\]") && [ "$PUT" == "1" ]; then
			COMPREPLY=( $( compgen -W "0 1" -- "$cur" ) )
		# Activate file/path completion for all commands with fname or path in first line of help text
		elif ( echo $OPTIONS | grep -sq -- ${COMP_WORDS[1]}) && (sls_detector_get -h ${COMP_WORDS[1]} | grep -sq "path\|fname") && [ "$PUT" == "1" ]; then
					COMPREPLY=($(compgen -f ${COMP_WORDS[${COMP_CWORD}]} ) )
		else
			case "${COMP_WORDS[1]}" in
				# Commands offering a certain static choice
				"badchannels"|"fformat"|"gainmode"|"polarity"|"romode"|"timing"|"timingsource"|"updatemode")
					if [ "$PUT" == "1" ]; then
						OPTIONS=`sls_detector_get -h ${COMP_WORDS[1]}| awk '{print $2}' | head -n 1| tr '|[]' ' '`
						COMPREPLY=( $( compgen -W "$OPTIONS" -- "$cur") )
					fi
					;;
				# Commands offering a detector dependend choice having a ${CMD}list command
				"slowadc"|"dac"|"dr"|"readoutspeed"|"settings"|"temp"|"timing")
					CMD=${COMP_WORDS[1]}
					if [ "$PUT" == "1" ]; then
						OPTIONS=`sls_detector_get ${CMD}list | tr '[],' ' ' | awk '{$1="" ; print }'`
						COMPREPLY=( $( compgen -W "$OPTIONS" -- "$cur") )
					fi
					;;
#				*)
#					COMPREPLY=($(compgen -f ${COMP_WORDS[${COMP_CWORD}]} ) )
			esac
		fi
#	else
#		COMPREPLY=($(compgen -f ${COMP_WORDS[${COMP_CWORD}]} ) )
	fi

	return 0
}

complete -F _sd -o filenames sls_detector_get
complete -F _sd -o filenames sls_detector_put

