#!/bin/bash

sudo chrt -f 1 ./test_rtx_linux -o "results/update/normal_rebuild.csv"            -f 64 -s eternal-valley --spp 64 -p normal --rebuild 1
sudo chrt -f 1 ./test_rtx_linux -o "results/update/normal_refit.csv"              -f 64 -s eternal-valley --spp 64 -p normal --rebuild 0
sudo chrt -f 1 ./test_rtx_linux -o "results/update/normal_rebuild_every_8.csv"    -f 64 -s eternal-valley --spp 64 -p normal --rebuild 8

sudo chrt -f 1 ./test_rtx_linux -o "results/update/shadow_rebuild.csv"            -f 64 -s eternal-valley --spp 64 -p shadow --rebuild 1
sudo chrt -f 1 ./test_rtx_linux -o "results/update/shadow_refit.csv"              -f 64 -s eternal-valley --spp 64 -p shadow --rebuild 0
sudo chrt -f 1 ./test_rtx_linux -o "results/update/shadow_rebuild_every_8.csv"    -f 64 -s eternal-valley --spp 64 -p shadow --rebuild 8