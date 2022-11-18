#ifdef IN_FAASM
#error Trying to compile PIN-compatible emulation in faasm mode
#endif
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <faasm/faasm.h>
#include <ndpapi.h>
#include <string.h>
#include <string>
#include <stublib.h>
#include <utility>
#include <vector>

namespace {
std::vector<std::pair<uint8_t*, size_t>> mmaps;
}

int32_t __faasmndp_put(const char* keyPtr,
                       uint32_t keyLen,
                       const uint8_t* dataPtr,
                       uint32_t dataLen)
{
    return 0;
}

uint8_t* __faasmndp_getMmap(const char* keyPtr,
                            int32_t keyLen,
                            int64_t offset,
                            int64_t maxRequestedLen,
                            uint32_t* outDataLenPtr)
{
    pinnearmap_phase("ignore-fread");
    std::string fpath(keyPtr, size_t(keyLen));
    FILE* fp = fopen(fpath.c_str(), "rb");
    if (fp == nullptr) {
        fprintf(stderr, "[PINfaasmEMU] Couldn't open file %s", fpath.c_str());
        std::terminate();
    }
    size_t size = 0;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    if (size > offset) {
        offset = size;
    }
    fseek(fp, offset, SEEK_SET);
    uint8_t* data = (uint8_t*)calloc(size + 32 - offset, 1);
    size = fread(data, 1, size - offset, fp);
    fclose(fp);
    *outDataLenPtr =
      std::min(uint32_t(maxRequestedLen), (uint32_t)(size - offset));
    mmaps.push_back(std::make_pair(data, size));
    pinnearmap_phase("after-fread");
    pinnearmap_io_bytes(size);
    return data;
}

int __faasmndp_storageCallAndAwait(const char* keyPtr,
                                   uint32_t keyLen,
                                   FaasmNdpFuncPtr funcPtr)
{
    pinnearmap_phase("offloaded");
    int val = funcPtr();
    pinnearmap_phase("ignore-mmaps-clear");
    for (auto [ptr, size] : mmaps) {
        memset(ptr, 0, size);
        free(ptr);
    }
    mmaps.clear();
    pinnearmap_phase("post-offloaded");
    return val;
}

int __faasmndp_storageCallAndAwait1(const char* keyPtr,
                                    uint32_t keyLen,
                                    FaasmNdpFuncPtr funcPtr,
                                    int32_t arg1)
{
    return __faasmndp_storageCallAndAwait(keyPtr, keyLen, funcPtr);
}
int __faasmndp_storageCallAndAwait2(const char* keyPtr,
                                    uint32_t keyLen,
                                    FaasmNdpFuncPtr funcPtr,
                                    int32_t arg1,
                                    int32_t arg2)
{
    return __faasmndp_storageCallAndAwait(keyPtr, keyLen, funcPtr);
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

#undef main
int faasm_main(int argc, char** argv);
int main(int argc, char** argv)
{
    pinnearmap_phase("main");
    return faasm_main(argc, argv);
}
