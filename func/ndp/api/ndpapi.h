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

    int32_t __faasmndp_unmap(uint8_t* buffer,
                             int32_t length);

    int32_t __faasmndp_put(const char* keyPtr,
                           uint32_t keyLen,
                           const uint8_t* dataPtr,
                           uint32_t dataLen);

    uint8_t* __faasmndp_getMmap(const char* keyPtr,
                                int32_t keyLen,
                                int64_t offset,
                                int64_t maxRequestedLen,
                                uint32_t* outDataLenPtr);

    int __faasmndp_storageCallAndAwait(const char* keyPtr,
                                       uint32_t keyLen,
                                       FaasmNdpFuncPtr funcPtr);
    int __faasmndp_storageCallAndAwait1(const char* keyPtr,
                                        uint32_t keyLen,
                                        FaasmNdpFuncPtr funcPtr,
                                        int32_t arg1);
    int __faasmndp_storageCallAndAwait2(const char* keyPtr,
                                        uint32_t keyLen,
                                        FaasmNdpFuncPtr funcPtr,
                                        int32_t arg1,
                                        int32_t arg2);

    /// Mode can only be "read"
    static inline FILE* faasmndp_fopen(const char* name, const char* mode)
    {
        uint32_t nameLen = strlen(name);
        uint32_t fileLen = 0;
        uint8_t* fileData =
          __faasmndp_getMmap(name, nameLen, 0, INT32_MAX, &fileLen);
        if (fileData == NULL) {
            return NULL;
        }
        return fmemopen(fileData, fileLen, mode);
    }

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
template<class F>
int faasmndp_storageCallAndAwait(const char* keyPtr, uint32_t keyLen, F funcArg)
{
    struct Callable
    {
        F func;
        int execute() { return this->func(); }
        static int executeOn(Callable* self) { return self->execute(); }
    };
    Callable callable{ static_cast<F&&>(funcArg) };
    return __faasmndp_storageCallAndAwait1(
      keyPtr,
      keyLen,
      &Callable::executeOn,
      reinterpret_cast<intptr_t>(&callable));
}
#endif

#endif