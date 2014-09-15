#!/bin/sh

# This shim is necessary to allow Candy Crisis to load resources from
# the app bundle without needing an obj-c compatibility layer. It sets
# the current working directory of the executable.

cd "${0%/*}"
./CandyCrisis
