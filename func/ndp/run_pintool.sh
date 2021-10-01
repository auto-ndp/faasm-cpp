#!/bin/bash
# FIXME
# Currently not in a single distribution, so using absolute path

NDPCODE="$HOME/phd/code"
ANALYSIS_EXE="${NDPCODE}/pin-NearMAP/analyse-results/target/release/analyse-results"
PIN_EXE="${NDPCODE}/third-party/pin3.18/pin"
PIN_TOOL="${NDPCODE}/pin-NearMAP/obj-intel64/NearMAP.so"

mkdir -p pintraces
rm -f pintraces/*.log

export FAASM_INPUT="$(pwd)/data/tiki-4m.txt"
FUNC=wordcount
echo "${FUNC}" && "${PIN_EXE}" -t "${PIN_TOOL}" -o "pintraces/${FUNC}.log" -- "./${FUNC}_emu" 2> "${FUNC}_pinlog.log" > "${FUNC}_funclog.log"

FUNC=sha256sum
echo "${FUNC}" && "${PIN_EXE}" -t "${PIN_TOOL}" -o "pintraces/${FUNC}.log" -- "./${FUNC}_emu" 2> "${FUNC}_pinlog.log" > "${FUNC}_funclog.log"

export FAASM_INPUT="$(pwd)/data/tiki-4m.txt User"
FUNC=grep
echo "${FUNC}" && "${PIN_EXE}" -t "${PIN_TOOL}" -o "pintraces/${FUNC}.log" -- "./${FUNC}_emu" 2> "${FUNC}_pinlog.log" > "${FUNC}_funclog.log"

echo Analysis
${ANALYSIS_EXE} pintraces
