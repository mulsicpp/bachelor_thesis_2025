#!/bin/bash

./test_rtx_linux -o "results/rebuild_every_8_frames.csv" -f 100 -s brainstem --spp 64 -p basic --rebuild 8
./test_rtx_linux -o "results/rebuild_never.csv"          -f 100 -s brainstem --spp 64 -p basic --rebuild 0
./test_rtx_linux -o "results/rebuild_always.csv"         -f 100 -s brainstem --spp 64 -p basic --rebuild 1

./test_rtx_linux -o "results/rebuild_fast_build.csv"     -f 100 -s brainstem --spp 64 -p basic --rebuild 1 --fast-build
./test_rtx_linux -o "results/rebuild_fast_trace.csv"     -f 100 -s brainstem --spp 64 -p basic --rebuild 1