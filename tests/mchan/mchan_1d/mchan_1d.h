#include "pmsis.h"
#include "stdio.h"
#include "mchan_def_1d.h"
#include "mchan_param_1d.h"

#ifdef DEBUG_TEST
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif

#define CORE_SPACE 2048