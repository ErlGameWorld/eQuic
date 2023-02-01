#ifndef __QUIC_ATOM_H_
#define __QUIC_ATOM_H_

#include<erl_nif.h>

#define NIF_ATOM_DECL(a) ERL_NIF_TERM atom_ ## a;
#define NIF_ATOM_INIT(a) atom_ ## a = enif_make_atom(env, #a);

#define NIF_ATOMS(A) \
    /* atom of common */ \
    A(ok) \
    A(quic) \
    A(error) \
    A(true) \
    A(false) \
    A(undefined) \
    A(freed) \
    /* atom of setting */    \
    A(eMaxBytesPerKey) \
    A(eHandshakeIdleTimeoutMs) \
    A(eIdleTimeoutMs) \
    A(eMtuDiscoverySearchCompleteTimeoutUs) \
    A(eTlsClientMaxSendBuffer) \
    A(eTlsServerMaxSendBuffer) \
    A(eStreamRecvWindowDefault) \
    A(eStreamRecvBufferDefault) \
    A(eConnFlowControlWindow) \
    A(eMaxWorkerQueueDelayUs) \
    A(eMaxStatelessOperations) \
    A(eInitialWindowPackets) \
    A(eSendIdleTimeoutMs) \
    A(eInitialRttMs) \
    A(eMaxAckDelayMs) \
    A(eDisconnectTimeoutMs) \
    A(eKeepAliveIntervalMs) \
    A(eDestCidUpdateIdleTimeoutMs) \
    A(eCongestionControlAlgorithm) \
    A(ePeerBidiStreamCount) \
    A(ePeerUnidiStreamCount) \
    A(eMaxBindingStatelessOperations) \
    A(eStatelessOperationExpirationMs) \
    A(eMinimumMtu) \
    A(eMaximumMtu) \
    A(eSendBufferingEnabled) \
    A(ePacingEnabled) \
    A(eMigrationEnabled) \
    A(eDatagramReceiveEnabled) \
    A(eServerResumptionLevel) \
    A(eGreaseQuicBitEnabled) \
    A(eMaxOperationsPerDrain) \
    A(eMtuDiscoveryMissingProbeCount)\
    /* Credential */ \
    A(eType) \
    A(eFlags) \
    A(eCertificateHash_ShaHash) \
    A(eCertificateHashStore_Flags) \
    A(eCertificateHashStore_ShaHash) \
    A(eCertificateHashStore_StoreName) \
    A(eCertificateFile_PrivateKeyFile) \
    A(eCertificateFile_CertificateFile) \
    A(eCertificateFileProtected_PrivateKeyFile) \
    A(eCertificateFileProtected_CertificateFile) \
    A(eCertificateFileProtected_PrivateKeyPassword) \
    A(eCertificatePkcs12_Asn1Blob) \
    A(eCertificatePkcs12_Asn1BlobLength) \
    A(eCertificatePkcs12_PrivateKeyPassword) \
    A(eCfgRef) \
    A(eAcceptorCnt) \
    A(eAlpn) \
    /* atom of msg tag */ \
    A(stopByPeer) \
    A(stopByTransport) \
    A(quicLStart) \
    A(quicLClose) \
    A(quicCNew) \
    A(quicCInfo) \
    A(quicCStop) \
    A(quicCClose) \
    A(quicSNew) \
    A(localAddrChanged) \
    A(peerAddrChanged)  \
    A(streamsAvailable)  \
    A(peerNeedsStreams) \
    A(idealProcessorChanged) \
    A(datagramStateChanged) \
    A(datagramSendStateChanged) \
    A(resumptionTicketReceived) \
    A(peerCertificateReceived) \
    A(datagamRecv)

NIF_ATOMS(NIF_ATOM_DECL)

#endif
