#!/bin/bash

sudo chrt -f 1 ./test_rtx_linux -o "results/fast_build/fast_build_normal.csv"    -f 50 -s eternal-valley -p normal --rebuild 1 --spp 64 --fast-build
sudo chrt -f 1 ./test_rtx_linux -o "results/fast_build/fast_trace_normal.csv"    -f 50 -s eternal-valley -p normal --rebuild 1 --spp 64

sudo chrt -f 1 ./test_rtx_linux -o "results/fast_build/fast_build_shadow.csv"    -f 50 -s eternal-valley -p shadow --rebuild 1 --spp 64 --fast-build
sudo chrt -f 1 ./test_rtx_linux -o "results/fast_build/fast_trace_shadow.csv"    -f 50 -s eternal-valley -p shadow --rebuild 1 --spp 64