# GENERATED FILE - DO NOT EDIT
# ANY CHANGES TO THIS FILE WILL BE OVERWRITTEN

_sd() {
  local FCN_RETURN=""
  local IS_PATH=0


  # -- THIS LINE WILL BE REPLACED WITH GENERATED CODE --


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
