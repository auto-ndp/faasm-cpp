#ifndef STUBLIB_PIN_NEARMAP_H_INCLUDED
#define STUBLIB_PIN_NEARMAP_H_INCLUDED 1

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef IN_FAASM
    static void pinnearmap_phase(const char* name) {}
    static void pinnearmap_io_bytes(uint64_t bytes) {}
#else
void pinnearmap_phase(const char* name);
void pinnearmap_io_bytes(uint64_t bytes);
#endif

#ifdef __cplusplus
}
#endif

#endif
