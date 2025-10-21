#include "pmsis.h"
#include "stdio.h"
#include "mchan_def_2d.h"
#include "mchan_param_2d.h"

#ifdef DEBUG_TEST
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif

#define CORE_SPACE 2048