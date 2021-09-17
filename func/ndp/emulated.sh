#!/bin/sh

clang++ -std=c++17 -O2 -g -I. -Iapi -I../../libfaasm/ json_parse.cpp pintool_emu.cpp $HOME/phd/code/pin-NearMAP/stublib/stublib.a -o json_parse_emu
