#ifndef STUBLIB_PIN_NEARMAP_H_INCLUDED
#define STUBLIB_PIN_NEARMAP_H_INCLUDED 1

#ifdef __cplusplus
extern "C" {
#endif

#ifdef IN_FAASM
static void pinnearmap_phase(const char* name) {}
#else
void pinnearmap_phase(const char* name);
#endif

#ifdef __cplusplus
}
#endif

#endif
