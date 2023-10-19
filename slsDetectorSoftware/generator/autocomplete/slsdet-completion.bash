#### simpler version of autocomplete.sh to understand the logic
#### each command has its own function when called it will produce the possible values for autocompletion


_sd() {
  local FCN_RETURN=""
  local IS_PATH=0

  __exptime(){
    if [ "${IS_GET}" == "1" ]; then
      if [ "${COMP_CWORD}" == "2" ]; then
        FCN_RETURN="s ms us ns"
      fi
    else
      if [ "${COMP_CWORD}" == "2" ]; then
        FCN_RETURN=""
      fi

      if [ "${COMP_CWORD}" == "3" ]; then
        FCN_RETURN="s ms us ns"
      fi

    fi
  }

  # trimbits will activate IS_PATH and signal that its input is a path
  __trimbits(){
    if [ "${IS_GET}" == "1" ]; then
      if [ "${COMP_CWORD}" == "2" ]; then
        FCN_RETURN=""
        IS_PATH=1
      fi
    else
      if [ "${COMP_CWORD}" == "2" ]; then
        FCN_RETURN=""
        IS_PATH=1
      fi
    fi
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
    local SLS_COMMANDS="trimbits exptime"

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

  # if a command is written, autocomplete with the options
  # call the function for the command
  __"${COMP_WORDS[1]##*:}"

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
