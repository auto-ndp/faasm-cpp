#!/bin/bash
# FIXME
# Currently not in a single distribution, so using absolute path

NDPCODE="$HOME/phd/code"
ANALYSIS_EXE="${NDPCODE}/pin-NearMAP/analyse-results/target/release/analyse-results"
PIN_EXE="${NDPCODE}/third-party/pin3.18/pin"
PIN_TOOL="${NDPCODE}/pin-NearMAP/obj-intel64/NearMAP.so"

mkdir -p pintraces
rm -f pintraces/*.log

for PAGESIZE in 16 64 256 1024 4096
do
    echo Page size: ${PAGESIZE}

    export FAASM_INPUT="$(pwd)/data/tiki-4m.txt"
    FUNC=wordcount
    echo "${FUNC}" && "${PIN_EXE}" -t "${PIN_TOOL}" -p "${PAGESIZE}" -o "pintraces/${FUNC}_${PAGESIZE}.log" -- "./${FUNC}_emu" 2> "${FUNC}_${PAGESIZE}_pinlog.log" > "${FUNC}_${PAGESIZE}_funclog.log"

    FUNC=sha256sum
    echo "${FUNC}" && "${PIN_EXE}" -t "${PIN_TOOL}" -p "${PAGESIZE}" -o "pintraces/${FUNC}_${PAGESIZE}.log" -- "./${FUNC}_emu" 2> "${FUNC}_${PAGESIZE}_pinlog.log" > "${FUNC}_${PAGESIZE}_funclog.log"

    export FAASM_INPUT="$(pwd)/data/tiki-4m.txt User"
    FUNC=grep
    echo "${FUNC}" && "${PIN_EXE}" -t "${PIN_TOOL}" -p "${PAGESIZE}" -o "pintraces/${FUNC}_${PAGESIZE}.log" -- "./${FUNC}_emu" 2> "${FUNC}_${PAGESIZE}_pinlog.log" > "${FUNC}_${PAGESIZE}_funclog.log"
    FUNC=substr
    echo "${FUNC}" && "${PIN_EXE}" -t "${PIN_TOOL}" -p "${PAGESIZE}" -o "pintraces/${FUNC}_${PAGESIZE}.log" -- "./${FUNC}_emu" 2> "${FUNC}_${PAGESIZE}_pinlog.log" > "${FUNC}_${PAGESIZE}_funclog.log"

done

echo Analysis
${ANALYSIS_EXE} pintraces
