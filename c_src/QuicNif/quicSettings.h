#ifndef __QUIC_SETTINGS_H_
#define __QUIC_SETTINGS_H_

#include<erl_nif.h>
#include<msquic.h>

bool getUint16FormTerm(ErlNifEnv* env, const ERL_NIF_TERM Term, uint16_t* Value) {
    unsigned TemValue = 0;
    if (!enif_get_uint(env, Term, &TemValue))
        return false;

    if (TemValue > UINT16_MAX)
        return false;

    *Value = (uint16_t)TemValue;
    return true;
}

bool getUint8FromMap(ErlNifEnv* env, const ERL_NIF_TERM Map, ERL_NIF_TERM Key, uint8_t* Value) {
    ERL_NIF_TERM TValue;
    if (!enif_get_map_value(env, Map, Key, &TValue))
        return false;

    unsigned TemValue = 0;

    if (!enif_get_uint(env, TValue, &TemValue))
        return false;

    if (TemValue > UINT8_MAX)
        return false;

    *Value = (uint8_t)TemValue;
    return true;
}

bool getUint16FromMap(ErlNifEnv* env, const ERL_NIF_TERM Map, ERL_NIF_TERM Key, uint16_t* Value) {
    ERL_NIF_TERM TValue;
    if (!enif_get_map_value(env, Map, Key, &TValue))
        return false;

    unsigned TemValue = 0;
    if (!enif_get_uint(env, TValue, &TemValue))
        return false;

    if (TemValue > UINT16_MAX)
        return false;

    *Value = (uint16_t)TemValue;
    return true;
}

bool getUint32FromMap(ErlNifEnv* env, const ERL_NIF_TERM Map, ERL_NIF_TERM Key, uint32_t* Value) {
    ERL_NIF_TERM TValue;
    if (!enif_get_map_value(env, Map, Key, &TValue))
        return false;

    unsigned TemValue = 0;
    if (!enif_get_uint(env, TValue, &TemValue))
        return false;

    *Value = (uint32_t)TemValue;
    return true;
}

bool getUint64FromMap(ErlNifEnv* env, const ERL_NIF_TERM Map, ERL_NIF_TERM Key, uint64_t* Value) {
    ERL_NIF_TERM TValue;
    if (!enif_get_map_value(env, Map, Key, &TValue))
        return false;

    ErlNifUInt64 TemValue = 0;
    if (!enif_get_uint64(env, TValue, &TemValue))
        return false;

    *Value = (uint64_t)TemValue;
    return true;
}

// the return ptr need enif_free after use
char* getStrFromMap(ErlNifEnv* env, const ERL_NIF_TERM Map, ERL_NIF_TERM Key) {
    ERL_NIF_TERM StrTerm;
    if (!enif_get_map_value(env, Map, Key, &StrTerm))
        return NULL;

    unsigned ListLen;
    if (!enif_get_list_length(env, StrTerm, &ListLen))
        return NULL;

    char* StrPtr = (char*)enif_alloc(ListLen + 1);
    if (!enif_get_string(env, StrTerm, StrPtr, ListLen + 1, ERL_NIF_LATIN1)) {
        enif_free(StrPtr);
        return NULL;
    }

    return StrPtr;
}

// the return ptr need enif_free after use
char* getStrFromTrem(ErlNifEnv* env, const ERL_NIF_TERM StrTerm) {
    unsigned ListLen;
    if (!enif_get_list_length(env, StrTerm, &ListLen))
        return NULL;

    char* StrPtr = (char*)enif_alloc(ListLen + 1);
    if (!enif_get_string(env, StrTerm, StrPtr, ListLen + 1, ERL_NIF_LATIN1)) {
        enif_free(StrPtr);
        return NULL;
    }

    return StrPtr;
}

bool getUint8ArrayFromMap(ErlNifEnv* env, const ERL_NIF_TERM Map, ERL_NIF_TERM Key, uint8_t* Array, unsigned MaxLen) {
    ERL_NIF_TERM List;

    if (!enif_get_map_value(env, Map, Key, &List))
        return false;

    unsigned ListLen;
    if (!enif_get_list_length(env, List, &ListLen))
        return false;

    if (ListLen > MaxLen)
        return false;

    unsigned int TempUint;
    unsigned int i = 0;
    ERL_NIF_TERM Head;
    while (enif_get_list_cell(env, List, &Head, &List)) {
        if (!enif_get_uint(env, Head, &TempUint)) {
            return false;
        }
        Array[i] = (uint8_t)TempUint;
        ++i;
    }
    return true;
}

bool getCharArrayFromMap(ErlNifEnv* env, const ERL_NIF_TERM Map, ERL_NIF_TERM Key, char* Array, unsigned MaxLen) {
    ERL_NIF_TERM List;

    if (!enif_get_map_value(env, Map, Key, &List))
        return false;

    unsigned ListLen;
    if (!enif_get_list_length(env, List, &ListLen))
        return false;

    if (ListLen > MaxLen)
        return false;

    int TempUint;
    unsigned int i = 0;
    ERL_NIF_TERM Head;
    while (enif_get_list_cell(env, List, &Head, &List)) {
        if (!enif_get_int(env, Head, &TempUint)) {
            return false;
        }
        Array[i] = (char)TempUint;
        ++i;
    }
    return true;
}

bool loadApln(ErlNifEnv* env, const ERL_NIF_TERM AlpnList, QUIC_BUFFER* AlpnBuffers, unsigned* AlpnSetCnt) {
    unsigned int i = 0;
    unsigned AlpnLen = 0;
    ERL_NIF_TERM List = AlpnList;
    ERL_NIF_TERM Head;
    enif_fprintf(stdout, "IMY************loadApln0000 %T\n", AlpnList);
    while (enif_get_list_cell(env, List, &Head, &List)) {
        int ret = enif_get_list_length(env, Head, &AlpnLen);
        enif_fprintf(stdout, "IMY************loadApln11111 %d %d %T\n", ret, AlpnLen, Head);
        if (!enif_get_list_length(env, Head, &AlpnLen)) {
            enif_fprintf(stdout, "IMY************loadApln1222 %d %d %T\n", ret, AlpnLen, Head);
            return false;
        }
        enif_fprintf(stdout, "IMY************loadApln13333 %d %d %T\n", ret, AlpnLen, Head);
        char* AlpnPtr = (char*)enif_alloc(AlpnLen + 1);
        if (!enif_get_string(env, Head, AlpnPtr, AlpnLen + 1, ERL_NIF_LATIN1)) {
            return false;
        }
        enif_fprintf(stdout, "IMY************loadApln1444 %d %d %T\n", ret, AlpnLen, Head);

        QUIC_BUFFER AlpnBuffer = { AlpnLen, (uint8_t*)AlpnPtr };
        enif_fprintf(stdout, "IMY************loadApln15555 %d %d %T\n", ret, AlpnLen, Head);
        AlpnBuffers[i] = AlpnBuffer;
        ++i;
        ++(*AlpnSetCnt);
    }
    enif_fprintf(stdout, "IMY************loadApln16666 %d %T\n", AlpnLen, Head);
    return true;
}

void freeApln(QUIC_BUFFER* AlpnBuffers, unsigned AlpnSetCnt) {
    QUIC_BUFFER OneAlpn;
    for (unsigned i = 0; i < AlpnSetCnt; ++i) {
        OneAlpn = AlpnBuffers[i];
        enif_fprintf(stdout, "IMY************freeApln11111 %d \n", AlpnSetCnt);
        enif_free(OneAlpn.Buffer);
    }
    enif_fprintf(stdout, "IMY************freeApln2222 %d \n", AlpnSetCnt);
    enif_free(AlpnBuffers);
    enif_fprintf(stdout, "IMY************freeApln3333 %d \n", AlpnSetCnt);
}

// the settings of msquic version: 2.1.0
bool createSettings(ErlNifEnv* env, const ERL_NIF_TERM SettingMap, QUIC_SETTINGS* Settings) {
    if (!enif_is_map(env, SettingMap))
        return false;

    if (getUint64FromMap(env, SettingMap, atom_eMaxBytesPerKey, &Settings->MaxBytesPerKey))
        Settings->IsSet.MaxBytesPerKey = TRUE;

    if (getUint64FromMap(env, SettingMap, atom_eHandshakeIdleTimeoutMs, &Settings->HandshakeIdleTimeoutMs))
        Settings->IsSet.HandshakeIdleTimeoutMs = TRUE;

    if (getUint64FromMap(env, SettingMap, atom_eIdleTimeoutMs, &Settings->IdleTimeoutMs))
        Settings->IsSet.IdleTimeoutMs = TRUE;

    if (getUint64FromMap(env, SettingMap, atom_eMtuDiscoverySearchCompleteTimeoutUs, &Settings->MtuDiscoverySearchCompleteTimeoutUs))
        Settings->IsSet.MtuDiscoverySearchCompleteTimeoutUs = TRUE;

    if (getUint32FromMap(env, SettingMap, atom_eTlsClientMaxSendBuffer, &Settings->TlsClientMaxSendBuffer))
        Settings->IsSet.TlsClientMaxSendBuffer = TRUE;

    if (getUint32FromMap(env, SettingMap, atom_eTlsServerMaxSendBuffer, &Settings->TlsServerMaxSendBuffer))
        Settings->IsSet.TlsServerMaxSendBuffer = TRUE;

    if (getUint32FromMap(env, SettingMap, atom_eStreamRecvWindowDefault, &Settings->StreamRecvWindowDefault))
        Settings->IsSet.StreamRecvWindowDefault = TRUE;

    if (getUint32FromMap(env, SettingMap, atom_eStreamRecvBufferDefault, &Settings->StreamRecvBufferDefault))
        Settings->IsSet.StreamRecvBufferDefault = TRUE;

    if (getUint32FromMap(env, SettingMap, atom_eConnFlowControlWindow, &Settings->ConnFlowControlWindow))
        Settings->IsSet.ConnFlowControlWindow = TRUE;

    if (getUint32FromMap(env, SettingMap, atom_eMaxWorkerQueueDelayUs, &Settings->MaxWorkerQueueDelayUs))
        Settings->IsSet.MaxWorkerQueueDelayUs = TRUE;

    if (getUint32FromMap(env, SettingMap, atom_eMaxStatelessOperations, &Settings->MaxStatelessOperations))
        Settings->IsSet.MaxStatelessOperations = TRUE;

    if (getUint32FromMap(env, SettingMap, atom_eInitialWindowPackets, &Settings->InitialWindowPackets))
        Settings->IsSet.InitialWindowPackets = TRUE;

    if (getUint32FromMap(env, SettingMap, atom_eSendIdleTimeoutMs, &Settings->SendIdleTimeoutMs))
        Settings->IsSet.SendIdleTimeoutMs = TRUE;

    if (getUint32FromMap(env, SettingMap, atom_eInitialRttMs, &Settings->InitialRttMs))
        Settings->IsSet.InitialRttMs = TRUE;

    if (getUint32FromMap(env, SettingMap, atom_eMaxAckDelayMs, &Settings->MaxAckDelayMs))
        Settings->IsSet.MaxAckDelayMs = TRUE;

    if (getUint32FromMap(env, SettingMap, atom_eDisconnectTimeoutMs, &Settings->DisconnectTimeoutMs))
        Settings->IsSet.DisconnectTimeoutMs = TRUE;

    if (getUint32FromMap(env, SettingMap, atom_eKeepAliveIntervalMs, &Settings->KeepAliveIntervalMs))
        Settings->IsSet.KeepAliveIntervalMs = TRUE;

    if (getUint32FromMap(env, SettingMap, atom_eDestCidUpdateIdleTimeoutMs, &Settings->DestCidUpdateIdleTimeoutMs))
        Settings->IsSet.DestCidUpdateIdleTimeoutMs = TRUE;

    if (getUint16FromMap(env, SettingMap, atom_eCongestionControlAlgorithm, &Settings->CongestionControlAlgorithm))
        Settings->IsSet.CongestionControlAlgorithm = TRUE;

    if (getUint16FromMap(env, SettingMap, atom_ePeerBidiStreamCount, &Settings->PeerBidiStreamCount))
        Settings->IsSet.PeerBidiStreamCount = TRUE;

    if (getUint16FromMap(env, SettingMap, atom_ePeerUnidiStreamCount, &Settings->PeerUnidiStreamCount))
        Settings->IsSet.PeerUnidiStreamCount = TRUE;

    if (getUint16FromMap(env, SettingMap, atom_eMaxBindingStatelessOperations, &Settings->MaxBindingStatelessOperations))
        Settings->IsSet.MaxBindingStatelessOperations = TRUE;

    if (getUint16FromMap(env, SettingMap, atom_eStatelessOperationExpirationMs, &Settings->StatelessOperationExpirationMs))
        Settings->IsSet.StatelessOperationExpirationMs = TRUE;

    if (getUint16FromMap(env, SettingMap, atom_eMinimumMtu, &Settings->MinimumMtu))
        Settings->IsSet.MinimumMtu = TRUE;

    if (getUint16FromMap(env, SettingMap, atom_eMaximumMtu, &Settings->MaximumMtu)) {
        Settings->IsSet.MaximumMtu = TRUE;
    }

    uint8_t TempValue;

    if (getUint8FromMap(env, SettingMap, atom_eSendBufferingEnabled, &TempValue)) {
        Settings->SendBufferingEnabled = TempValue;
        Settings->IsSet.SendBufferingEnabled = TRUE;
    }

    if (getUint8FromMap(env, SettingMap, atom_ePacingEnabled, &TempValue)) {
        Settings->PacingEnabled = TempValue;
        Settings->IsSet.PacingEnabled = TRUE;
    }

    if (getUint8FromMap(env, SettingMap, atom_eMigrationEnabled, &TempValue)) {
        Settings->MigrationEnabled = TempValue;
        Settings->IsSet.MigrationEnabled = TRUE;
    }

    if (getUint8FromMap(env, SettingMap, atom_eDatagramReceiveEnabled, &TempValue)) {
        Settings->DatagramReceiveEnabled = TempValue;
        Settings->IsSet.DatagramReceiveEnabled = TRUE;
    }

    if (getUint8FromMap(env, SettingMap, atom_eServerResumptionLevel, &TempValue)) {
        Settings->ServerResumptionLevel = TempValue;
        Settings->IsSet.ServerResumptionLevel = TRUE;
    }

    if (getUint8FromMap(env, SettingMap, atom_eRESERVED, &TempValue)) {
        Settings->RESERVED = TempValue;
        Settings->IsSet.RESERVED = TRUE;
    }

    if (getUint8FromMap(env, SettingMap, atom_eMaxOperationsPerDrain, &Settings->MaxOperationsPerDrain))
        Settings->IsSet.MaxOperationsPerDrain = TRUE;

    if (getUint8FromMap(env, SettingMap, atom_eMtuDiscoveryMissingProbeCount, &Settings->MtuDiscoveryMissingProbeCount))
        Settings->IsSet.MtuDiscoveryMissingProbeCount = TRUE;

    return true;
}

bool createCredentialAndLoad(ErlNifEnv* env, HQUIC Configuration, const ERL_NIF_TERM CredentialMap) {
    if (!enif_is_map(env, CredentialMap))
        return false;
    enif_fprintf(stdout, "IMY*************createCredentialAndLoad0000%T\n", CredentialMap);
    uint64_t TempValue;
    QUIC_CREDENTIAL_CONFIG CredConfig;
    memset(&CredConfig, 0, sizeof(CredConfig));
    if (!getUint64FromMap(env, CredentialMap, atom_eType, &TempValue)) {
        return false;
    }
    CredConfig.Type = (QUIC_CREDENTIAL_TYPE)TempValue;

    if (!getUint64FromMap(env, CredentialMap, atom_eFlags, &TempValue)) {
        return false;
    }
    CredConfig.Flags = (QUIC_CREDENTIAL_FLAGS)TempValue;
    enif_fprintf(stdout, "IMY*************createCredentialAndLoad1111\n");
    char* Str1 = NULL;
    char* Str2 = NULL;
    char* Str3 = NULL;
    QUIC_STATUS Status = QUIC_STATUS_SUCCESS;
    bool Isok = true;

    switch (CredConfig.Type)
    {
    case QUIC_CREDENTIAL_TYPE_NONE:
        if (QUIC_FAILED(Status = MsQuic->ConfigurationLoadCredential(Configuration, &CredConfig)))
            Isok = false;
        break;
    case QUIC_CREDENTIAL_TYPE_CERTIFICATE_HASH: {
        QUIC_CERTIFICATE_HASH  CertificateHash = { 0 };
        if (!getUint8ArrayFromMap(env, CredentialMap, atom_eCertificateHash_ShaHash, CertificateHash.ShaHash, 20))
            return false;

        CredConfig.CertificateHash = &CertificateHash;
        if (QUIC_FAILED(Status = MsQuic->ConfigurationLoadCredential(Configuration, &CredConfig)))
            Isok = false;
        break; }

    case QUIC_CREDENTIAL_TYPE_CERTIFICATE_HASH_STORE: {
        QUIC_CERTIFICATE_HASH_STORE CertificateHashStore = { 0 };
        if (!getUint64FromMap(env, CredentialMap, atom_eCertificateHashStore_ShaHash, &TempValue))
            return false;
        if (!getUint8ArrayFromMap(env, CredentialMap, atom_eCertificateHashStore_ShaHash, CertificateHashStore.ShaHash, 20))
            return false;
        if (!getCharArrayFromMap(env, CredentialMap, atom_eCertificateHashStore_StoreName, CertificateHashStore.StoreName, 128))
            return false;
        CertificateHashStore.Flags = (uint32_t)TempValue;
        CredConfig.CertificateHashStore = &CertificateHashStore;

        if (QUIC_FAILED(Status = MsQuic->ConfigurationLoadCredential(Configuration, &CredConfig)))
            Isok = false;
        break; }

    case QUIC_CREDENTIAL_TYPE_CERTIFICATE_CONTEXT: {
        Str1 = getStrFromMap(env, CredentialMap, atom_eCertificateFile_PrivateKeyFile);
        if (NULL == Str1)
            return false;

        CredConfig.CertificateContext = (void*)Str1;
        if (QUIC_FAILED(Status = MsQuic->ConfigurationLoadCredential(Configuration, &CredConfig)))
            Isok = false;
        break; }
    case QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE: {
        enif_fprintf(stdout, "IMY*************createCredentialAndLoad4444\n");
        Str1 = getStrFromMap(env, CredentialMap, atom_eCertificateFile_PrivateKeyFile);
        if (NULL == Str1)
            return false;
        Str2 = getStrFromMap(env, CredentialMap, atom_eCertificateFile_CertificateFile);
        if (NULL == Str2) {
            return false;
        }

        QUIC_CERTIFICATE_FILE CertificateFile = { 0 };
        CertificateFile.PrivateKeyFile = Str1;
        CertificateFile.CertificateFile = Str2;
        enif_fprintf(stdout, "IMY*************createCredentialAndLoad44440000%s  %s\n", Str1, Str2);
        CredConfig.CertificateFile = &CertificateFile;
        enif_fprintf(stdout, "IMY*************createCredentialAndLoad444411111\n");

        if (QUIC_FAILED(Status = MsQuic->ConfigurationLoadCredential(Configuration, &CredConfig))) {
            enif_fprintf(stdout, "IMY*************createCredentialAndLoad444411111eeeee %x %x\n", Status, QUIC_FAILED(Status));
            Isok = false;
        }

        break; }

    case QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE_PROTECTED: {
        Str1 = getStrFromMap(env, CredentialMap, atom_eCertificateFileProtected_PrivateKeyFile);
        if (NULL == Str1)
            return false;
        Str2 = getStrFromMap(env, CredentialMap, atom_eCertificateFileProtected_CertificateFile);
        if (NULL == Str2) {
            return false;
        }
        Str3 = getStrFromMap(env, CredentialMap, atom_eCertificateFileProtected_PrivateKeyPassword);
        if (NULL == Str3) {
            return false;
        }
        QUIC_CERTIFICATE_FILE_PROTECTED CertificateFileProtected = { 0 };
        CertificateFileProtected.PrivateKeyFile = Str1;
        CertificateFileProtected.CertificateFile = Str2;
        CertificateFileProtected.PrivateKeyPassword = Str3;
        CredConfig.CertificateFileProtected = &CertificateFileProtected;

        if (QUIC_FAILED(Status = MsQuic->ConfigurationLoadCredential(Configuration, &CredConfig)))
            Isok = false;
        break;
    }

    case QUIC_CREDENTIAL_TYPE_CERTIFICATE_PKCS12: {
        QUIC_CERTIFICATE_PKCS12 CertificatePkcs12 = { 0 };
        if (!getUint32FromMap(env, CredentialMap, atom_eCertificatePkcs12_Asn1BlobLength, &CertificatePkcs12.Asn1BlobLength))
            return false;
        Str1 = getStrFromMap(env, CredentialMap, atom_eCertificatePkcs12_Asn1Blob);
        if (NULL == Str1)
            return false;
        Str2 = getStrFromMap(env, CredentialMap, atom_eCertificatePkcs12_PrivateKeyPassword);
        if (NULL == Str2) {
            return false;
        }
        CertificatePkcs12.Asn1Blob = (uint8_t*)Str1;
        CertificatePkcs12.PrivateKeyPassword = Str2;
        CredConfig.CertificatePkcs12 = &CertificatePkcs12;

        if (QUIC_FAILED(Status = MsQuic->ConfigurationLoadCredential(Configuration, &CredConfig)))
            Isok = false;
        break;
    }
    default:
        return false;
        break;
    }

    enif_fprintf(stdout, "IMY*************createCredentialAndLoad99999\n");

    enif_fprintf(stdout, "IMY*************createCredentialAndLoad999991111\n");
    if (NULL != Str1) enif_free(Str1);
    if (NULL != Str2) enif_free(Str2);
    if (NULL != Str3) enif_free(Str3);
    enif_fprintf(stdout, "IMY*************createCredentialAndLoad99999222\n");
    return Isok;
}

#endif
