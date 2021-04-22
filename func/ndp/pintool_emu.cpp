#ifdef IN_FAASM
#error Trying to compile PIN-compatible emulation in faasm mode
#endif
#include <faasm/faasm.h>
#include <ndpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    return 0;
}

int __faasmndp_storageCallAndAwait(FaasmNdpFuncPtr funcPtr)
{
    return funcPtr();
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
