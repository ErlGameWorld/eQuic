#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

#include<erl_nif.h>
#include<msquic.h>

#include "quicAtom.h"
#include "quicLList.h"
#include "quicStatus.h"
#include "quicSettings.h"
#include "quicParam.h"


const QUIC_API_TABLE *MsQuic = NULL;
HQUIC Registration = NULL;
HQUIC DefSrvCfg = NULL;
HQUIC DefCliCfg = NULL;
QUIC_REGISTRATION_CONFIG RegConfig = {"eQuic", QUIC_EXECUTION_PROFILE_LOW_LATENCY};

ErlNifResourceType *CfgResIns = NULL;

typedef struct ListenerRes_r {
    LLNode* AcceptWorkers;
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
    HQUIC* ObjIns = (HQUIC*)(obj);
    MsQuic->ConfigurationClose(*ObjIns);
    enif_fprintf(stdout, "IMY*************cfgClose222\n");
    return;
}

bool CompareAWPid(void* Data, void *Key) {
    return 0 == enif_compare_pids((ErlNifPid*)Data, (ErlNifPid*)Key);
}

void AWPidFree(void* Data) {
    enif_free(Data);
    return;
}

void listenerResClose(ErlNifEnv* caller_env, void* obj) {
    ListenerRes* ObjIns = (ListenerRes*)obj;
    MsQuic->ListenerClose(ObjIns->Listener);

    ErlNifPid* CurAWPid = (ErlNifPid*)nextLList(&ObjIns->AcceptWorkers);
    if (NULL != CurAWPid) {
        ErlNifEnv* MsgEnv = enif_alloc_env();
        if (NULL == MsgEnv) {
            clearLList(&ObjIns->AcceptWorkers, AWPidFree);
            return;
        }
        ERL_NIF_TERM CloseMsg;
        ErlNifPid* TempAWPid = CurAWPid;
        do {
            CloseMsg = enif_make_tuple2(MsgEnv, atom_quicLClose, ObjIns->RefTerm);
            enif_send(caller_env, TempAWPid, MsgEnv, CloseMsg);
            TempAWPid = (ErlNifPid*)nextLList(&ObjIns->AcceptWorkers);
            enif_clear_env(MsgEnv);
        } while (TempAWPid != CurAWPid);
        enif_free_env(MsgEnv);
        clearLList(&ObjIns->AcceptWorkers, AWPidFree);
    }
}

void listenerResDown(ErlNifEnv* caller_env, void* obj, ErlNifPid* pid, ErlNifMonitor* mon) {
    ListenerRes* ObjIns = (ListenerRes*)obj;
    if (0 == enif_compare_pids(&(ObjIns->ListerPid), pid)) {
        MsQuic->ListenerClose(ObjIns->Listener);

        ErlNifPid* CurAWPid = (ErlNifPid*)nextLList(&ObjIns->AcceptWorkers);
        if (NULL != CurAWPid) {
            ErlNifEnv* MsgEnv = enif_alloc_env();
            if (NULL == MsgEnv) {
                clearLList(&ObjIns->AcceptWorkers, AWPidFree);
                return;
            }
            ERL_NIF_TERM CloseMsg;
            ErlNifPid* TempAWPid = CurAWPid;
            do {
                CloseMsg = enif_make_tuple2(MsgEnv, atom_quicLClose, ObjIns->RefTerm);
                enif_send(caller_env, TempAWPid, MsgEnv, CloseMsg);
                TempAWPid = (ErlNifPid*)nextLList(&ObjIns->AcceptWorkers);
                enif_clear_env(MsgEnv);
            } while (TempAWPid != CurAWPid);
            enif_free_env(MsgEnv);
            clearLList(&ObjIns->AcceptWorkers, AWPidFree);
        }
    }else{
        void* FreePid = delLList(&ObjIns->AcceptWorkers, pid, CompareAWPid);
        if (NULL != FreePid) enif_free(FreePid);
        return;
    }
}

void ConnResClose(ErlNifEnv* caller_env, void* obj) {
    ConnRes* ObjIns = (ConnRes*)obj;
    enif_free_env(ObjIns->ConnEnv);
    MsQuic->ConnectionClose(ObjIns->Connection);
    enif_release_resource(ObjIns);
    return ;
}

void ConnResDown(ErlNifEnv* caller_env, void* obj, ErlNifPid* pid, ErlNifMonitor* mon) {
    ConnRes* ObjIns = (ConnRes*)obj;
    if (0 == enif_compare_pids(ObjIns->OwnerPid, pid)) {
        MsQuic->ConnectionShutdown(ObjIns->Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
    }
    return ;
}

int nifLoad(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info) {
    NIF_ATOMS(NIF_ATOM_INIT)
    * priv_data = NULL;
    enif_fprintf(stdout, "IMY*************nifload00001\n");
    QUIC_STATUS Status = QUIC_STATUS_SUCCESS;
    enif_fprintf(stdout, "IMY*************nifload1111\n");
    if (QUIC_FAILED(Status = MsQuicOpen2(&MsQuic))) {
        return -1;
    }
    enif_fprintf(stdout, "IMY*************nifload222\n");
    const ERL_NIF_TERM* tuple;
    int arity;

    if (!enif_get_tuple(env, load_info, &arity, &tuple)) {
        return -2;
    }

    unsigned int ExecutionProfile = 0;
    if (!enif_get_uint(env, tuple[0], &ExecutionProfile)) {
        return -2;
    }
    RegConfig.ExecutionProfile = (QUIC_EXECUTION_PROFILE)ExecutionProfile;
    enif_fprintf(stdout, "IMY*************nifload333\n");
    if (QUIC_FAILED(Status = MsQuic->RegistrationOpen(&RegConfig, &Registration))) {
        return -3;
    }
    enif_fprintf(stdout, "IMY*************nifload444\n");
    ErlNifResourceFlags flags = ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER;
    CfgResIns = enif_open_resource_type(env, NULL, "eCfgResIns", cfgResClose, flags, NULL);
    if (NULL == CfgResIns)
        return -4;

    ErlNifResourceTypeInit ListenerInit = { .dtor = listenerResClose, .down = listenerResDown, .stop = NULL };
    ListenerResIns = enif_open_resource_type_x(env, "eListenerResIns", &ListenerInit, flags, NULL);
    if (NULL == ListenerResIns)
        return -4;

    ErlNifResourceTypeInit ConnInit = { .dtor = ConnResClose, .down = ConnResDown, .stop = NULL };
    ConnResIns = enif_open_resource_type_x(env, "eConnResIns", &ConnInit, flags, NULL);
    if (NULL == ConnResIns)
        return -4;

    // make Settings
    QUIC_SETTINGS Settings = { 0 };
    if (!createSettings(env, tuple[2], &Settings))
        return enif_make_tuple2(env, atom_error, enif_make_string(env, "createSettings", ERL_NIF_LATIN1));

    // make Alpn
    unsigned Alpnlen;
    if (!enif_get_list_length(env, tuple[1], &Alpnlen))
        return -5;

    char* AlpnPtr = (char*)enif_alloc(Alpnlen + 1);
    if (!enif_get_string(env, tuple[1], AlpnPtr, Alpnlen + 1, ERL_NIF_LATIN1)) {
        enif_free(AlpnPtr);
        return -6;
    }
    QUIC_BUFFER AlpnBuffer = { Alpnlen, (uint8_t*)AlpnPtr };

    if (QUIC_FAILED(Status = MsQuic->ConfigurationOpen(Registration, &AlpnBuffer, 1, &Settings, sizeof(Settings), NULL, &DefSrvCfg))) {
        enif_free(AlpnPtr);
        return -7;
    }

    if (QUIC_FAILED(Status = MsQuic->ConfigurationOpen(Registration, &AlpnBuffer, 1, &Settings, sizeof(Settings), NULL, &DefCliCfg))) {
        enif_free(AlpnPtr);
        return -8;
    }

    enif_fprintf(stdout, "IMY*************createCredentialAndLoadstart000\n");

    // make srv Credential and load
    if (!createCredentialAndLoad(env, DefSrvCfg, tuple[3])) {
        MsQuic->ConfigurationClose(DefSrvCfg);
        MsQuic->ConfigurationClose(DefCliCfg);
        enif_free(AlpnPtr);
        return -9;;
    }

    // make cli Credential and load
    if (!createCredentialAndLoad(env, DefCliCfg, tuple[4])) {
        MsQuic->ConfigurationClose(DefSrvCfg);
        MsQuic->ConfigurationClose(DefCliCfg);
        enif_free(AlpnPtr);
        return -9;;
    }

    return 0;
}

int nifUpgrade(ErlNifEnv* env, void** priv_data, void** old_priv_data, ERL_NIF_TERM load_info) {
    *priv_data = *old_priv_data;

    return 0;
}

void nifUnload(ErlNifEnv* env, void* priv_data) {
    enif_fprintf(stdout, "IMY*************nifUnload00001\n");
    if (NULL != DefSrvCfg) MsQuic->ConfigurationClose(DefSrvCfg);
    if (NULL != DefCliCfg) MsQuic->ConfigurationClose(DefCliCfg);
    if (NULL != Registration) MsQuic->RegistrationClose(Registration);
    if (NULL != MsQuic) MsQuicClose(MsQuic);
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

ERL_NIF_TERM nifCfgOpen3(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    ERL_NIF_TERM AlpnList = argv[0];
    ERL_NIF_TERM SettingsMap = argv[1];
    ERL_NIF_TERM CredentialMap = argv[2];
    enif_fprintf(stdout, "IMY*************nifCfgOpen3000\n");
    if (!enif_is_list(env, AlpnList) || !enif_is_map(env, SettingsMap) || !enif_is_map(env, CredentialMap))
        return enif_make_badarg(env);

    char* ErrorMsg = NULL;
    QUIC_BUFFER* AlpnBuffers = NULL;
    unsigned AlpnSetCnt = 0;
    HQUIC* ObjIns = NULL;

    // make Settings
    QUIC_SETTINGS Settings = {0};
    if (!createSettings(env, SettingsMap, &Settings)) {
        ErrorMsg = "createSettings";
        goto Failed;
    }
    enif_fprintf(stdout, "IMY*************nifCfgOpen31111\n");
    // make Alpn
    unsigned AlpnCnt = 0;
    if (!enif_get_list_length(env, AlpnList, &AlpnCnt)) {
        ErrorMsg = "getAlpnCnt";
        goto Failed;
    }

    if (AlpnCnt <= 0) {
        ErrorMsg = "AlpnCnt";
        goto Failed;
    }
    enif_fprintf(stdout, "IMY*************nifCfgOpen33333\n");
    AlpnBuffers = (QUIC_BUFFER*)enif_alloc(sizeof(QUIC_BUFFER) * AlpnCnt);
    if (NULL == AlpnBuffers) {
        ErrorMsg = "allocAlpnBuffers";
        goto Failed;
    }
    enif_fprintf(stdout, "IMY*************nifCfgOpen344444 %d %T\n", AlpnCnt, AlpnList);
    if (!loadApln(env, AlpnList, AlpnBuffers, &AlpnSetCnt)) {
        enif_fprintf(stdout, "IMY*************nifCfgOpen3444445555555 %d \n", AlpnSetCnt);
        ErrorMsg = "loadApln";
        goto Failed;
    }
    enif_fprintf(stdout, "IMY*************nifCfgOpen3555555\n");
    // open config
    ObjIns = (HQUIC*)(enif_alloc_resource(CfgResIns, sizeof(HQUIC)));
    if (NULL == ObjIns) {
        ErrorMsg = "allocObjIns";
        goto Failed;
    }
    *ObjIns = NULL;

    enif_fprintf(stdout, "IMY*************nifCfgOpen36666\n");
    QUIC_STATUS Status = QUIC_STATUS_SUCCESS;
    if (QUIC_FAILED(Status = MsQuic->ConfigurationOpen(Registration, AlpnBuffers, AlpnCnt, &Settings, sizeof(Settings), NULL, ObjIns))) {
        ErrorMsg = "ConfigurationOpen";
        goto Failed;
    }
    enif_fprintf(stdout, "IMY*************nifCfgOpen37777  %p \n", *ObjIns);

    // make Credential and load
    if (!createCredentialAndLoad(env, *ObjIns, CredentialMap)) {
        ErrorMsg = "createCredentialAndLoad";
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

    if(NULL!= AlpnBuffers) freeApln(AlpnBuffers, AlpnSetCnt);

    if (NULL != ObjIns) {
        if (NULL != *ObjIns) MsQuic->ConfigurationClose(*ObjIns);
        enif_release_resource(ObjIns);
    }
    return enif_make_tuple2(env, atom_error, enif_make_string(env, ErrorMsg, ERL_NIF_LATIN1));
}

QUIC_STATUS StreamHandler(HQUIC Stream, void* Context, QUIC_STREAM_EVENT* Event){
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
    //     if (!enif_send(NULL, &(s_ctx->owner->Pid), NULL, report))
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
    //     if (!enif_send(NULL, &(s_ctx->owner->Pid), NULL, report))
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
    //     enif_send(NULL, &(s_ctx->owner->Pid), NULL, report);
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

QUIC_STATUS ConnHandler(HQUIC Connection, void* Context, QUIC_CONNECTION_EVENT* Event){
    //QuicerConnCTX* c_ctx = (QuicerConnCTX*)Context;
    //ACCEPTOR* acc = NULL;
    //ErlNifPid* acc_pid = NULL;
    //ERL_NIF_TERM report;
    //ErlNifEnv* env = c_ctx->env;
    //BOOLEAN is_destroy = FALSE;
    //
    //enif_mutex_lock(c_ctx->lock);
    //TP_CB_3(event, (uintptr_t)Connection, Event->Type);
    //switch (Event->Type)
    //{
    //case QUIC_CONNECTION_EVENT_CONNECTED:
    //    //
    //    // The handshake has completed for the connection.
    //    //
    //
    //    assert(c_ctx->Connection == Connection);
    //    c_ctx->Connection = Connection;
    //    acc = c_ctx->owner;
    //    assert(acc);
    //    acc_pid = &(acc->Pid);
    //
    //    // A monitor is automatically removed when it triggers or when the
    //    // resource is deallocated.
    //    enif_monitor_process(NULL, c_ctx, acc_pid, &c_ctx->owner_mon);
    //
    //    ERL_NIF_TERM ConnHandler = enif_make_resource(c_ctx->env, c_ctx);
    //    // testing this, just unblock accecptor
    //    // should pick a 'acceptor' here?
    //    if (!enif_send(NULL,
    //        acc_pid,
    //        NULL,
    //        enif_make_tuple3(
    //            c_ctx->env, ATOM_QUIC, ATOM_CONNECTED, ConnHandler)))
    //    {
    //        enif_mutex_unlock(c_ctx->lock);
    //        return QUIC_STATUS_UNREACHABLE;
    //    }
    //
    //    MsQuic->ConnectionSendResumptionTicket(
    //        Connection, QUIC_SEND_RESUMPTION_FLAG_NONE, 0, NULL);
    //
    //    break;
    //case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
    //    //
    //    // The connection has been shut down by the transport. Generally, this
    //    // is the expected way for the connection to shut down with this
    //    // protocol, since we let idle timeout kill the connection.
    //    //
    //    /* printf("[conn][%p] Shut down by transport, 0x%x\n", Connection, */
    //    /*        Event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status); */
    //    break;
    //case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
    //    //
    //    // The connection was explicitly shut down by the peer.
    //    //
    //    report = enif_make_tuple3(
    //        env, ATOM_QUIC, ATOM_SHUTDOWN, enif_make_resource(env, c_ctx));
    //
    //    if (!enif_send(NULL, &(c_ctx->owner->Pid), NULL, report))
    //    {
    //        // Owner is gone, we shutdown our side as well.
    //        // connection shutdown could result a connection close
    //        MsQuic->ConnectionShutdown(Connection,
    //            QUIC_CONNECTION_SHUTDOWN_FLAG_NONE,
    //            QUIC_STATUS_UNREACHABLE);
    //    }
    //
    //    break;
    //case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
    //    //
    //    // The connection has completed the shutdown process and is ready to be
    //    // safely cleaned up.
    //    //
    //    TP_CB_3(shutdown_complete,
    //        (uintptr_t)Connection,
    //        Event->SHUTDOWN_COMPLETE.AppCloseInProgress);
    //
    //    report = enif_make_tuple3(
    //        env, ATOM_QUIC, ATOM_CLOSED, enif_make_resource(env, c_ctx));
    //
    //    enif_send(NULL, &(c_ctx->owner->Pid), NULL, report);
    //    c_ctx->is_closed = TRUE; // server shutdown_complete
    //    is_destroy = TRUE;
    //    break;
    //case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED:
    //    //
    //    // The peer has started/created a new stream. The app MUST set the
    //    // callback handler before returning.
    //    //
    //    // maybe alloc later
    //    ;
    //    QuicerStreamCTX* s_ctx = init_s_ctx();
    //    enif_keep_resource(c_ctx);
    //    s_ctx->c_ctx = c_ctx;
    //    s_ctx->eHandler = enif_make_resource(s_ctx->imm_env, s_ctx);
    //
    //    ErlNifEnv* env = s_ctx->env;
    //    s_ctx->Stream = Event->PEER_STREAM_STARTED.Stream;
    //
    //    acc = AcceptorDequeue(c_ctx->acceptor_queue);
    //
    //    if (!acc)
    //    {
    //        // If we don't have available process
    //        // fallback to the connection owner
    //        acc = AcceptorAlloc();
    //        if (!acc)
    //        {
    //            return QUIC_STATUS_UNREACHABLE;
    //        }
    //        // We must copy here, otherwise it will become double free
    //        // in resource dealloc callbacks (for Stream and Connection)
    //        memcpy(acc, c_ctx->owner, sizeof(ACCEPTOR));
    //    }
    //
    //    assert(acc);
    //    acc_pid = &(acc->Pid);
    //
    //    s_ctx->owner = acc;
    //    s_ctx->is_closed = FALSE;
    //
    //    // @todo add monitor here.
    //    if (!enif_send(NULL,
    //        acc_pid,
    //        NULL,
    //        enif_make_tuple3(env,
    //            ATOM_QUIC,
    //            ATOM_NEW_STREAM,
    //            enif_make_resource(env, s_ctx))))
    //    {
    //        // @TODO: check RFC for the error code
    //        MsQuic->ConnectionShutdown(
    //            Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
    //        enif_mutex_unlock(s_ctx->lock);
    //        return QUIC_STATUS_UNREACHABLE;
    //    }
    //    else
    //    {
    //        MsQuic->SetCallbackHandler(Event->PEER_STREAM_STARTED.Stream,
    //            (void*)StreamHandler,
    //            s_ctx);
    //    }
    //    break;
    //case QUIC_CONNECTION_EVENT_RESUMED:
    //    //
    //    // The connection succeeded in doing a TLS resumption of a previous
    //    // connection's session.
    //    //
    //    break;
    //case QUIC_CONNECTION_EVENT_DATAGRAM_STATE_CHANGED:
    //    handle_dgram_state_event(c_ctx, Event);
    //    break;
    //case QUIC_CONNECTION_EVENT_DATAGRAM_SEND_STATE_CHANGED:
    //    handle_dgram_send_state_event(c_ctx, Event);
    //    break;
    //case QUIC_CONNECTION_EVENT_DATAGRAM_RECEIVED:
    //    handle_dgram_recv_event(c_ctx, Event);
    //    break;
    //default:
    //    break;
    //}
    //enif_clear_env(env);
    //enif_mutex_unlock(c_ctx->lock);
    //
    //if (is_destroy)
    //{
    //    destroy_c_ctx(c_ctx);
    //}

    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS ListenerHandler(HQUIC Listener, void* Context, QUIC_LISTENER_EVENT* Event){
    QUIC_STATUS Status = QUIC_STATUS_SUCCESS;
    ConnRes* ObjIns = NULL;
    ErlNifEnv* ConnEnv = NULL;
    bool IsNeedClose = false;

    switch (Event->Type){
        case QUIC_LISTENER_EVENT_NEW_CONNECTION: {
            IsNeedClose = true;
            ObjIns = (ConnRes*)(enif_alloc_resource(ConnResIns, sizeof(ConnRes)));
            if (NULL == ObjIns) {
                Status = QUIC_STATUS_OUT_OF_MEMORY;
                goto Failed;
            }

            ConnEnv = enif_alloc_env();
            if (NULL == ConnEnv){
                Status = QUIC_STATUS_OUT_OF_MEMORY;
                goto Failed;
            }

            ObjIns->ConnEnv = ConnEnv;

            ObjIns->Connection = Event->NEW_CONNECTION.Connection;
            ErlNifPid* OwnerPid = nextLList(&((ListenerRes*)Context)->AcceptWorkers);

            if (NULL == OwnerPid){
                Status = QUIC_STATUS_INTERNAL_ERROR;
                ObjIns->Connection = NULL;
                goto Failed;
            }

            ObjIns->OwnerPid = OwnerPid;
            if (0 != enif_monitor_process(NULL, ObjIns, OwnerPid, &(ObjIns->mon))) {
                Status = QUIC_STATUS_INTERNAL_ERROR;
                goto Failed;
            }

            MsQuic->SetCallbackHandler(Event->NEW_CONNECTION.Connection,(void*)ConnHandler, ObjIns);
            Status = MsQuic->ConnectionSetConfiguration(Event->NEW_CONNECTION.Connection, ((ListenerRes*)Context)->Configuration);

            if (QUIC_FAILED(Status = MsQuic->ConnectionSetConfiguration(Event->NEW_CONNECTION.Connection, ((ListenerRes*)Context)->Configuration))) {
                goto Failed;
            }

                if (!enif_send(NULL, OwnerPid, ObjIns->ConnEnv, enif_make_tuple2(ObjIns->ConnEnv, atom_quicNConn, enif_make_resource(ObjIns->ConnEnv, ObjIns)))) {
                    Status = QUIC_STATUS_INTERNAL_ERROR;
                    goto Failed;
                }

                enif_clear_env(ObjIns->ConnEnv);
                break;
        }
        case QUIC_LISTENER_EVENT_STOP_COMPLETE:
            break;
        default:
            break;
    }
    return Status;

Failed:
    if (NULL != ObjIns) enif_release_resource(ObjIns);
    if (NULL != ConnEnv) enif_free_env(ConnEnv);
    if (IsNeedClose) MsQuic->ConnectionClose(Event->NEW_CONNECTION.Connection);
    return Status;
}

ERL_NIF_TERM nifListenerStart3(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    ERL_NIF_TERM ListenAddr = argv[0];
    ERL_NIF_TERM ListenPort = argv[1];
    ERL_NIF_TERM ListenOpts = argv[2];

    if (!enif_is_list(env, ListenAddr) || !enif_is_number(env, ListenPort) || !enif_is_map(env, ListenOpts))
        return enif_make_badarg(env);
    enif_fprintf(stdout, "IMY*************nifListenerStart3 000\n");
    char* ErrorMsg = NULL;
    ListenerRes* ObjIns = NULL;
    HQUIC Listener = NULL;
    QUIC_BUFFER* AlpnBuffers = NULL;
    unsigned AlpnSetCnt = 0;
    enif_fprintf(stdout, "IMY*************nifListenerStart3 111\n");
    // open config
    ObjIns = (ListenerRes*)(enif_alloc_resource(ListenerResIns, sizeof(ListenerRes)));
    if (NULL == ObjIns) {
        ErrorMsg = "allocObjIns";
        goto Failed;
    }
    ObjIns->AcceptWorkers = NULL;
    enif_self(env, &(ObjIns->ListerPid));

    enif_fprintf(stdout, "IMY*************nifListenerStart3 2222\n");
    // open listener
    QUIC_STATUS Status = QUIC_STATUS_SUCCESS;
    if (QUIC_FAILED(Status = MsQuic->ListenerOpen(Registration, ListenerHandler, ObjIns, &Listener))) {
        ErrorMsg = "ListenerOpen";
        goto Failed;
    }
    enif_fprintf(stdout, "IMY*************nifListenerStart3 333\n");
    // start listener
    unsigned UdpPort;
    if (!enif_get_uint(env, ListenPort, &UdpPort)) {
        ErrorMsg = "ListenPort";
        goto Failed;
    }
    enif_fprintf(stdout, "IMY*************nifListenerStart3 4444\n");
    QUIC_ADDR Address = {0};
    if (enif_is_empty_list(env, ListenAddr)) {
        QuicAddrSetFamily(&Address, QUIC_ADDRESS_FAMILY_UNSPEC);
        QuicAddrSetPort(&Address, (uint16_t)UdpPort);
    }else {
        char* AddrPtr = NULL;
        AddrPtr = getStrFromTrem(env, ListenAddr);
        if (NULL == AddrPtr) {
            ErrorMsg = "ListenAddr";
            goto Failed;
        }
        if (!QuicAddrFromString(AddrPtr, (uint16_t)UdpPort, &Address)) {
            enif_free(AddrPtr);
            ErrorMsg = "QuicAddrFromString";
            goto Failed;
        }
        enif_free(AddrPtr);
        AddrPtr = NULL;
    }
    ERL_NIF_TERM AlpnList;
    if (!enif_get_map_value(env, ListenOpts, atom_eAlpn, &AlpnList)) {
        ErrorMsg = "AlpnList";
        goto Failed;
    }
    enif_fprintf(stdout, "IMY*************nifListenerStart3 5555\n");
    // make Alpn
    unsigned AlpnCnt = 0;
    if (!enif_get_list_length(env, AlpnList, &AlpnCnt)) {
        ErrorMsg = "AlpnCnt";
        goto Failed;
    }

    if (AlpnCnt <= 0) {
        ErrorMsg = "AlpnCnt=0";
        goto Failed;
    }

    AlpnBuffers = (QUIC_BUFFER*)enif_alloc(sizeof(QUIC_BUFFER) * AlpnCnt);

    if (!loadApln(env, AlpnList, AlpnBuffers, &AlpnSetCnt)) {
        ErrorMsg = "loadApln";
        goto Failed;
    }

    // set ObjIns fields
    ERL_NIF_TERM RefTerm;
    if (!enif_get_map_value(env, ListenOpts, atom_eCfgRef, &RefTerm)) {
        ObjIns->Configuration = DefSrvCfg;
    }else {
        HQUIC* CfgRes = NULL;
        if (!enif_get_resource(env, RefTerm, CfgResIns, (void**)&CfgRes)) {
            ErrorMsg = "getCfgResObj";
            goto Failed;
        }
        ObjIns->Configuration = *CfgRes;
    }
    enif_fprintf(stdout, "IMY*************nifListenerStart3 666611\n");

    ObjIns->Listener = Listener;

    enif_fprintf(stdout, "IMY*************nifListenerStart3 666622 %T\n", ObjIns->ListerPid);
    if (0 != enif_monitor_process(env, ObjIns, &(ObjIns->ListerPid), &(ObjIns->mon))) {
        ErrorMsg = "MonitorObj";
        goto Failed;
    }
    enif_fprintf(stdout, "IMY*************nifListenerStart3 6666\n");
    // set ObjIns AcceptWorkers  todo 设置完acceptworkers
    ERL_NIF_TERM AcceptWorkers;
    if (!enif_get_map_value(env, ListenOpts, atom_eAcceptWorkers, &AcceptWorkers)) {
        ErrorMsg = "AcceptWorkers";
        goto Failed;
    }

    unsigned AcceptWorkerLen = 0;
    if (!enif_get_list_length(env, AcceptWorkers, &AcceptWorkerLen)) {
        ErrorMsg = "AcceptWorkersLen";
        goto Failed;
    }

    if (AcceptWorkerLen <= 0) {
        ErrorMsg = "AcceptWorkersLen=0";
        goto Failed;
    }
    enif_fprintf(stdout, "IMY*************nifListenerStart3 7777\n");
    ERL_NIF_TERM PidTerm;
    while (enif_get_list_cell(env, AcceptWorkers, &PidTerm, &AcceptWorkers)) {
        ErlNifPid* AWPid = enif_alloc(sizeof(ErlNifPid));
        if (NULL == AWPid) {
            ErrorMsg = "allocPid";
            goto Failed;
        }
        if (!enif_get_local_pid(env, PidTerm, AWPid)) {
            enif_free(AWPid);
            ErrorMsg = "getPid";
            goto Failed;
        }
        if (0 != enif_monitor_process(env, ObjIns, AWPid, NULL)) {
            enif_free(AWPid);
            ErrorMsg = "MonitorPid";
            goto Failed;
        }
        addLList(&ObjIns->AcceptWorkers, AWPid);
    }

    if (QUIC_FAILED(Status = MsQuic->ListenerStart(Listener, AlpnBuffers, AlpnCnt, &Address))) {
        enif_fprintf(stdout, "IMY*************nifListenerStart3 888 %d\n", Status);
        ErrorMsg = "ListenerStart";
        goto Failed;
    }

    RefTerm = enif_make_resource(env, ObjIns);
    ObjIns->RefTerm = RefTerm;
    enif_fprintf(stdout, "IMY*************nifListenerStart3 888\n");

    ErlNifEnv* MsgEnv = enif_alloc_env();
    if (NULL == MsgEnv) {
        ErrorMsg = "allocMsgEnv";
        goto Failed;
    }

    ERL_NIF_TERM AcceptMsg, ListenerTag;
    if (!enif_get_map_value(env, ListenOpts, atom_eListenerTag, &ListenerTag)) {
        ListenerTag = atom_undefined;
    }
    ErlNifPid* CurAWPid = (ErlNifPid*)nextLList(&ObjIns->AcceptWorkers);
    ErlNifPid* TempAWPid = CurAWPid;
    do {
        AcceptMsg = enif_make_tuple3(MsgEnv, atom_quicLStart, ListenerTag, RefTerm);
        enif_send(env, TempAWPid, MsgEnv, AcceptMsg);
        TempAWPid = (ErlNifPid*)nextLList(&ObjIns->AcceptWorkers);
        enif_clear_env(MsgEnv);
    } while (TempAWPid != CurAWPid);
    enif_free_env(MsgEnv);

    freeApln(AlpnBuffers, AlpnSetCnt);
    enif_release_resource(ObjIns);
    return enif_make_tuple2(env, atom_ok, RefTerm);

Failed:
    enif_fprintf(stdout, "IMY*************nifListenerStart3 Failed\n");
    if(NULL != AlpnBuffers) freeApln(AlpnBuffers, AlpnSetCnt);
    if (NULL != ObjIns) {
        clearLList(&ObjIns->AcceptWorkers, AWPidFree);
        enif_release_resource(ObjIns);
    }
    if(NULL != Listener) MsQuic->ListenerClose(Listener);
    return enif_make_tuple2(env, atom_error, enif_make_string(env, ErrorMsg, ERL_NIF_LATIN1));
}

ERL_NIF_TERM nifListenerClose1(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    ListenerRes* ObjIns = NULL;
    if (!enif_get_resource(env, argv[0], ListenerResIns, (void**)&ObjIns)) {
        return enif_make_tuple2(env, atom_error, argv[0]);
    }

    MsQuic->ListenerStop(ObjIns->Listener);

    ErlNifPid* CurAWPid = (ErlNifPid*)nextLList(&ObjIns->AcceptWorkers);
    if (NULL != CurAWPid) {
        ErlNifEnv* MsgEnv = enif_alloc_env();
        if (NULL == MsgEnv) {
            clearLList(&ObjIns->AcceptWorkers, AWPidFree);
            return;
        }
        ERL_NIF_TERM CloseMsg = enif_make_tuple2(MsgEnv, atom_quicLClose, ObjIns->RefTerm);
        ErlNifPid* TempAWPid = CurAWPid;
        do {
            enif_send(NULL, TempAWPid, MsgEnv, CloseMsg);
            TempAWPid = (ErlNifPid*)nextLList(&ObjIns->AcceptWorkers);
            enif_clear_env(MsgEnv);
        } while (TempAWPid != CurAWPid);
        enif_free_env(MsgEnv);
        clearLList(&ObjIns->AcceptWorkers, AWPidFree);
    }
}

static ErlNifFunc nifFuns[] = {
        {"paramCfgs", 0, nifParamCfgs0},
        {"cfgOpen", 3, nifCfgOpen3},
        {"listenerStart", 3, nifListenerStart3},
        {"listenerClose", 1, nifListenerClose1},

        /* for DEBUG */
        //{ "getConnId", 1, nifGetConnId1, ERL_NIF_DIRTY_JOB_CPU_BOUND},
        //{ "getStreamId", 1, nigGetStreamId1, ERL_NIF_DIRTY_JOB_CPU_BOUND}

};

ERL_NIF_INIT(eQuic, nifFuns, nifLoad, NULL, nifUpgrade, nifUnload)