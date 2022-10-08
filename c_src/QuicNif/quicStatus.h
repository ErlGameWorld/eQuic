#ifndef __QUIC_STATUS_H_
#define __QUIC_STATUS_H_

#include<erl_nif.h>
#include<msquic.h>

// https://github.com/microsoft/msquic/blob/main/docs/api/QUIC_STATUS.md
ERL_NIF_TERM status_info(ErlNifEnv* env, QUIC_STATUS status)
{
    ERL_NIF_TERM eterm;
    switch (status)
    {
    case QUIC_STATUS_SUCCESS:
        eterm = enif_make_string(env, "SUCCESS", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_PENDING:
        eterm = enif_make_string(env, "PENDING", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_CONTINUE:
        eterm = enif_make_string(env, "CONTINUE", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_OUT_OF_MEMORY:
        eterm = enif_make_string(env, "OUT_OF_MEMORY", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_INVALID_PARAMETER:
        eterm = enif_make_string(env, "INVALID_PARAMETER", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_INVALID_STATE:
        eterm = enif_make_string(env, "INVALID_STATE", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_NOT_SUPPORTED:
        eterm = enif_make_string(env, "NOT_SUPPORTED", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_NOT_FOUND:
        eterm = enif_make_string(env, "NOT_FOUND", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_BUFFER_TOO_SMALL:
        eterm = enif_make_string(env, "BUFFER_TOO_SMALL", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_HANDSHAKE_FAILURE:
        eterm = enif_make_string(env, "HANDSHAKE_FAILURE", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_ABORTED:
        eterm = enif_make_string(env, "ABORTED", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_ADDRESS_IN_USE:
        eterm = enif_make_string(env, "ADDRESS_IN_USE", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_INVALID_ADDRESS:
        eterm = enif_make_string(env, "INVALID_ADDRESS", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_CONNECTION_TIMEOUT:
        eterm = enif_make_string(env, "CONNECTION_TIMEOUT", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_CONNECTION_IDLE:
        eterm = enif_make_string(env, "CONNECTION_IDLE", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_INTERNAL_ERROR:
        eterm = enif_make_string(env, "INTERNAL_ERROR", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_UNREACHABLE:
        eterm = enif_make_string(env, "UNREACHABLE", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_CONNECTION_REFUSED:
        eterm = enif_make_string(env, "CONNECTION_REFUSED", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_PROTOCOL_ERROR:
        eterm = enif_make_string(env, "PROTOCOL_ERROR", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_VER_NEG_ERROR:
        eterm = enif_make_string(env, "VER_NEG_ERROR", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_TLS_ERROR:
        eterm = enif_make_string(env, "TLS_ERROR", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_USER_CANCELED:
        eterm = enif_make_string(env, "USER_CANCELED", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_ALPN_NEG_FAILURE:
        eterm = enif_make_string(env, "ALPN_NEG_FAILURE", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_STREAM_LIMIT_REACHED:
        eterm = enif_make_string(env, "LIMIT_REACHED", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_ALPN_IN_USE:
        eterm = enif_make_string(env, "ALPN_IN_USE", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_CLOSE_NOTIFY:
        eterm = enif_make_string(env, "CLOSE_NOTIFY", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_BAD_CERTIFICATE:
        eterm = enif_make_string(env, "BAD_CERTIFICATE", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_UNSUPPORTED_CERTIFICATE:
        eterm = enif_make_string(env, "UNSUPPORTED_CERTIFICATE", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_REVOKED_CERTIFICATE:
        eterm = enif_make_string(env, "REVOKED_CERTIFICATE", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_EXPIRED_CERTIFICATE:
        eterm = enif_make_string(env, "EXPIRED_CERTIFICATE", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_UNKNOWN_CERTIFICATE:
        eterm = enif_make_string(env, "UNKNOWN_CERTIFICATE", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_REQUIRED_CERTIFICATE:
        eterm = enif_make_string(env, "REQUIRED_CERTIFICATE", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_CERT_EXPIRED:
        eterm = enif_make_string(env, "CERT_EXPIRED", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_CERT_UNTRUSTED_ROOT:
        eterm = enif_make_string(env, "CERT_UNTRUSTED_ROOT", ERL_NIF_LATIN1);
        break;
    case QUIC_STATUS_CERT_NO_CERT:
        eterm = enif_make_string(env, "CERT_NO_CERT", ERL_NIF_LATIN1);
        break;
    default:
        eterm = enif_make_tuple2(env, enif_make_string(env, "STATUS_CODE", ERL_NIF_LATIN1), enif_make_int(env, status));
    }
    return eterm;
}

#endif
