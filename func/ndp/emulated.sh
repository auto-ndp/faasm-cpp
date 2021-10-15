#!/bin/bash

EMUCLANGPP="clang++ -static -flto=full -Dmain=faasm_main -std=c++17 -O2 -g -I. -Iapi -I../../libfaasm/ -I../../third-party/eigen-3.4/ -DEIGEN_INITIALIZE_MATRICES_BY_ZERO=1 -DEIGEN_NO_AUTOMATIC_RESIZING=1 -DEIGEN_NO_IO=1 -DEIGEN_DONT_PARALLELIZE=1 -DEIGEN_NO_CUDA=1 pintool_emu.cpp"

for PROG in *.cpp
do
    if [[ "${PROG}" == "pintool_emu.cpp" ]]; then
        continue;
    fi
    OUTF=${PROG/.cpp/_emu}
    #if [[ "${PROG}" -nt "${OUTF}" ]]; then
        echo "Compiling ${PROG} -> ${OUTF}"
        ${EMUCLANGPP} "${PROG}" "$HOME/phd/code/pin-NearMAP/stublib/stublib.a" -o "${OUTF}" || exit 1
    #else
    #    echo "Skipping ${PROG} - no changes"
    #fi
done
