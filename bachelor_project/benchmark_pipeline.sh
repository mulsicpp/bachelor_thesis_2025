#!/bin/bash

sudo chrt -f 1 ./test_rtx_linux -o "results/pipeline/basic.csv"     -f 64 -s eternal-valley --rebuild 1 --spp 64 -p basic
sudo chrt -f 1 ./test_rtx_linux -o "results/pipeline/normal.csv"    -f 64 -s eternal-valley --rebuild 1 --spp 64 -p normal
sudo chrt -f 1 ./test_rtx_linux -o "results/pipeline/shadow.csv"    -f 64 -s eternal-valley --rebuild 1 --spp 64 -p shadow