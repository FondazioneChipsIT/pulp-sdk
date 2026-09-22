#include "pmsis.h"
#include "stdio.h"

#include "idma_def_1d.h"
#include "idma_presets.h"
#include "idma_param_1d.h"

#ifndef _DMA_FRONTEND_REG_DEFS_
#define _DMA_FRONTEND_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif

#define DMA_CONF_DECOUPLE 0
#define DMA_CONF_DEBURST 0
#define DMA_CONF_SERIALIZE 0

#define CORE_SPACE 1024
#define TOT_SIZE ARCHI_CLUSTER_NB_PE * CORE_SPACE
#define NB_PRESETS 14

#ifdef DEBUG_TEST
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif

#ifdef CYCLE_COUNT
    static inline void start_cycle_count () { pi_perf_cl_start(); }
    static inline void stop_cycle_count () { pi_perf_cl_stop(); }
    static inline void reset_cycle_count () { pi_perf_conf((1<<PI_PERF_CYCLES) | \
    (1<<PI_PERF_INSTR) | (1<<PI_PERF_ACTIVE_CYCLES) | (1<<PI_PERF_LD_EXT) | (1<<PI_PERF_ST_EXT) | \
    (1<<PI_PERF_TCDM_CONT) | (1<<PI_PERF_LD_STALL) | (1<<PI_PERF_JR_STALL) | (1<<PI_PERF_IMISS) | \
    (1<<PI_PERF_LD) | (1<<PI_PERF_ST) | (1<<PI_PERF_LD_EXT_CYC) | (1<<PI_PERF_ST_EXT_CYC) | \
    (1<<PI_PERF_BRANCH) | (1<<PI_PERF_BTAKEN) | (1<<PI_PERF_JUMP));
    pi_perf_cl_reset(); }
    static inline unsigned int getcycles() { return pi_perf_cl_read(PI_PERF_CYCLES); }
    static inline void print_stats() {
        unsigned int _cycles, _instr, _active, _ldext, _stext, _ldextcyc, _stextcyc, _tcdmcont, _ldstall, _jrstall, _imiss, _ild, _ist, _ijump, _ibranch, _ibtaken;
        _cycles   += pi_perf_cl_read (PI_PERF_CYCLES);
        _instr    += pi_perf_cl_read (PI_PERF_INSTR);
        _active   += pi_perf_cl_read (PI_PERF_ACTIVE_CYCLES);
        _ldstall  += pi_perf_cl_read (PI_PERF_LD_STALL);
        _jrstall  += pi_perf_cl_read (PI_PERF_JR_STALL);
        _imiss    += pi_perf_cl_read (PI_PERF_IMISS);
        _ild      += pi_perf_cl_read (PI_PERF_LD);
        _ist      += pi_perf_cl_read (PI_PERF_ST);
        _ijump    += pi_perf_cl_read (PI_PERF_JUMP);
        _ibranch  += pi_perf_cl_read (PI_PERF_BRANCH);
        _ibtaken  += pi_perf_cl_read (PI_PERF_BTAKEN);
        _ldext    += pi_perf_cl_read (PI_PERF_LD_EXT);
        _stext    += pi_perf_cl_read (PI_PERF_ST_EXT);
        _ldextcyc += pi_perf_cl_read (PI_PERF_LD_EXT_CYC);
        _stextcyc += pi_perf_cl_read (PI_PERF_ST_EXT_CYC);
        _tcdmcont += pi_perf_cl_read (PI_PERF_TCDM_CONT);

        printf("cycles = %d\n", _cycles);
        printf("instr = %d\n", _instr);
        printf("active cycles = %d\n", _active);
        printf("ld stall = %d\n", _ldstall);
        printf("jr stall = %d\n", _jrstall);
        printf("imiss = %d\n", _imiss);
        printf("ld = %d\n", _ild);
        printf("st = %d\n", _ist);
        printf("jump = %d\n", _ijump);
        printf("branch = %d\n", _ibranch);
        printf("btaken = %d\n", _ibtaken);
        printf("ext load = %d\n", _ldext);
        printf("ext store = %d\n", _stext);
        printf("ext load cycles = %d\n", _ldextcyc);
        printf("ext store cycles = %d\n", _stextcyc);
        printf("TCDM cont = %d\n", _tcdmcont);
    }
#else
    static inline void start_cycle_count () {}
    static inline void stop_cycle_count () {}
    static inline void reset_cycle_count () {}
    static inline unsigned int getcycles() {return 0;}
    static inline void print_stats() {}
#endif


#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _DMA_FRONTEND_REG_DEFS_
