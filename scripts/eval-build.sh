#!/bin/bash

set -e

cd "${0%/*}"
cd ..

rm -rf rel
mkdir rel
cd rel

cmake .. -GNinja -DCMAKE_CXX_FLAGS="-march=native -Ofast"
ninja scc

