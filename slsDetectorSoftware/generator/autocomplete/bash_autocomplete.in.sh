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


  # -- THIS LINE WILL BE REPLACED WITH GENERATED CODE --


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
