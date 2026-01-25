#!/bin/bash
set -o pipefail

make -C build

./build/src/junkcraft
