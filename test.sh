#!/usr/bin/env bash

if [ $(git rev-parse --show-toplevel) != $(pwd) ]; then
    echo "You need to run this script from the root folder of the Git repository."
    echo "Try to run it from: "$(git rev-parse --show-toplevel)
    exit 1
fi

export FOAM_FOAMUT=/tmp/FoamUT
rm -rf $FOAM_FOAMUT
git clone https://github.com/FoamScience/foamUT $FOAM_FOAMUT

export CURR_TESTS=$PWD

ln -s "$CURR_TESTS" "$FOAM_FOAMUT/tests/openmp"
cd $FOAM_FOAMUT || exit 1
rm -rf tests/exampleTests
./Alltest --no-parallel "$@"
