#ifndef NDPAPI_H_INCLUDED
#define NDPAPI_H_INCLUDED 1

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef int (*FaasmNdpFuncPtr)();

    int32_t __faasmndp_put(const uint8_t* keyPtr,
                           uint32_t keyLen,
                           const uint8_t* dataPtr,
                           uint32_t dataLen);

    uint8_t* __faasmndp_getMmap(const uint8_t* keyPtr,
                                uint32_t keyLen,
                                uint32_t maxRequestedLen,
                                uint32_t* outDataLenPtr);

    int __faasmndp_storageCallAndAwait(FaasmNdpFuncPtr funcPtr);

    /// Mode can only be "read"
    static inline FILE* faasmndp_fopen(const char* name, const char* mode)
    {
        uint32_t nameLen = strlen(name);
        uint32_t fileLen = 0;
        uint8_t* fileData = __faasmndp_getMmap(
          (const uint8_t*)name, nameLen, 0xFFFFFFFF, &fileLen);
        if (fileData == NULL) {
            return NULL;
        }
        return fmemopen(fileData, fileLen, mode);
    }

#ifdef __cplusplus
}
#endif

#endif