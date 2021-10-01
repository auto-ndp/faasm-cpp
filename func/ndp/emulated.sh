#!/bin/sh

EMUCLANGPP="clang++ -std=c++17 -O2 -g -I. -Iapi -I../../libfaasm/ pintool_emu.cpp"

${EMUCLANGPP} json_parse.cpp $HOME/phd/code/pin-NearMAP/stublib/stublib.a -o json_parse_emu
${EMUCLANGPP} wordcount.cpp $HOME/phd/code/pin-NearMAP/stublib/stublib.a -o wordcount_emu
${EMUCLANGPP} sha256sum.cpp $HOME/phd/code/pin-NearMAP/stublib/stublib.a -o sha256sum_emu
${EMUCLANGPP} grep.cpp $HOME/phd/code/pin-NearMAP/stublib/stublib.a -o grep_emu
