# GENERATED FILE - DO NOT EDIT
# ANY CHANGES TO THIS FILE WILL BE OVERWRITTEN

_sd() {


  # -- THIS LINE WILL BE REPLACED WITH GENERATED CODE --


  local FCN_RETURN=""
  local IS_PATH=0
  COMPREPLY=()
  local OPTIONS_NEW=""
  words=("${COMP_WORDS[@]}")
  cword=$COMP_CWORD

  local cur=${words[cword]}
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
