#ifndef __QUIC_STATUS_H_
#define __QUIC_STATUS_H_

#include<erl_nif.h>
#include<msquic.h>

#define ErrMsg(env, Str) (enif_make_string(env, Str, ERL_NIF_LATIN1))

// https://github.com/microsoft/msquic/blob/main/docs/api/QUIC_STATUS.md
ERL_NIF_TERM StatusMsg(ErlNifEnv* env, QUIC_STATUS status)
{
	switch (status) {
	case QUIC_STATUS_SUCCESS:					return ErrMsg(env, "SUCCESS");
	case QUIC_STATUS_PENDING:					return ErrMsg(env, "PENDING");
	case QUIC_STATUS_CONTINUE:					return ErrMsg(env, "CONTINUE");
	case QUIC_STATUS_OUT_OF_MEMORY:				return ErrMsg(env, "OUT_OF_MEMORY");
	case QUIC_STATUS_INVALID_PARAMETER:			return ErrMsg(env, "INVALID_PARAMETER");
	case QUIC_STATUS_INVALID_STATE:				return ErrMsg(env, "INVALID_STATE");
	case QUIC_STATUS_NOT_SUPPORTED:				return ErrMsg(env, "NOT_SUPPORTED");
	case QUIC_STATUS_NOT_FOUND:					return ErrMsg(env, "NOT_FOUND");
	case QUIC_STATUS_BUFFER_TOO_SMALL:			return ErrMsg(env, "BUFFER_TOO_SMALL");
	case QUIC_STATUS_HANDSHAKE_FAILURE:			return ErrMsg(env, "HANDSHAKE_FAILURE");
	case QUIC_STATUS_ABORTED:					return ErrMsg(env, "ABORTED");
	case QUIC_STATUS_ADDRESS_IN_USE:			return ErrMsg(env, "ADDRESS_IN_USE");
	case QUIC_STATUS_INVALID_ADDRESS:			return ErrMsg(env, "INVALID_ADDRESS");
	case QUIC_STATUS_CONNECTION_TIMEOUT:		return ErrMsg(env, "CONNECTION_TIMEOUT");
	case QUIC_STATUS_CONNECTION_IDLE:			return ErrMsg(env, "CONNECTION_IDLE");
	case QUIC_STATUS_INTERNAL_ERROR:			return ErrMsg(env, "INTERNAL_ERROR");
	case QUIC_STATUS_UNREACHABLE:				return ErrMsg(env, "UNREACHABLE");
	case QUIC_STATUS_CONNECTION_REFUSED:		return ErrMsg(env, "CONNECTION_REFUSED");
	case QUIC_STATUS_PROTOCOL_ERROR:			return ErrMsg(env, "PROTOCOL_ERROR");
	case QUIC_STATUS_VER_NEG_ERROR:				return ErrMsg(env, "VER_NEG_ERROR");
	case QUIC_STATUS_TLS_ERROR:					return ErrMsg(env, "TLS_ERROR");
	case QUIC_STATUS_USER_CANCELED:				return ErrMsg(env, "USER_CANCELED");
	case QUIC_STATUS_ALPN_NEG_FAILURE:			return ErrMsg(env, "ALPN_NEG_FAILURE");
	case QUIC_STATUS_STREAM_LIMIT_REACHED:		return ErrMsg(env, "LIMIT_REACHED");
	case QUIC_STATUS_ALPN_IN_USE:				return ErrMsg(env, "ALPN_IN_USE");
	case QUIC_STATUS_CLOSE_NOTIFY:				return ErrMsg(env, "CLOSE_NOTIFY");
	case QUIC_STATUS_BAD_CERTIFICATE:			return ErrMsg(env, "BAD_CERTIFICATE");
	case QUIC_STATUS_UNSUPPORTED_CERTIFICATE:	return ErrMsg(env, "UNSUPPORTED_CERTIFICATE");
	case QUIC_STATUS_REVOKED_CERTIFICATE:		return ErrMsg(env, "REVOKED_CERTIFICATE");
	case QUIC_STATUS_EXPIRED_CERTIFICATE:		return ErrMsg(env, "EXPIRED_CERTIFICATE");
	case QUIC_STATUS_UNKNOWN_CERTIFICATE:		return ErrMsg(env, "UNKNOWN_CERTIFICATE");
	case QUIC_STATUS_REQUIRED_CERTIFICATE:		return ErrMsg(env, "REQUIRED_CERTIFICATE");
	case QUIC_STATUS_CERT_EXPIRED:				return ErrMsg(env, "CERT_EXPIRED");
	case QUIC_STATUS_CERT_UNTRUSTED_ROOT:		return ErrMsg(env, "CERT_UNTRUSTED_ROOT");
	case QUIC_STATUS_CERT_NO_CERT:				return ErrMsg(env, "CERT_NO_CERT");
	default:									return enif_make_tuple2(env, ErrMsg(env, "STATUS_CODE"), enif_make_int64(env, status));
	}

	return ErrMsg(env, "UNKNOWN");
}

#endif
