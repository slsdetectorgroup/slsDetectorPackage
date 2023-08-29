#/usr/bin/env bash
_slsdet_completions()
{
  if [ "${#COMP_WORDS[@]}" != "2" ]; then
    return
  fi
  SLSDETCMDS=$(sls_detector_get list | sed -e "1d")
  COMPREPLY=($(compgen -W "${SLSDETCMDS}" "${COMP_WORDS[1]}"))

}
complete -F _slsdet_completions sls_detector_get
complete -F _slsdet_completions sls_detector_put


#if the "normal" aliases are set up then
#add completion for them as well
if command -v g &> /dev/null; then
  complete -F _slsdet_completions g
fi
if command -v p &> /dev/null; then
  complete -F _slsdet_completions p
fi
