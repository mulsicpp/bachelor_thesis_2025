#!/bin/bash

./test_rtx_linux -o "results/spp_1.csv"    -f 100 -s space-station -p shadow --rebuild 1 --spp 1
./test_rtx_linux -o "results/spp_4.csv"    -f 100 -s space-station -p shadow --rebuild 1 --spp 4
./test_rtx_linux -o "results/spp_9.csv"    -f 100 -s space-station -p shadow --rebuild 1 --spp 9
./test_rtx_linux -o "results/spp_16.csv"   -f 100 -s space-station -p shadow --rebuild 1 --spp 16
./test_rtx_linux -o "results/spp_25.csv"   -f 100 -s space-station -p shadow --rebuild 1 --spp 25
./test_rtx_linux -o "results/spp_36.csv"   -f 100 -s space-station -p shadow --rebuild 1 --spp 36
./test_rtx_linux -o "results/spp_49.csv"   -f 100 -s space-station -p shadow --rebuild 1 --spp 49
./test_rtx_linux -o "results/spp_64.csv"   -f 100 -s space-station -p shadow --rebuild 1 --spp 64