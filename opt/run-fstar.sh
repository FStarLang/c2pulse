#!/usr/bin/env bash
OPTDIR="$(dirname "${BASH_SOURCE[0]}")"
exec fstar.exe $OTHERFLAGS --include $OPTDIR/../pulse "$@"