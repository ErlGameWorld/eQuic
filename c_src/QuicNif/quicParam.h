#ifndef __QUIC_PARAM_H_
#define __QUIC_PARAM_H_

#include<erl_nif.h>
#include<msquic.h>

#define MakeFlag(Type, IsCanGet, IsCanSet) (1 << (Type + 3) | IsCanGet << 1 | IsCanSet)
#define IsCanGet(Flag) ((Flag >> 1) & 1 == 1)
#define IsCanSet(Flag) (Flag & 1 == 1)
#define IsCanType(Flag, Type) (Flag >> (Type + 3) & 1 == 1)

// https://github.com/microsoft/msquic/blob/main/docs/Settings.md
struct ParamCfg_r {
	char* ParamName;
	uint32_t ParamType;
	uint16_t HTGSFlag;	  // HandleTypeAndGetSetFlag ob0000000(not use) 111111(QUIC_HANDLE_TYPE (0-5)) 1(Global(-1))  11(get/set)
	ERL_NIF_TERM(*getFun)(HQUIC, uint32_t);
	QUIC_STATUS(*setFun)(HQUIC, uint32_t, ERL_NIF_TERM);
}ParamCfg[] = {
	{"test", QUIC_PARAM_STREAM_STATISTICS, MakeFlag(5, 1, 0), NULL, NULL},
	//	// Global Parameters    Type -1
	//	{"gpRETRY_MEMORY_PERCENT", QUIC_PARAM_GLOBAL_RETRY_MEMORY_PERCENT, MakeFlag(-1, 1, 1), Uint16ToTerm, TermToUint16},
	//	{"gpSUPPORTED_VERSIONST", QUIC_PARAM_GLOBAL_SUPPORTED_VERSIONS, MakeFlag(-1,1,0), Uint32sToTerm, NULL},
	//	{"gpLOAD_BALACING_MODE", QUIC_PARAM_GLOBAL_LOAD_BALACING_MODE, MakeFlag(-1,1,1), Uint16ToTerm, TermToUint16},
	//	{"gpPERF_COUNTERS", QUIC_PARAM_GLOBAL_PERF_COUNTERS, MakeFlag(-1,1,0), Uint64sToTerm, TermToUint64s},
	//	{"gpLIBRARY_VERSION", QUIC_PARAM_GLOBAL_LIBRARY_VERSION, MakeFlag(-1,1,0), Uint32sToTerm, NULL},
	//	{"gpSETTINGS", QUIC_PARAM_GLOBAL_SETTINGS, MakeFlag(-1,1,1), SettingsToTerm, TermToSettings},
	//	{"gpGLOBAL_SETTINGS", QUIC_PARAM_GLOBAL_GLOBAL_SETTINGS, MakeFlag(-1,1,1), GSettingsToTerm, TermToGSettings},
	//	{"gpVERSION_SETTINGS", QUIC_PARAM_GLOBAL_VERSION_SETTINGS, MakeFlag(-1,1,1), VSettingsToTerm, TermToVSettings},
	//	{"gpLIBRARY_GIT_HASH", QUIC_PARAM_GLOBAL_LIBRARY_GIT_HASH, MakeFlag(-1,1,0), CharsToTerm, NULL},
	//	{"gpDATAPATH_PROCESSORS", QUIC_PARAM_GLOBAL_EXECUTION_CONFIG, MakeFlag(-1,1,1), Uint16ToTerm, TermToUint16},
	//	{"gpTLS_PROVIDER", QUIC_PARAM_GLOBAL_TLS_PROVIDER, MakeFlag(-1,1, 0), TlsProviderToTerm, NULL},
	//
	//	// Global Parameters QUIC_HANDLE_TYPE:QUIC_HANDLE_TYPE_REGISTRATION(0)
	//	// none 
	//
	//	// Configuration Parameters QUIC_HANDLE_TYPE:QUIC_HANDLE_TYPE_CONFIGURATION(1)
	//	{"cpSETTINGS", QUIC_PARAM_CONFIGURATION_SETTINGS, MakeFlag(1, 1, 1), SettingsToTerm, TermToSettings},
	//	{"cpTICKET_KEYS", QUIC_PARAM_CONFIGURATION_TICKET_KEYS, MakeFlag(1,0, 1), NULL, TermToTicketKeyCfgs},
	//	{"cpVERSION_SETTINGS", QUIC_PARAM_CONFIGURATION_VERSION_SETTINGS, MakeFlag(1,1, 1), VSettingsToTerm, TermToVSettings},
	//	{"cpSCHANNEL_CREDENTIAL_ATTRIBUTE_W", QUIC_PARAM_CONFIGURATION_SCHANNEL_CREDENTIAL_ATTRIBUTE_W, MakeFlag(1, 0, 1), NULL, TermToSCAW},
	//
	//	// Listener Parameters QUIC_HANDLE_TYPE:QUIC_HANDLE_TYPE_LISTENER(2)
	//	{"lpLOCAL_ADDRESS", QUIC_PARAM_LISTENER_LOCAL_ADDRESS, MakeFlag(2,1, 0), AddrToTerm, NULL},
	//	{"lpSTATS", QUIC_PARAM_LISTENER_STATS, MakeFlag(2,1, 0), LStatisticsToTerm, NULL},
	//	{"lpCIBIR_ID", QUIC_PARAM_LISTENER_CIBIR_ID, MakeFlag(2,1, 1), Uint8sToTerm, TermToUint8s},
	//
	//	// Connection Parameters QUIC_HANDLE_TYPE:QUIC_HANDLE_TYPE_CONNECTION_CLIENT(3) QUIC_HANDLE_TYPE_CONNECTION_SERVER(4)
	//	{"cpQUIC_VERSION" , QUIC_PARAM_CONN_QUIC_VERSION, MakeFlag(3, 1, 0) | MakeFlag(4, 1, 0), Uint32sToTerm, NULL},
	//	{"cpLOCAL_ADDRESS" , QUIC_PARAM_CONN_LOCAL_ADDRESS, MakeFlag(3, 1, 1), AddrToTerm, TermToAddr},
	//	{"cpREMOTE_ADDRESS" , QUIC_PARAM_CONN_REMOTE_ADDRESS, MakeFlag(3, 1, 1), AddrToTerm, TermToAddr},
	//	{"cpIDEAL_PROCESSOR" , QUIC_PARAM_CONN_IDEAL_PROCESSOR, MakeFlag(3, 1, 0) | MakeFlag(4, 1, 0), Uint16ToTerm, NULL},
	//	{"cpSETTINGS" , QUIC_PARAM_CONN_SETTINGS, MakeFlag(3, 1, 1) | MakeFlag(4, 1, 1), SettingsToTerm, TermToSettings},
	//	{"cpSTATISTICS" , QUIC_PARAM_CONN_STATISTICS, MakeFlag(3, 1, 0) | MakeFlag(4, 1, 0), StatisticsToTerm, NULL},
	//	{"cpSTATISTICS_PLAT" , QUIC_PARAM_CONN_STATISTICS_PLAT, MakeFlag(3, 1, 0) | MakeFlag(4, 1, 0), StatisticsToTerm, NULL},
	//	{"cpSHARE_UDP_BINDING" , QUIC_PARAM_CONN_SHARE_UDP_BINDING, MakeFlag(3, 1, 1), Uint8ToTerm, TermToUint8},
	//	{"cpLOCAL_BIDI_STREAM_COUNT" ,QUIC_PARAM_CONN_LOCAL_BIDI_STREAM_COUNT, MakeFlag(3, 1, 0) | MakeFlag(4, 1, 0), Uint16ToTerm, NULL},
	//	{"cpLOCAL_UNIDI_STREAM_COUNT" , QUIC_PARAM_CONN_LOCAL_UNIDI_STREAM_COUNT, MakeFlag(3, 1, 0) | MakeFlag(4, 1, 0), Uint16ToTerm, NULL},
	//	{"cpMAX_STREAM_IDS" , QUIC_PARAM_CONN_MAX_STREAM_IDS, MakeFlag(3, 1, 0) | MakeFlag(4, 1, 0), Uint64sToTerm, NULL},
	//	{"cpCLOSE_REASON_PHRASE" , QUIC_PARAM_CONN_CLOSE_REASON_PHRASE, MakeFlag(3, 1, 1) | MakeFlag(4, 1, 1), CharsToTerm, TermToChars},
	//	{"cpSTREAM_SCHEDULING_SCHEME" , QUIC_PARAM_CONN_STREAM_SCHEDULING_SCHEME, MakeFlag(3, 1, 1) | MakeFlag(4, 1, 1), SSSToTerm, TermToSSS},
	//	{"cpDATAGRAM_RECEIVE_ENABLED" , QUIC_PARAM_CONN_DATAGRAM_RECEIVE_ENABLED, MakeFlag(3, 1, 1) | MakeFlag(4, 1, 1), Uint8ToTerm, TermToUint8},
	//	{"cpDATAGRAM_SEND_ENABLED" , QUIC_PARAM_CONN_DATAGRAM_SEND_ENABLED, MakeFlag(3, 1, 0) | MakeFlag(4, 1, 0), Uint8ToTerm, NULL},
	//	{"cpDISABLE_1RTT_ENCRYPTION" , QUIC_PARAM_CONN_DISABLE_1RTT_ENCRYPTION	, MakeFlag(3, 1, 1) | MakeFlag(4, 1, 1), Uint8ToTerm, TermToUint8},
	//	{"cpRESUMPTION_TICKET" , QUIC_PARAM_CONN_RESUMPTION_TICKET, MakeFlag(3, 0, 1), NULL, TermToUint8s},
	//	{"cpPEER_CERTIFICATE_VALID" , QUIC_PARAM_CONN_PEER_CERTIFICATE_VALID, MakeFlag(3, 0, 1) | MakeFlag(4, 0, 1), NULL, TermToUint8},
	//	{"cpLOCAL_INTERFACE" , QUIC_PARAM_CONN_LOCAL_INTERFACE, MakeFlag(3, 0, 1) | MakeFlag(4, 0, 1), NULL, TermToUint8},
	//	{"cpTLS_SECRETS" , QUIC_PARAM_CONN_TLS_SECRETS, MakeFlag(3, 0, 1) | MakeFlag(4, 0, 1), NULL, TermToTs},
	//	{"cpVERSION_SETTINGS" , QUIC_PARAM_CONN_VERSION_SETTINGS, MakeFlag(3, 1, 1) | MakeFlag(4, 1, 1), VsToTerm, TermToVs},
	//	{"cpCIBIR_ID" , QUIC_PARAM_CONN_CIBIR_ID, MakeFlag(3, 0, 1) | MakeFlag(4, 0, 1), NULL, TermToUint8s},
	//	{"cpSTATISTICS_V2" , QUIC_PARAM_CONN_STATISTICS_V2	, MakeFlag(3, 1, 0) | MakeFlag(4, 1, 0), SV2ToTerm, NULL},
	//	{"cpSTATISTICS_V2_PLAT" , QUIC_PARAM_CONN_STATISTICS_V2_PLAT, MakeFlag(3, 1, 0) | MakeFlag(4, 1, 0), SV2ToTerm, NULL},
	//
	//	// TLS Parameters
	//	{"cpTLS_HANDSHAKE_INFO" , QUIC_PARAM_TLS_HANDSHAKE_INFO, MakeFlag(3, 1, 0) | MakeFlag(4, 1, 0), HSInfoToTerm, NULL},
	//	{"cpTLS_NEGOTIATED_ALPN" , QUIC_PARAM_TLS_NEGOTIATED_ALPN, MakeFlag(3, 1, 0) | MakeFlag(4, 1, 0), Uint8sToTerm, NULL},
	//
	//
	//	// Schannel-only TLS Parameters
	//	{"cpTLS_SCHANNEL_CONTEXT_ATTRIBUTE_W" , QUIC_PARAM_TLS_SCHANNEL_CONTEXT_ATTRIBUTE_W, MakeFlag(3, 1, 0) | MakeFlag(4, 1, 0), SCAWToTerm, NULL},
	//	{"cpTLS_SCHANNEL_CONTEXT_ATTRIBUTE_EX_W" , QUIC_PARAM_TLS_SCHANNEL_CONTEXT_ATTRIBUTE_EX_W, MakeFlag(3, 1, 0) | MakeFlag(4, 1, 0), SCAEWToTerm, NULL},
	//	{"cpTLS_SCHANNEL_SECURITY_CONTEXT_TOKEN" , QUIC_PARAM_TLS_SCHANNEL_SECURITY_CONTEXT_TOKEN, MakeFlag(3, 1, 0) | MakeFlag(4, 1, 0), HANDLEToTerm, NULL},
	//
	//	// Stream Parameters QUIC_HANDLE_TYPE:QUIC_HANDLE_TYPE_STREAM(5)
	//	{"spID", QUIC_PARAM_STREAM_ID, MakeFlag(5, 1, 0), NULL, NULL},
	//	{"sp0RTT_LENGTH", QUIC_PARAM_STREAM_0RTT_LENGTH, MakeFlag(5, 1, 0), Uint64ToTerm, NULL},
	//	{"spIDEAL_SEND_BUFFER_SIZE", QUIC_PARAM_STREAM_IDEAL_SEND_BUFFER_SIZE, MakeFlag(5, 1, 0), Uint64ToTerm, NULL},
	//	{"spPRIORITY", QUIC_PARAM_STREAM_PRIORITY, MakeFlag(5, 1, 0), Uint64ToTerm, NULL},
	//	{"spSTATISTICS", QUIC_PARAM_STREAM_STATISTICS, MakeFlag(5, 1, 0), StreamSToTerm, NULL},

};


ERL_NIF_TERM AddrToTerm(ErlNifEnv* MsgEnv, const QUIC_ADDR* addr) {
	return atom_undefined;
}

ERL_NIF_TERM StreamsAvailable(ErlNifEnv* MsgEnv, QUIC_CONNECTION_EVENT* Event) {
	return atom_undefined;
}

ERL_NIF_TERM IdealProcessorChanged(ErlNifEnv* MsgEnv, QUIC_CONNECTION_EVENT* Event) {
	return atom_undefined;
}

ERL_NIF_TERM DatagramStateChanged(ErlNifEnv* MsgEnv, QUIC_CONNECTION_EVENT* Event) {
	return atom_undefined;
}

ERL_NIF_TERM DatagramSendStateChanged(ErlNifEnv* MsgEnv, QUIC_CONNECTION_EVENT* Event) {
	return atom_undefined;
}

ERL_NIF_TERM ResumptionTicketReceived(ErlNifEnv* MsgEnv, QUIC_CONNECTION_EVENT* Event) {
	return atom_undefined;
}

ERL_NIF_TERM PeerCertificateReceived(ErlNifEnv* MsgEnv, QUIC_CONNECTION_EVENT* Event) {
	return atom_undefined;
}

#endif
