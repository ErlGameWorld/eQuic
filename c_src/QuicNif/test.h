#ifndef __test_H_
#define __test_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MakeFlag(Type, IsCanGet, IsCanSet) (1 << (Type + 3) | IsCanGet << 1 | IsCanSet)
#define IsCanGet(Flag) ((Flag >> 1) & 1 == 1)
#define IsCanSet(Flag) (Flag & 1 == 1)
#define IsCanType(Flag, Type) (Flag >> (Type + 3) & 1 == 1)


// https://github.com/microsoft/msquic/blob/main/docs/Settings.md
struct ParamCfg_r {
	char* ParamName;
	uint32_t ParamType;
	uint16_t HTGSFlag;	  // HandleTypeAndGetSetFlag ob0000000(not use) 111111(QUIC_HANDLE_TYPE) 1(Global)  11(get/set)
	int(*getFun)(uint32_t, uint32_t);
	int(*setFun)(uint32_t, uint32_t, uint32_t);
} ParamCfg[] = {
	// Global Parameters    Type -1
	{"eQUIC_PARAM_GLOBAL_RETRY_MEMORY_PERCENT", 0, MakeFlag(-1, 1, 1), NULL, NULL},
	{"eQUIC_PARAM_GLOBAL_SUPPORTED_VERSIONST", 1, MakeFlag(-1, 1, 0), NULL, NULL},
	{"eQUIC_PARAM_GLOBAL_LOAD_BALACING_MODE", 2, MakeFlag(-1, 1, 1), NULL, NULL},
	{"eQUIC_PARAM_GLOBAL_PERF_COUNTERS", 3, MakeFlag(-1, 1, 0), NULL, NULL},
	{"eQUIC_PARAM_GLOBAL_LIBRARY_VERSION", 4, MakeFlag(-1, 1, 1), NULL, NULL},
	{"eQUIC_PARAM_GLOBAL_SETTINGS", 5, MakeFlag(-1, 1, 1), NULL, NULL},
	{"eQUIC_PARAM_GLOBAL_GLOBAL_SETTINGS", 6, MakeFlag(-1, 1, 1), NULL, NULL},
	{"eQUIC_PARAM_GLOBAL_VERSION_SETTINGS", 7, MakeFlag(-1, 1, 1), NULL, NULL},
	{"eQUIC_PARAM_GLOBAL_LIBRARY_GIT_HASH", 8, MakeFlag(-1, 1, 0), NULL, NULL},
	{"eQUIC_PARAM_GLOBAL_DATAPATH_PROCESSORS", 9, MakeFlag(0, 1, 1), NULL, NULL},
	{"eQUIC_PARAM_GLOBAL_TLS_PROVIDER", 10, MakeFlag(1, 1, 1)  , NULL, NULL}
};

#endif
