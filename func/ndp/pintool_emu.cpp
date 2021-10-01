#ifdef IN_FAASM
#error Trying to compile PIN-compatible emulation in faasm mode
#endif
#include <algorithm>
#include <faasm/faasm.h>
#include <ndpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stublib.h>
#include <utility>
#include <vector>
#include <iostream>
#include <cstdlib>

namespace {
std::vector<std::pair<uint8_t*, size_t>> mmaps;
}

int32_t __faasmndp_put(const uint8_t* keyPtr,
                       uint32_t keyLen,
                       const uint8_t* dataPtr,
                       uint32_t dataLen)
{
    return 0;
}

uint8_t* __faasmndp_getMmap(const uint8_t* keyPtr,
                            uint32_t keyLen,
                            uint32_t maxRequestedLen,
                            uint32_t* outDataLenPtr)
{
    std::string fpath(reinterpret_cast<const char*>(keyPtr), size_t(keyLen));
    FILE* fp = fopen(fpath.c_str(), "rb");
    if (fp == nullptr) {
        std::cerr << "[PINfaasmEMU] Couldn't open file " << fpath;
        std::terminate();
    }
    size_t size = 0;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    uint8_t* data = (uint8_t*)calloc(size + 32, 1);
    size = fread(data, 1, size, fp);
    fclose(fp);
    *outDataLenPtr = std::min(maxRequestedLen, (uint32_t)size);
    mmaps.push_back(std::make_pair(data, size));
    return data;
}

int __faasmndp_storageCallAndAwait(FaasmNdpFuncPtr funcPtr)
{
    pinnearmap_phase("offloaded");
    int val = funcPtr();
    pinnearmap_phase("post-offloaded");
    for (auto [ptr, size] : mmaps) {
        memset(ptr, 0, size);
    }
    mmaps.clear();
    return val;
}

const char* FAASM_INPUT_ENV = "FAASM_INPUT";

/**
 * Returns the size of the input in bytes. Returns zero if none.
 * */
long faasmGetInputSize()
{
    return strlen(getenv(FAASM_INPUT_ENV));
}

/**
 * Returns a pointer to the input data for this function
 */
void faasmGetInput(uint8_t* buffer, long bufferLen)
{
    char* src = getenv(FAASM_INPUT_ENV);
    strncpy((char*)buffer, src, bufferLen);
}

/**
 * Sets the given array as the output data for this function
 */
void faasmSetOutput(const uint8_t* newOutput, long outputLen)
{
    fwrite(newOutput, 1, outputLen, stdout);
}
