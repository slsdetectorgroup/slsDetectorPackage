# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
#
# Put the binaries of a build tree on PATH/LD_LIBRARY_PATH/PYTHONPATH.
#
# Usage (the exports only survive when the script is sourced):
#   source this_build_bin_path.sh                # auto-detect build dir
#   source this_build_bin_path.sh build_qt6      # explicit build dir
#   SLS_BUILD_DIR=/path/to/build_qt6 source this_build_bin_path.sh
#
# Auto-detection looks for <script dir>/build_qt6, <script dir>/build,
# ../build_qt6, ../build and the same two names under $PWD; it prefers the
# Qt6 build directory.  The first candidate containing a bin/ subdirectory
# wins.  An explicit argument may also point directly at a bin/ directory.

# Resolve the directory of this script, also when it is sourced.
if [ -n "${BASH_SOURCE[0]:-}" ]; then
    _sls_script="${BASH_SOURCE[0]}"
else
    _sls_script="$0"
fi
_sls_script_dir=$(cd "$(dirname "${_sls_script}")" 2>/dev/null && pwd)

# pwd -P normalises symlinks and relative paths.
_sls_abs() {
    ( cd "$1" 2>/dev/null && pwd -P )
}

_sls_candidates=()
[ -n "${SLS_BUILD_DIR:-}" ] && _sls_candidates+=("${SLS_BUILD_DIR}")
[ -n "${1:-}" ] && _sls_candidates+=("${1}")
[ -n "${_sls_script_dir:-}" ] && _sls_candidates+=(
    "${_sls_script_dir}/build_qt6"
    "${_sls_script_dir}/build"
    "${_sls_script_dir}/../build_qt6"
    "${_sls_script_dir}/../build"
)
_sls_candidates+=("${PWD}/build_qt6" "${PWD}/build")

THIS_PATH=""
for _sls_candidate in "${_sls_candidates[@]}"; do
    if [ -d "${_sls_candidate}/bin" ]; then
        THIS_PATH="$(_sls_abs "${_sls_candidate}/bin")"
        break
    fi
    # Allow an explicit argument that already points at a bin/ directory.
    if [ -d "${_sls_candidate}" ] && [ "$(basename "${_sls_candidate}")" = "bin" ]; then
        THIS_PATH="$(_sls_abs "${_sls_candidate}")"
        break
    fi
done

if [ -z "${THIS_PATH}" ]; then
    echo "this_build_bin_path.sh: could not find a build bin/ directory." >&2
    echo "Tried:" >&2
    for _sls_candidate in "${_sls_candidates[@]}"; do
        [ -n "${_sls_candidate}" ] && echo "  ${_sls_candidate}/bin" >&2
    done
    echo "Pass the build directory explicitly, e.g.:" >&2
    echo "  source this_build_bin_path.sh build_qt6" >&2
    unset _sls_script _sls_script_dir _sls_candidates _sls_candidate _sls_abs
    return 1 2>/dev/null || exit 1
fi

# Keep the path free of a trailing slash and duplicate entries.
case ":${PATH:-}:" in
    *":${THIS_PATH}:"*) ;;
    *) export PATH="${THIS_PATH}:${PATH:-}" ;;
esac
case ":${LD_LIBRARY_PATH:-}:" in
    *":${THIS_PATH}:"*) ;;
    *) export LD_LIBRARY_PATH="${THIS_PATH}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}" ;;
esac
case ":${PYTHONPATH:-}:" in
    *":${THIS_PATH}:"*) ;;
    *) export PYTHONPATH="${THIS_PATH}${PYTHONPATH:+:${PYTHONPATH}}" ;;
esac
export SLS_BUILD_BIN="${THIS_PATH}"

echo "SLS build bin: ${THIS_PATH}"
echo "  PATH                += ${THIS_PATH}"
echo "  LD_LIBRARY_PATH     += ${THIS_PATH}"
echo "  PYTHONPATH          += ${THIS_PATH}"

# The command-line clients are separate targets; a GUI-only build may not have
# them.  Remind the user instead of silently leaving put/get unavailable.
_sls_missing=""
for _sls_tool in sls_detector_put sls_detector_get; do
    if ! command -v "${_sls_tool}" >/dev/null 2>&1; then
        _sls_missing="${_sls_missing} ${_sls_tool}"
    fi
done
if [ -n "${_sls_missing}" ]; then
    echo "note: not found in the selected build bin:${_sls_missing}" >&2
    echo "      build them with:" >&2
    echo "      cmake --build <build-dir> --target${_sls_missing} -j\$(nproc)" >&2
fi
unset _sls_missing _sls_tool

# When executed rather than sourced the exports disappear with the shell.
if [ "${BASH_SOURCE[0]:-}" = "$0" ]; then
    echo "NOTE: this script must be sourced to keep the environment:" >&2
    echo "      source $0" >&2
fi

unset _sls_script _sls_script_dir _sls_candidates _sls_candidate _sls_abs
