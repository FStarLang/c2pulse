#!/usr/bin/env bash
OPTDIR="$(dirname "${BASH_SOURCE[0]}")"
source $OPTDIR/env.sh
exec fstar.exe --include $PULSE_HOME/lib/pulse --include $OPTDIR/../pulse "$@"