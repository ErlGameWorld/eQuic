#include"test.h"
#include<erl_nif.h>
#include "quicAcceptor.h"

int main(int argc, char* argv[]) {
	//int len = sizeof(ParamCfg) / sizeof(struct ParamCfg_r);
	//for (int i = 0; i < len; i++) {
	//	printf("test I:%d name:%s falg:%d Isget:%d IsSet%d IsType:%d \n", i, ParamCfg[i].ParamName, ParamCfg[i].HTGSFlag, IsCanGet(ParamCfg[i].HTGSFlag), IsCanSet(ParamCfg[i].HTGSFlag), IsCanType(ParamCfg[i].HTGSFlag, -1));
	//
	uint64_t a = 11;
	uint64_t* p = &a;

	printf("%u %u \n", a, p);
	ERL_NIF_TERM b = (ERL_NIF_TERM)p;

	printf("%u %u \n", a, b);

	uint64_t* t = (uint64_t*)b;

	printf("%u %u \n", a, t);
	*t = 22;

	printf("%u %u %u %u   %u %u\n ", a, p, b, a, sizeof(int), sizeof(uint64_t));
	return 0;

}