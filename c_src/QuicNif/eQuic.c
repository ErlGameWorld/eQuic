#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<stdatomic.h>

#include<erl_nif.h>
#include<msquic.h>

#include "quicAtom.h"
#include "quicAcceptor.h"
#include "quicStatus.h"
#include "quicSettings.h"
#include "quicParam.h"


const QUIC_API_TABLE* MsQuic = NULL;
HQUIC Registration = NULL;
HQUIC DefSrvCfg = NULL;
HQUIC DefCliCfg = NULL;
QUIC_REGISTRATION_CONFIG RegConfig = { "eQuic", QUIC_EXECUTION_PROFILE_LOW_LATENCY };

ErlNifResourceType* CfgResIns = NULL;

typedef struct ListenerRes_r {
	atomic_ullong IndexPtr;
	HQUIC Listener;
	HQUIC Configuration;
	ErlNifPid ListerPid;
	ErlNifMonitor mon;
	ERL_NIF_TERM RefTerm;
} ListenerRes;

typedef struct ConnRes_r {
	HQUIC Connection;
	ErlNifPid* OwnerPid;
	ErlNifMonitor mon;
	ERL_NIF_TERM RefTerm;
	ErlNifEnv* ConnEnv;
} ConnRes;

ErlNifResourceType* ListenerResIns = NULL;
ErlNifResourceType* ConnResIns = NULL;


void cfgResClose(ErlNifEnv* env, void* obj) {
	enif_fprintf(stdout, "IMY*************cfgClose1111\n");
	if (NULL == obj) return;
	HQUIC* ObjIns = (HQUIC*)(obj);

	if (NULL != *ObjIns) {
		MsQuic->ConfigurationClose(*ObjIns);
		*ObjIns = NULL;
	}

	enif_fprintf(stdout, "IMY*************cfgClose222\n");
	return;
}

void listenerResClose(ErlNifEnv* caller_env, void* obj) {
	enif_fprintf(stdout, "IMY*************listenerResClose1 %p\n", obj);
	if (NULL == obj) return;

	ListenerRes* ObjIns = (ListenerRes*)obj;
	enif_fprintf(stdout, "IMY*************listenerResClose2 %p\n", ObjIns->Listener);
	if (NULL != ObjIns->Listener) {
		MsQuic->ListenerClose(ObjIns->Listener);
		ObjIns->Listener = NULL;
	}

	Acceptor* CurPtr = curAcceptor(&ObjIns->IndexPtr);
	enif_fprintf(stdout, "IMY*************listenerResClose2 %p\n", CurPtr);
	if (NULL == CurPtr) return;
	enif_fprintf(stdout, "IMY*************listenerResClose3 %p\n", ObjIns->IndexPtr);
	ErlNifEnv* MsgEnv = enif_alloc_env();
	if (NULL == MsgEnv) {
		clearAcceptors(&ObjIns->IndexPtr);
		return;
	}
	ERL_NIF_TERM CloseMsg;
	Acceptor* TempPtr = CurPtr;
	do {
		if (atomic_load(&TempPtr->IsWork)) {
			atomic_store(&TempPtr->IsWork, false);
			CloseMsg = enif_make_tuple2(MsgEnv, atom_quicLClose, ObjIns->RefTerm);
			enif_send(caller_env, &TempPtr->AcceptorPid, MsgEnv, CloseMsg);
		}
		TempPtr = TempPtr->next;
		enif_clear_env(MsgEnv);
	} while (TempPtr != CurPtr);

	enif_free_env(MsgEnv);
	clearAcceptors(&ObjIns->IndexPtr);
	enif_fprintf(stdout, "IMY*************listenerResClose5 %p\n", ObjIns->IndexPtr);
}

void listenerResDown(ErlNifEnv* caller_env, void* obj, ErlNifPid* pid, ErlNifMonitor* mon) {
	enif_fprintf(stdout, "IMY*************listenerResDown %p %T\n", obj, pid, mon);
	if (NULL == obj) return;

	ListenerRes* ObjIns = (ListenerRes*)obj;
	if (enif_compare_pids(&ObjIns->ListerPid, pid)) {
		MsQuic->ListenerStop(ObjIns->Listener);

		Acceptor* CurPtr = curAcceptor(&ObjIns->IndexPtr);
		if (NULL != CurPtr) {
			ErlNifEnv* MsgEnv = enif_alloc_env();
			if (NULL == MsgEnv) return;

			ERL_NIF_TERM CloseMsg;
			Acceptor* TempPtr = CurPtr;
			do {
				if (atomic_load(&TempPtr->IsWork)) {
					atomic_store(&TempPtr->IsWork, false);
					CloseMsg = enif_make_tuple2(MsgEnv, atom_quicLClose, ObjIns->RefTerm);
					enif_send(caller_env, &TempPtr->AcceptorPid, MsgEnv, CloseMsg);
				}
				TempPtr = TempPtr->next;
				enif_clear_env(MsgEnv);
			} while (TempPtr != CurPtr);
			enif_free_env(MsgEnv);
		}
	}
	else {
		unsetAcceptor(&ObjIns->IndexPtr, *pid);
	}
	return;
}

void ConnResClose(ErlNifEnv* caller_env, void* obj) {
	if (NULL == obj) return;

	ConnRes* ObjIns = (ConnRes*)obj;
	if (NULL != ObjIns->ConnEnv) {
		enif_free_env(ObjIns->ConnEnv);
		ObjIns->ConnEnv = NULL;
	}
	if (NULL != ObjIns->Connection) {
		MsQuic->ConnectionClose(ObjIns->Connection);
		ObjIns->Connection = NULL;
	}
	return;
}

void ConnResDown(ErlNifEnv* caller_env, void* obj, ErlNifPid* pid, ErlNifMonitor* mon) {
	if (NULL == obj) return;

	ConnRes* ObjIns = (ConnRes*)obj;
	if (enif_compare_pids(ObjIns->OwnerPid, pid) && NULL != ObjIns->Connection) {
		MsQuic->ConnectionShutdown(ObjIns->Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, QUIC_STATUS_INTERNAL_ERROR);
	}

	if (0 != enif_compare(ObjIns->RefTerm, atom_undefined)) enif_release_resource(ObjIns);
	return;
}

int nifLoad(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info) {
	enif_fprintf(stdout, "IMY*************nifload00000\n");
	NIF_ATOMS(NIF_ATOM_INIT)


		enif_fprintf(stdout, "IMY*************nifload00001\n");
	*priv_data = NULL;

	int RetCode = 0;
	QUIC_BUFFER* AlpnBuffers = NULL;
	unsigned AlpnSetCnt = 0;

	QUIC_STATUS Status = QUIC_STATUS_SUCCESS;
	enif_fprintf(stdout, "IMY*************nifload1111\n");
	if (QUIC_FAILED(Status = MsQuicOpen2(&MsQuic))) {
		RetCode = -1; goto Failed;
	}
	enif_fprintf(stdout, "IMY*************nifload222\n");
	const ERL_NIF_TERM* tuple;
	int arity;

	if (!enif_get_tuple(env, load_info, &arity, &tuple)) {
		RetCode = -2; goto Failed;
	}

	unsigned int ExecutionProfile = 0;
	if (!enif_get_uint(env, tuple[0], &ExecutionProfile)) {
		RetCode = -3; goto Failed;
	}

	RegConfig.ExecutionProfile = (QUIC_EXECUTION_PROFILE)ExecutionProfile;
	enif_fprintf(stdout, "IMY*************nifload333\n");
	if (QUIC_FAILED(Status = MsQuic->RegistrationOpen(&RegConfig, &Registration))) {
		RetCode = -3; goto Failed;
	}
	enif_fprintf(stdout, "IMY*************nifload444\n");
	ErlNifResourceFlags flags = ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER;
	CfgResIns = enif_open_resource_type(env, NULL, "eCfgResIns", cfgResClose, flags, NULL);
	if (NULL == CfgResIns) {
		RetCode = -4; goto Failed;
	}

	ErlNifResourceTypeInit ListenerInit = { .dtor = listenerResClose, .down = listenerResDown, .stop = NULL };
	ListenerResIns = enif_open_resource_type_x(env, "eListenerResIns", &ListenerInit, flags, NULL);
	if (NULL == ListenerResIns) {
		RetCode = -4; goto Failed;
	}

	ErlNifResourceTypeInit ConnInit = { .dtor = ConnResClose, .down = ConnResDown, .stop = NULL };
	ConnResIns = enif_open_resource_type_x(env, "eConnResIns", &ConnInit, flags, NULL);
	if (NULL == ConnResIns) {
		RetCode = -4; goto Failed;
	}

	// make Alpn
	unsigned AlpnCnt = 0;
	if (!enif_get_list_length(env, tuple[1], &AlpnCnt) || AlpnCnt < 1) {
		RetCode = -5; goto Failed;
	}
	enif_fprintf(stdout, "IMY*************nifload666\n");
	AlpnBuffers = (QUIC_BUFFER*)enif_alloc(sizeof(QUIC_BUFFER) * AlpnCnt);
	if (NULL == AlpnBuffers) {
		RetCode = -5; goto Failed;
	}
	enif_fprintf(stdout, "IMY*************nifload7777\n");
	if (!loadApln(env, tuple[1], AlpnBuffers, &AlpnSetCnt)) {
		RetCode = -5; goto Failed;
	}

	// make Settings
	QUIC_SETTINGS Settings = { 0 };
	if (!createSettings(env, tuple[2], &Settings)) {
		RetCode = -5; goto Failed;
	}
	enif_fprintf(stdout, "IMY*************nifload555\n");

	if (QUIC_FAILED(Status = MsQuic->ConfigurationOpen(Registration, AlpnBuffers, AlpnSetCnt, &Settings, sizeof(Settings), NULL, &DefSrvCfg))) {
		RetCode = -5; goto Failed;
	}

	if (QUIC_FAILED(Status = MsQuic->ConfigurationOpen(Registration, AlpnBuffers, AlpnSetCnt, &Settings, sizeof(Settings), NULL, &DefCliCfg))) {
		RetCode = -5; goto Failed;
	}

	freeApln(AlpnBuffers, AlpnSetCnt);
	AlpnBuffers = NULL;
	AlpnSetCnt = 0;

	enif_fprintf(stdout, "IMY*************createCredentialAndLoadstart000\n");

	// make srv Credential and load
	if (!createCredentialAndLoad(env, MsQuic, DefSrvCfg, tuple[3])) {
		RetCode = -5; goto Failed;
	}

	// make cli Credential and load
	if (!createCredentialAndLoad(env, MsQuic, DefCliCfg, tuple[4])) {
		RetCode = -5; goto Failed;
	}

	return 0;

Failed:
	if (NULL != AlpnBuffers) freeApln(AlpnBuffers, AlpnSetCnt);
	if (NULL != DefSrvCfg) MsQuic->ConfigurationClose(DefSrvCfg);
	if (NULL != DefCliCfg) MsQuic->ConfigurationClose(DefCliCfg);
	if (NULL != Registration) MsQuic->RegistrationClose(Registration);
	if (NULL != MsQuic) MsQuicClose(MsQuic);
	return RetCode;
}

int nifUpgrade(ErlNifEnv* env, void** priv_data, void** old_priv_data, ERL_NIF_TERM load_info) {
	*priv_data = *old_priv_data;
	enif_fprintf(stdout, "IMY*************nifUpgrade %p %T\n", old_priv_data, load_info);
	return 0;
}

void nifUnload(ErlNifEnv* env, void* priv_data) {
	enif_fprintf(stdout, "IMY*************nifUnload00001 %p \n", priv_data);
	enif_fprintf(stdout, "IMY*************nifUnload00002 %p \n", DefSrvCfg);
	enif_fprintf(stdout, "IMY*************nifUnload00002 %p \n", DefCliCfg);
	enif_fprintf(stdout, "IMY*************nifUnload00003 %p\n", Registration);
	enif_fprintf(stdout, "IMY*************nifUnload00004 %p\n", MsQuic);
	enif_fprintf(stdout, "IMY*************nifUnload00005 \n");
	return;
}

ERL_NIF_TERM nifParamCfgs0(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	int len = sizeof(ParamCfg) / sizeof(struct ParamCfg_r);
	ERL_NIF_TERM RetList = enif_make_list(env, 0);
	ERL_NIF_TERM OutTerm;
	for (int i = 0; i < len; i++) {
		OutTerm = enif_make_tuple2(env, enif_make_atom(env, ParamCfg[i].ParamName), enif_make_int(env, i));
		RetList = enif_make_list_cell(env, OutTerm, RetList);
	}
	return RetList;
}

ERL_NIF_TERM nifCfgOpen3(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	ERL_NIF_TERM AlpnList = argv[0];
	ERL_NIF_TERM SettingsMap = argv[1];
	ERL_NIF_TERM CredentialMap = argv[2];
	enif_fprintf(stdout, "IMY*************nifCfgOpen3000\n");
	if (!enif_is_list(env, AlpnList) || !enif_is_map(env, SettingsMap) || !enif_is_map(env, CredentialMap)) return enif_make_badarg(env);

	ERL_NIF_TERM ErrTerm;
	QUIC_BUFFER* AlpnBuffers = NULL;
	unsigned AlpnSetCnt = 0;
	HQUIC* ObjIns = NULL;

	// make Alpn
	unsigned AlpnCnt = 0;
	if (!enif_get_list_length(env, AlpnList, &AlpnCnt) || AlpnCnt < 1) {
		ErrTerm = ErrMsg(env, "getAlpnCnt");
		goto Failed;
	}

	enif_fprintf(stdout, "IMY*************nifCfgOpen33333\n");
	AlpnBuffers = (QUIC_BUFFER*)enif_alloc(sizeof(QUIC_BUFFER) * AlpnCnt);
	if (NULL == AlpnBuffers) {
		ErrTerm = ErrMsg(env, "allocAlpnBuffers");
		goto Failed;
	}
	enif_fprintf(stdout, "IMY*************nifCfgOpen344444 %d %T\n", AlpnCnt, AlpnList);
	if (!loadApln(env, AlpnList, AlpnBuffers, &AlpnSetCnt)) {
		enif_fprintf(stdout, "IMY*************nifCfgOpen3444445555555 %d \n", AlpnSetCnt);
		ErrTerm = ErrMsg(env, "loadApln");
		goto Failed;
	}

	// make Settings
	QUIC_SETTINGS Settings = { 0 };
	if (!createSettings(env, SettingsMap, &Settings)) {
		ErrTerm = ErrMsg(env, "createSettings");
		goto Failed;
	}
	enif_fprintf(stdout, "IMY*************nifCfgOpen31111\n");

	enif_fprintf(stdout, "IMY*************nifCfgOpen3555555\n");
	// open config
	ObjIns = (HQUIC*)(enif_alloc_resource(CfgResIns, sizeof(HQUIC)));
	if (NULL == ObjIns) {
		ErrTerm = ErrMsg(env, "allocObjIns");
		goto Failed;
	}
	*ObjIns = NULL;

	enif_fprintf(stdout, "IMY*************nifCfgOpen36666\n");
	QUIC_STATUS Status = QUIC_STATUS_SUCCESS;
	if (QUIC_FAILED(Status = MsQuic->ConfigurationOpen(Registration, AlpnBuffers, AlpnCnt, &Settings, sizeof(Settings), NULL, ObjIns))) {
		ErrTerm = ErrMsg(env, "ConfigurationOpen");
		goto Failed;
	}
	enif_fprintf(stdout, "IMY*************nifCfgOpen37777  %p \n", *ObjIns);

	// make Credential and load
	if (!createCredentialAndLoad(env, MsQuic, *ObjIns, CredentialMap)) {
		ErrTerm = ErrMsg(env, "createCredentialAndLoad");
		goto Failed;
	}
	enif_fprintf(stdout, "IMY*************nifCfgOpen38888\n");
	freeApln(AlpnBuffers, AlpnSetCnt);
	ERL_NIF_TERM RefTerm = enif_make_resource(env, ObjIns);
	enif_release_resource(ObjIns);
	enif_fprintf(stdout, "IMY*************nifCfgOpen39999\n");
	return enif_make_tuple2(env, atom_ok, RefTerm);

Failed:
	enif_fprintf(stdout, "IMY*************nifCfgOpen3failed %p %p %p %d\n", AlpnBuffers, ObjIns, AlpnSetCnt);

	if (NULL != AlpnBuffers) freeApln(AlpnBuffers, AlpnSetCnt);

	if (NULL != ObjIns) {
		if (NULL != *ObjIns) {
			MsQuic->ConfigurationClose(*ObjIns);
			*ObjIns = NULL;
		};
		enif_release_resource(ObjIns);
	}
	return enif_make_tuple2(env, atom_error, ErrTerm);
}

QUIC_STATUS StreamHandler(HQUIC Stream, void* Context, QUIC_STREAM_EVENT* Event) {
	// QuicerStreamCTX* s_ctx = (QuicerStreamCTX*)Context;
	// ErlNifEnv* env = s_ctx->env;
	// ERL_NIF_TERM report;
	// QUIC_STATUS status = QUIC_STATUS_SUCCESS;
	// QuicerStreamSendCTX* send_ctx = NULL;
	// BOOLEAN is_destroy = FALSE;
	// 
	// enif_mutex_lock(s_ctx->lock);
	// 
	// TP_CB_3(event, (uintptr_t)Stream, Event->Type);
	// switch (Event->Type)
	// {
	// case QUIC_STREAM_EVENT_SEND_COMPLETE:
	//     //
	//     // A previous StreamSend call has completed, and the context is being
	//     // returned back to the app.
	//     //
	//     //
	//     send_ctx = (QuicerStreamSendCTX*)(Event->SEND_COMPLETE.ClientContext);
	// 
	//     if (!send_ctx)
	//     {
	//         status = QUIC_STATUS_INVALID_STATE;
	//         break;
	//     }
	// 
	//     if (send_ctx->is_sync)
	//     {
	//         report = enif_make_tuple4(
	//             env,
	//             ATOM_QUIC,
	//             ATOM_SEND_COMPLETE,
	//             enif_make_copy(env, s_ctx->eHandler),
	//             enif_make_uint64(env, Event->SEND_COMPLETE.Canceled));
	// 
	//         // note, report to caller instead of stream owner
	//         if (!enif_send(NULL, &send_ctx->caller, NULL, report))
	//         {
	//             // Owner is gone, we shutdown the stream as well.
	//             TP_CB_3(owner_die, (uintptr_t)Stream, Event->Type);
	//             MsQuic->StreamShutdown(
	//                 Stream, QUIC_STREAM_SHUTDOWN_FLAG_GRACEFUL, 0);
	//             // @todo return proper bad status
	//         }
	//     }
	// 
	//     destroy_send_ctx(send_ctx);
	//     break;
	// case QUIC_STREAM_EVENT_RECEIVE:
	//     //
	//     // Data was received from the peer on the stream.
	//     //
	//     status = handle_stream_recv_event(Stream, s_ctx, Event);
	//     break;
	// case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
	//     //
	//     // The peer gracefully shut down its send direction of the stream.
	//     //
	//     report = enif_make_tuple3(env,
	//         ATOM_QUIC,
	//         ATOM_PEER_SEND_SHUTDOWN,
	//         enif_make_copy(env, s_ctx->eHandler));
	// 
	//     if (!enif_send(NULL, &s_ctx->owner->Pid, NULL, report))
	//     {
	//         // App down, close it.
	//         TP_CB_3(app_down, (uintptr_t)Stream, Event->Type);
	//         MsQuic->StreamShutdown(Stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
	//     }
	//     break;
	// case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
	//     //
	//     // The peer aborted its send direction of the stream.
	//     //
	//     TP_CB_3(peer_send_aborted,
	//         (uintptr_t)Stream,
	//         Event->PEER_SEND_ABORTED.ErrorCode);
	//     report = enif_make_tuple4(
	//         env,
	//         ATOM_QUIC,
	//         ATOM_PEER_SEND_ABORTED,
	//         enif_make_copy(env, s_ctx->eHandler),
	//         enif_make_uint64(env, Event->PEER_SEND_ABORTED.ErrorCode));
	// 
	//     if (!enif_send(NULL, &s_ctx->owner->Pid, NULL, report))
	//     {
	//         // Owner is gone, we shutdown the stream as well.
	//         TP_CB_3(app_down, (uintptr_t)Stream, Event->Type);
	//         MsQuic->StreamShutdown(
	//             Stream, QUIC_STREAM_SHUTDOWN_FLAG_GRACEFUL, 0);
	//         // @todo return proper bad status
	//     }
	//     break;
	// case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
	//     //
	//     // Both directions of the stream have been shut down and MsQuic is done
	//     // with the stream. It can now be safely cleaned up.
	//     //
	//     // we don't use trylock since we are in callback context
	//     report = enif_make_tuple4(
	//         env,
	//         ATOM_QUIC,
	//         ATOM_CLOSED,
	//         enif_make_copy(env, s_ctx->eHandler),
	//         enif_make_uint64(env, Event->SHUTDOWN_COMPLETE.ConnectionShutdown));
	// 
	//     enif_send(NULL, &s_ctx->owner->Pid, NULL, report);
	// 
	//     is_destroy = TRUE;
	//     break;
	// default:
	//     break;
	// }
	// 
	// enif_clear_env(env);
	// 
	// if (is_destroy)
	// {
	//     s_ctx->is_closed = TRUE;
	// }
	// 
	// enif_mutex_unlock(s_ctx->lock);
	// 
	// if (is_destroy)
	// {
	//     // must be called after mutex unlock
	//     destroy_s_ctx(s_ctx);
	// }
	// return status;
	return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS ConnHandler(HQUIC Connection, void* Context, QUIC_CONNECTION_EVENT* Event) {
	ConnRes* ConnObjIns = (ConnRes*)Context;
	ErlNifEnv* MsgEnv = ConnObjIns->ConnEnv;
	QUIC_STATUS status = QUIC_STATUS_SUCCESS;
	if (NULL == MsgEnv) {
		status = QUIC_STATUS_INVALID_STATE;
		goto Failed;
	}
	switch (Event->Type) {
	case QUIC_CONNECTION_EVENT_CONNECTED: {
		// 握手完毕
		ConnObjIns->RefTerm = enif_make_resource(MsgEnv, ConnObjIns);
		enif_release_resource(ConnObjIns);
		ERL_NIF_TERM NewConnMsg = enif_make_tuple2(MsgEnv, atom_quicCNew, ConnObjIns->RefTerm);
		enif_send(NULL, ConnObjIns->OwnerPid, MsgEnv, NewConnMsg);
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT: {
		// 每当传输（例如 QUIC 层）确定连接已终止时，都会传递此事件。这可能由于多种不同的原因而发生。有些如下。
		// 	握手失败（任何原因）。
		// 	连接空闲足够长的时间。
		// 	连接断开（与对等方失去联系；没有确认）。
		// 	连接遇到协议冲突。
		ERL_NIF_TERM ErrorCode = enif_make_uint64(MsgEnv, Event->SHUTDOWN_INITIATED_BY_TRANSPORT.ErrorCode);
		ERL_NIF_TERM Status = enif_make_long(MsgEnv, Event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status);
		ERL_NIF_TERM StopMsg = enif_make_tuple3(MsgEnv, atom_quicCStop, ConnObjIns->RefTerm, enif_make_tuple3(MsgEnv, atom_stopByTransport, ErrorCode, Status));
		enif_send(NULL, ConnObjIns->OwnerPid, MsgEnv, StopMsg);
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER: {
		// 此事件在对等应用程序终止应用程序时传递，并带有应用程序特定的 62 位错误代码。
		ERL_NIF_TERM ErrorCode = enif_make_uint64(MsgEnv, Event->SHUTDOWN_INITIATED_BY_PEER.ErrorCode);
		ERL_NIF_TERM StopMsg = enif_make_tuple3(MsgEnv, atom_quicCStop, ConnObjIns->RefTerm, enif_make_tuple2(MsgEnv, atom_stopByPeer, ErrorCode));
		enif_send(NULL, ConnObjIns->OwnerPid, MsgEnv, StopMsg);
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE: {
		// 此事件是传递给应用程序的最后一个事件，表明连接现在可以安全关闭。
		if (!Event->SHUTDOWN_COMPLETE.AppCloseInProgress && NULL != ConnObjIns->Connection) {
			MsQuic->ConnectionClose(ConnObjIns->Connection);
			ConnObjIns->Connection = NULL;
		}

		ERL_NIF_TERM HandshakeCompleted = atom_false;
		if (Event->SHUTDOWN_COMPLETE.HandshakeCompleted) HandshakeCompleted = atom_true;
		ERL_NIF_TERM PeerAcknowledgedShutdown = atom_false;
		if (Event->SHUTDOWN_COMPLETE.PeerAcknowledgedShutdown) PeerAcknowledgedShutdown = atom_true;
		ERL_NIF_TERM CloseMsg = enif_make_tuple3(MsgEnv, atom_quicCClose, ConnObjIns->RefTerm, enif_make_tuple2(MsgEnv, HandshakeCompleted, PeerAcknowledgedShutdown));
		enif_send(NULL, ConnObjIns->OwnerPid, MsgEnv, CloseMsg);
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_LOCAL_ADDRESS_CHANGED: {
		// 当用于主要 / 活动路径通信的本地地址发生更改时，将传递此事件。
		ERL_NIF_TERM InfoTerm = AddrToTerm(MsgEnv, Event->LOCAL_ADDRESS_CHANGED.Address);
		ERL_NIF_TERM InfoMsg = enif_make_tuple3(MsgEnv, atom_quicCInfo, ConnObjIns->RefTerm, enif_make_tuple2(MsgEnv, atom_localAddrChanged, InfoTerm));
		enif_send(NULL, ConnObjIns->OwnerPid, MsgEnv, InfoMsg);
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_PEER_ADDRESS_CHANGED: {
		// 当用于主要/活动路径通信的远程地址发生更改时，将传递此事件。
		ERL_NIF_TERM InfoTerm = AddrToTerm(MsgEnv, Event->PEER_ADDRESS_CHANGED.Address);
		ERL_NIF_TERM InfoMsg = enif_make_tuple3(MsgEnv, atom_quicCInfo, ConnObjIns->RefTerm, enif_make_tuple2(MsgEnv, atom_peerAddrChanged, InfoTerm));
		enif_send(NULL, ConnObjIns->OwnerPid, MsgEnv, InfoMsg);
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED:
		// 当对等方创建新流时，将传递此事件。
		break;
	case QUIC_CONNECTION_EVENT_STREAMS_AVAILABLE: {
		// 此事件指示对等方愿意接受的流数已更改。
		ERL_NIF_TERM InfoTerm = StreamsAvailable(MsgEnv, Event);
		ERL_NIF_TERM InfoMsg = enif_make_tuple3(MsgEnv, atom_quicCInfo, ConnObjIns->RefTerm, enif_make_tuple2(MsgEnv, atom_streamsAvailable, InfoTerm));
		enif_send(NULL, ConnObjIns->OwnerPid, MsgEnv, InfoMsg);
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_PEER_NEEDS_STREAMS: {
		// 此事件表示对等方当前在应用程序配置的愿意接受的并行流数量上被阻止。
		ERL_NIF_TERM InfoMsg = enif_make_tuple3(MsgEnv, atom_quicCInfo, ConnObjIns->RefTerm, atom_peerNeedsStreams);
		enif_send(NULL, ConnObjIns->OwnerPid, MsgEnv, InfoMsg);
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_IDEAL_PROCESSOR_CHANGED: {
		// 此事件指示 MsQuic 已确定最适合处理给定连接的处理器或 CPU。
		ERL_NIF_TERM InfoTerm = IdealProcessorChanged(MsgEnv, Event);
		ERL_NIF_TERM InfoMsg = enif_make_tuple3(MsgEnv, atom_quicCInfo, ConnObjIns->RefTerm, enif_make_tuple2(MsgEnv, atom_idealProcessorChanged, InfoTerm));
		enif_send(NULL, ConnObjIns->OwnerPid, MsgEnv, InfoMsg);
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_DATAGRAM_STATE_CHANGED: {
		// 此事件指示发送不可靠数据报的当前状态已更改。
		ERL_NIF_TERM InfoTerm = DatagramStateChanged(MsgEnv, Event);
		ERL_NIF_TERM InfoMsg = enif_make_tuple3(MsgEnv, atom_quicCInfo, ConnObjIns->RefTerm, enif_make_tuple2(MsgEnv, atom_datagramStateChanged, InfoTerm));
		enif_send(NULL, ConnObjIns->OwnerPid, MsgEnv, InfoMsg);
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_DATAGRAM_RECEIVED: {
		// 此事件指示从对等方接收到的不可靠数据报。
		ERL_NIF_TERM Datagam;
		memcpy(enif_make_new_binary(MsgEnv, Event->DATAGRAM_RECEIVED.Buffer->Length, &Datagam), Event->DATAGRAM_RECEIVED.Buffer->Buffer, Event->DATAGRAM_RECEIVED.Buffer->Length);
		ERL_NIF_TERM InfoMsg = enif_make_tuple3(MsgEnv, atom_datagamRecv, ConnObjIns->RefTerm, Datagam);
		enif_send(NULL, ConnObjIns->OwnerPid, MsgEnv, InfoMsg);
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_DATAGRAM_SEND_STATE_CHANGED: {
		// 此事件指示先前通过DatagramSend发送的不可靠数据报的状态更改。
		ERL_NIF_TERM InfoTerm = DatagramSendStateChanged(MsgEnv, Event);
		ERL_NIF_TERM InfoMsg = enif_make_tuple3(MsgEnv, atom_quicCInfo, ConnObjIns->RefTerm, enif_make_tuple2(MsgEnv, atom_datagramSendStateChanged, InfoTerm));
		enif_send(NULL, ConnObjIns->OwnerPid, MsgEnv, InfoMsg);
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_RESUMED: {
		// 此事件表示之前的会话已在 TLS 层成功恢复。此事件仅针对服务器端传递。服务器应用程序必须通过从事件返回成功或失败状态代码来指示接受或拒绝恢复票证。如果被服务器应用程序拒绝，则拒绝恢复并执行正常的握手。
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_RESUMPTION_TICKET_RECEIVED: {
		// 此事件表示已从服务器接收到 TLS 恢复票证。  此事件仅针对客户端传递
		ERL_NIF_TERM InfoTerm = ResumptionTicketReceived(MsgEnv, Event);
		ERL_NIF_TERM InfoMsg = enif_make_tuple3(MsgEnv, atom_quicCInfo, ConnObjIns->RefTerm, enif_make_tuple2(MsgEnv, atom_resumptionTicketReceived, InfoTerm));
		enif_send(NULL, ConnObjIns->OwnerPid, MsgEnv, InfoMsg);
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_PEER_CERTIFICATE_RECEIVED:
	{
		// 此事件表示已从对等方收到证书。
		ERL_NIF_TERM InfoTerm = PeerCertificateReceived(MsgEnv, Event);
		ERL_NIF_TERM InfoMsg = enif_make_tuple3(MsgEnv, atom_quicCInfo, ConnObjIns->RefTerm, enif_make_tuple2(MsgEnv, atom_peerCertificateReceived, InfoTerm));
		enif_send(NULL, ConnObjIns->OwnerPid, MsgEnv, InfoMsg);
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	default:
		break;
	}

	enif_clear_env(MsgEnv);
	return QUIC_STATUS_SUCCESS;

Failed:
	// do some thing
	enif_clear_env(MsgEnv);
	return status;
}

QUIC_STATUS ListenerHandler(HQUIC Listener, void* Context, QUIC_LISTENER_EVENT* Event) {
	QUIC_STATUS Status = QUIC_STATUS_SUCCESS;
	ConnRes* ObjIns = NULL;

	switch (Event->Type) {
	case QUIC_LISTENER_EVENT_NEW_CONNECTION: {
		ObjIns = (ConnRes*)(enif_alloc_resource(ConnResIns, sizeof(ConnRes)));
		if (NULL == ObjIns) {
			Status = QUIC_STATUS_OUT_OF_MEMORY;
			goto Failed;
		}

		ObjIns->Connection = NULL;

		ObjIns->ConnEnv = enif_alloc_env();
		if (NULL == ObjIns->ConnEnv) {
			Status = QUIC_STATUS_INVALID_STATE;
			goto Failed;
		}

		ObjIns->RefTerm = atom_undefined;


		ErlNifPid* OwnerPid = nextAcceptor(&((ListenerRes*)Context)->IndexPtr);

		if (NULL == OwnerPid) {
			Status = QUIC_STATUS_INVALID_STATE;
			goto Failed;
		}

		MsQuic->SetCallbackHandler(Event->NEW_CONNECTION.Connection, (void*)ConnHandler, ObjIns);

		if (QUIC_FAILED(Status = MsQuic->ConnectionSetConfiguration(Event->NEW_CONNECTION.Connection, ((ListenerRes*)Context)->Configuration))) {
			goto Failed;
		}
		ObjIns->OwnerPid = OwnerPid;
		if (0 != enif_monitor_process(NULL, ObjIns, OwnerPid, &ObjIns->mon)) {
			Status = QUIC_STATUS_ABORTED;
			goto Failed;
		}
		ObjIns->Connection = Event->NEW_CONNECTION.Connection;
		break;
	}
	case QUIC_LISTENER_EVENT_STOP_COMPLETE:
		break;
	default:
		break;
	}
	return QUIC_STATUS_SUCCESS;

Failed:
	if (NULL != ObjIns) {
		if (NULL != ObjIns->ConnEnv) {
			enif_free_env(ObjIns->ConnEnv);
			ObjIns->ConnEnv = NULL;
		};
		ObjIns->Connection = NULL;
		ObjIns->RefTerm = atom_freed;
		enif_release_resource(ObjIns);
	}

	return Status;
}

ERL_NIF_TERM nifListenerStart3(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	ERL_NIF_TERM ListenAddr = argv[0];
	ERL_NIF_TERM ListenPort = argv[1];
	ERL_NIF_TERM ListenOpts = argv[2];

	if (!enif_is_list(env, ListenAddr) || !enif_is_number(env, ListenPort) || !enif_is_map(env, ListenOpts)) return enif_make_badarg(env);

	enif_fprintf(stdout, "IMY*************nifListenerStart3 000  %T %T %T\n", ListenAddr, ListenPort, ListenOpts);
	ERL_NIF_TERM ErrTerm;
	ListenerRes* ObjIns = NULL;
	QUIC_BUFFER* AlpnBuffers = NULL;
	unsigned AlpnSetCnt = 0;
	enif_fprintf(stdout, "IMY*************nifListenerStart3 111 %p %d\n", ListenerResIns, sizeof(ListenerRes));
	// open config
	ObjIns = (ListenerRes*)(enif_alloc_resource(ListenerResIns, sizeof(ListenerRes)));
	enif_fprintf(stdout, "IMY*************nifListenerStart32 111\n");
	if (NULL == ObjIns) {
		ErrTerm = ErrMsg(env, "allocObjIns");
		goto Failed;
	}
	// do some init
	atomic_store(&ObjIns->IndexPtr, 0);
	ObjIns->Listener = NULL;
	ObjIns->Configuration = NULL;

	if (NULL == enif_self(env, &ObjIns->ListerPid)) {
		ErrTerm = ErrMsg(env, "getSelfPid");
		goto Failed;
	}

	enif_fprintf(stdout, "IMY*************nifListenerStart3 2222\n");

	enif_fprintf(stdout, "IMY*************nifListenerStart3 333\n");
	// start listener
	unsigned UdpPort;
	if (!enif_get_uint(env, ListenPort, &UdpPort)) {
		ErrTerm = ErrMsg(env, "ListenPort");
		goto Failed;
	}
	enif_fprintf(stdout, "IMY*************nifListenerStart3 4444\n");
	QUIC_ADDR Address = { 0 };
	if (enif_is_empty_list(env, ListenAddr)) {
		QuicAddrSetFamily(&Address, QUIC_ADDRESS_FAMILY_UNSPEC);
		QuicAddrSetPort(&Address, (uint16_t)UdpPort);
	}
	else {
		char* AddrPtr = NULL;
		AddrPtr = getStrFromTrem(env, ListenAddr);
		if (NULL == AddrPtr) {
			ErrTerm = ErrMsg(env, "ListenAddr");
			goto Failed;
		}
		if (!QuicAddrFromString(AddrPtr, (uint16_t)UdpPort, &Address)) {
			enif_free(AddrPtr);
			ErrTerm = ErrMsg(env, "QuicAddrFromString");
			goto Failed;
		}
		enif_free(AddrPtr);
		AddrPtr = NULL;
	}
	ERL_NIF_TERM AlpnList;
	if (!enif_get_map_value(env, ListenOpts, atom_eAlpn, &AlpnList)) {
		ErrTerm = ErrMsg(env, "AlpnList");
		goto Failed;
	}
	enif_fprintf(stdout, "IMY*************nifListenerStart3 5555\n");
	// make Alpn
	unsigned AlpnCnt = 0;
	if (!enif_get_list_length(env, AlpnList, &AlpnCnt) || AlpnCnt < 1) {
		ErrTerm = ErrMsg(env, "AlpnCnt");
		goto Failed;
	}

	if (AlpnCnt <= 0) {
		ErrTerm = ErrMsg(env, "AlpnCnt=0");
		goto Failed;
	}

	AlpnBuffers = (QUIC_BUFFER*)enif_alloc(sizeof(QUIC_BUFFER) * AlpnCnt);

	if (!loadApln(env, AlpnList, AlpnBuffers, &AlpnSetCnt)) {
		ErrTerm = ErrMsg(env, "loadApln");
		goto Failed;
	}

	// set ObjIns fields
	ERL_NIF_TERM RefTerm;
	if (!enif_get_map_value(env, ListenOpts, atom_eCfgRef, &RefTerm)) {
		ObjIns->Configuration = DefSrvCfg;
	}
	else {
		HQUIC* CfgRes = NULL;
		if (!enif_get_resource(env, RefTerm, CfgResIns, (void**)&CfgRes)) {
			ErrTerm = ErrMsg(env, "getCfgResObj");
			goto Failed;
		}
		ObjIns->Configuration = *CfgRes;
	}
	enif_fprintf(stdout, "IMY*************nifListenerStart3 666611\n");

	enif_fprintf(stdout, "IMY*************nifListenerStart3 6666\n");
	// init ObjIns AcceptWorkers
	uint64_t AcceptorCnt = 0;
	if (!getUint64FromMap(env, ListenOpts, atom_eAcceptorCnt, &AcceptorCnt)) {
		ErrTerm = ErrMsg(env, "AcceptorCnt");
		goto Failed;
	}

	if (!initAcceptors(&ObjIns->IndexPtr, AcceptorCnt)) {
		ErrTerm = ErrMsg(env, "initAcceptors");
		goto Failed;
	}

	// open listener
	QUIC_STATUS Status = QUIC_STATUS_SUCCESS;
	if (QUIC_FAILED(Status = MsQuic->ListenerOpen(Registration, ListenerHandler, ObjIns, &ObjIns->Listener))) {
		ErrTerm = StatusMsg(env, Status);
		goto Failed;
	}

	if (QUIC_FAILED(Status = MsQuic->ListenerStart(ObjIns->Listener, AlpnBuffers, AlpnCnt, &Address))) {
		enif_fprintf(stdout, "IMY*************nifListenerStart3 888 %d\n", Status);
		ErrTerm = StatusMsg(env, Status);
		goto Failed;
	}

	freeApln(AlpnBuffers, AlpnSetCnt);
	AlpnBuffers = NULL;
	AlpnSetCnt = 0;

	enif_fprintf(stdout, "IMY*************nifListenerStart3 666622 %T\n", ObjIns->ListerPid);
	if (0 != enif_monitor_process(env, ObjIns, &ObjIns->ListerPid, &ObjIns->mon)) {
		ErrTerm = ErrMsg(env, "MonitorObj");
		goto Failed;
	}

	RefTerm = enif_make_resource(env, ObjIns);
	ObjIns->RefTerm = RefTerm;
	enif_fprintf(stdout, "IMY*************nifListenerStart3 888\n");
	enif_release_resource(ObjIns);

	return enif_make_tuple2(env, atom_ok, RefTerm);

Failed:
	enif_fprintf(stdout, "IMY*************nifListenerStart3 Failed1  %p %d \n", AlpnBuffers, AlpnSetCnt);
	if (NULL != AlpnBuffers) freeApln(AlpnBuffers, AlpnSetCnt);
	enif_fprintf(stdout, "IMY*************nifListenerStart3 Failed2  %p \n", ObjIns);
	if (NULL != ObjIns) {
		clearAcceptors(&ObjIns->IndexPtr);
		enif_fprintf(stdout, "IMY*************nifListenerStart3 Failed3  %p %T %T \n", ObjIns->Listener, atom_error, ErrTerm);
		if (NULL != ObjIns->Listener) {
			MsQuic->ListenerClose(ObjIns->Listener);
			ObjIns->Listener = NULL;
		}
		enif_release_resource(ObjIns);
	}

	return enif_make_tuple2(env, atom_error, ErrTerm);
}

ERL_NIF_TERM nifListenerAccept2(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	ListenerRes* ObjIns = NULL;
	if (!enif_get_resource(env, argv[0], ListenerResIns, (void**)&ObjIns)) return enif_make_tuple2(env, atom_error, argv[0]);

	ErlNifPid SelfPid;
	if (NULL == enif_self(env, &SelfPid)) 	return enif_make_tuple2(env, atom_error, argv[0]);

	if (0 != enif_monitor_process(env, ObjIns, &SelfPid, NULL)) return enif_make_tuple2(env, atom_error, argv[0]);

	uint64_t Id;
	if (!enif_get_uint64(env, argv[1], &Id)) return enif_make_tuple2(env, atom_error, argv[1]);

	if (setAcceptor(&ObjIns->IndexPtr, Id, SelfPid)) {
		return atom_ok;
	}
	else {
		return enif_make_tuple2(env, atom_error, argv[1]);
	}
}

ERL_NIF_TERM nifListenerClose1(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	ListenerRes* ObjIns = NULL;
	if (!enif_get_resource(env, argv[0], ListenerResIns, (void**)&ObjIns)) {
		return enif_make_tuple2(env, atom_error, argv[0]);
	}

	MsQuic->ListenerStop(ObjIns->Listener);
	ObjIns->Listener = NULL;
	enif_demonitor_process(env, ObjIns, &ObjIns->mon);

	Acceptor* CurPtr = curAcceptor(&ObjIns->IndexPtr);
	if (NULL == CurPtr) return atom_ok;

	ErlNifEnv* MsgEnv = enif_alloc_env();
	if (NULL == MsgEnv) {
		return atom_ok;
	}
	ERL_NIF_TERM CloseMsg;
	Acceptor* TempPtr = CurPtr;
	do {
		if (atomic_load(&TempPtr->IsWork)) {
			atomic_store(&TempPtr->IsWork, false);
			CloseMsg = enif_make_tuple2(MsgEnv, atom_quicLClose, ObjIns->RefTerm);
			enif_send(env, &TempPtr->AcceptorPid, MsgEnv, CloseMsg);
		}
		TempPtr = TempPtr->next;
		enif_clear_env(MsgEnv);
	} while (TempPtr != CurPtr);

	enif_free_env(MsgEnv);
	return atom_ok;
}

ERL_NIF_TERM nifConnOpen3(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	return atom_ok;
}

ERL_NIF_TERM nifConnClose1(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	return atom_ok;
}

ERL_NIF_TERM test1(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	enif_fprintf(stdout, "IMY*************test11111\n");
	// make Alpn
	unsigned AlpnCnt = 0;
	if (!enif_get_list_length(env, argv[0], &AlpnCnt) || AlpnCnt < 1) {
		return ErrMsg(env, "enif_get_list_length");
	}

	if (AlpnCnt <= 0) {
		return ErrMsg(env, "AlpnCnt");
	}
	enif_fprintf(stdout, "IMY*************test11111\n");
	QUIC_BUFFER* AlpnBuffers = (QUIC_BUFFER*)enif_alloc(sizeof(QUIC_BUFFER) * AlpnCnt);
	if (NULL == AlpnBuffers) {
		return ErrMsg(env, "AlpnBuffers");
	}
	enif_fprintf(stdout, "IMY*************test144444 %d %T\n", AlpnCnt, argv[0]);
	if (!loadApln(env, argv[0], AlpnBuffers, &AlpnCnt)) {
		enif_fprintf(stdout, "IMY*************test1loadApln %d \n", AlpnCnt);
		return ErrMsg(env, "loadApln");
	}

	enif_fprintf(stdout, "IMY*************test155555550000 %d \n", AlpnCnt);

	for (unsigned i = 0; i < AlpnCnt; ++i) {
		enif_fprintf(stdout, "IMY*************test55551111 %d %s \n", AlpnBuffers[i].Length, AlpnBuffers[i].Buffer);
	}

	freeApln(AlpnBuffers, AlpnCnt);
	return atom_ok;
}

static ErlNifFunc nifFuns[] = {
		{"paramCfgs", 0, nifParamCfgs0},
		{"cfgOpen", 3, nifCfgOpen3},
		{"listenerStart", 3, nifListenerStart3},
		{"listenerAccept", 2, nifListenerAccept2},
		{"listenerClose", 1, nifListenerClose1},

	{"connOpen", 3, nifConnOpen3},
	{"connClose", 1, nifConnClose1},


		/* for DEBUG */
		//{ "getConnId", 1, nifGetConnId1, ERL_NIF_DIRTY_JOB_CPU_BOUND},
		//{ "getStreamId", 1, nigGetStreamId1, ERL_NIF_DIRTY_JOB_CPU_BOUND}
	{"test", 1, test1},

};

ERL_NIF_INIT(eQuic, nifFuns, nifLoad, NULL, nifUpgrade, nifUnload)