#ifndef __QUIC_ACCEPTOR_H_
#define __QUIC_ACCEPTOR_H_

#include<stdint.h>
#include<stdbool.h>
#include<stdatomic.h>

#include<erl_nif.h>

typedef struct Acceptor_r {
	uint64_t Id;
	ErlNifPid AcceptorPid;
	atomic_bool IsWork;
	struct Acceptor_r* next;
} Acceptor;

bool initAcceptors(atomic_ullong* IndexPtr, uint64_t Cnt) {
	if (Cnt < 1) return false;
	Acceptor* HeadPtr = NULL;
	Acceptor* TemPtr = NULL;
	Acceptor* NewPtr = NULL;

	for (uint64_t i = 1; i <= Cnt; ++i) {
		NewPtr = (Acceptor*)enif_alloc(sizeof(Acceptor));
		if (NULL == NewPtr) goto Failed;
		if (NULL == HeadPtr) HeadPtr = NewPtr;

		NewPtr->Id = i;
		atomic_store(&NewPtr->IsWork, false);
		NewPtr->next = NULL;

		if (NULL != TemPtr) TemPtr->next = NewPtr;
		TemPtr = NewPtr;
	}
	TemPtr->next = HeadPtr;
	atomic_store(IndexPtr, (atomic_ullong)HeadPtr);
	return true;

Failed:
	TemPtr = HeadPtr;
	for (uint64_t i = 1; i <= Cnt; ++i) {
		if (NULL == TemPtr) break;

		NewPtr = TemPtr->next;
		enif_free(TemPtr);
		TemPtr = NewPtr;
	}

	return false;
}

bool setAcceptor(atomic_ullong* IndexPtr, uint64_t Id, ErlNifPid AcceptorPid) {
	Acceptor* CurPtr = (Acceptor*)atomic_load(IndexPtr);
	if (NULL == CurPtr) return false;

	Acceptor* TemPtr = CurPtr;
	do {
		if (TemPtr->Id == Id) {
			TemPtr->AcceptorPid = AcceptorPid;
			atomic_store(&TemPtr->IsWork, true);
			return true;
		}
		TemPtr = TemPtr->next;
	} while (TemPtr != CurPtr);
	return false;
}

bool unsetAcceptor(atomic_ullong* IndexPtr, ErlNifPid AcceptorPid) {
	Acceptor* CurPtr = (Acceptor*)atomic_load(IndexPtr);
	if (NULL == CurPtr) return false;

	Acceptor* TemPtr = CurPtr;
	do {
		if (!enif_compare_pids(&TemPtr->AcceptorPid, &AcceptorPid)) {
			atomic_store(&TemPtr->IsWork, false);
			return true;
		}
		TemPtr = TemPtr->next;
	} while (TemPtr != CurPtr);
	return false;
}

uint64_t cntAcceptor(atomic_ullong* IndexPtr) {
	Acceptor* CurPtr = (Acceptor*)atomic_load(IndexPtr);
	uint64_t i = 0;
	if (NULL == CurPtr) return i;

	Acceptor* TemPtr = CurPtr;
	do {
		i++;
		TemPtr = TemPtr->next;
	} while (TemPtr != CurPtr);
	return i;
}

ErlNifPid* nextAcceptor(atomic_ullong* IndexPtr) {
	Acceptor* CurPtr = (Acceptor*)atomic_load(IndexPtr);
	if (NULL == CurPtr) return NULL;
	if (atomic_load(&CurPtr->IsWork)) {
		atomic_store(IndexPtr, (atomic_ullong)CurPtr->next);
		return &CurPtr->AcceptorPid;
	}

	Acceptor* TemPtr = CurPtr->next;
	do {
		if (atomic_load(&TemPtr->IsWork)) {
			atomic_store(IndexPtr, (atomic_ullong)TemPtr->next);
			return &TemPtr->AcceptorPid;
		}
		TemPtr = TemPtr->next;
	} while (TemPtr != CurPtr);

	return NULL;
}

Acceptor* curAcceptor(atomic_ullong* IndexPtr) {
	Acceptor* CurPtr = (Acceptor*)atomic_load(IndexPtr);
	if (NULL == CurPtr) return NULL;
	return CurPtr;
}

void clearAcceptors(atomic_ullong* IndexPtr) {
	Acceptor* CurPtr = (Acceptor*)atomic_load(IndexPtr);
	if (NULL == CurPtr) return;

	Acceptor* TemPtr = CurPtr;
	Acceptor* FreePtr = NULL;
	do {
		FreePtr = TemPtr;
		TemPtr = TemPtr->next;
		enif_free(FreePtr);
	} while (TemPtr != CurPtr);

	atomic_store(IndexPtr, 0);
	return;
}

#endif
