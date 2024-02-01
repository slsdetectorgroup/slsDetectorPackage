#### simpler version of autocomplete.sh to understand the logic
#### each command has its own function when called it will produce the possible values for autocompletion


_sd() {

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



  __exptime(){
    if [ "${IS_GET}" == "1" ]; then
      if [ "${cword}" == "2" ]; then
        FCN_RETURN="s ms us ns"
      fi
    else
      if [ "${cword}" == "2" ]; then
        FCN_RETURN=""
      fi

      if [ "${cword}" == "3" ]; then
        FCN_RETURN="s ms us ns"
      fi

    fi
  }

  # trimbits will activate IS_PATH and signal that its input is a path
  __trimbits(){
    if [ "${IS_GET}" == "1" ]; then
      if [ "${cword}" == "2" ]; then
        FCN_RETURN=""
        IS_PATH=1
      fi
    else
      if [ "${cword}" == "2" ]; then
        FCN_RETURN=""
        IS_PATH=1
      fi
    fi
  }

  local cword words=()
  _comp__reassemble_words ":" words cword
  local FCN_RETURN=""
  local IS_PATH=0
  COMPREPLY=()
  local OPTIONS_NEW=""
#  _get_comp_words_by_ref -n : cur
  local cur=${words[cword]}
  #  check the action (get or put)
  if [ "${words[0]}" == "sls_detector_get" ]; then
    local IS_GET=1
  else
    local IS_GET=0
  fi


  # if no command is written, autocomplete with the commands
  if [[ ${cword} -eq 1 ]]; then
    local SLS_COMMANDS="trimbits exptime"
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

  # if a command is written, autocomplete with the options
  # call the function for the command
  __"${words[1]##*:}"

  # if IS_PATH is activated, autocomplete with the path
  if [[ ${IS_PATH} -eq 1 ]]; then
    COMPREPLY=($(compgen -f -- "${cur}"))
    return 0
  fi

  # autocomplete with the options
  COMPREPLY=($(compgen -W "${FCN_RETURN}" -- "${cur}"))



}

complete -F _sd -o filenames sls_detector_put
complete -F _sd -o filenames sls_detector_get

complete -F _sd -o filenames g
complete -F _sd -o filenames p

complete -F _sd -o filenames detg
complete -F _sd -o filenames detp


complete -F _sd -o filenames sls_detector
complete -F _sd -o filenames det

