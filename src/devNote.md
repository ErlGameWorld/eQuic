QUIC_PARAM_CONN_DISABLE_1RTT_ENCRYPTION   应用程序必须#define QUIC_API_ENABLE_INSECURE_FEATURES在包含 msquic.h 之前。
QUIC_PARAM_CONN_DATAGRAM_RECEIVE_ENABLED  指示/查询对 QUIC 数据报扩展的支持。必须在start前设置。

QUIC_PARAM_CONN_SHARE_UDP_BINDING         仅在客户端上设置。必须在start前调用。
QUIC_PARAM_CONN_REMOTE_ADDRESS            仅在客户端上设置。必须在start前设置。  
QUIC_PARAM_CONN_LOCAL_ADDRESS             仅在客户端上设置。必须在开始前或握手确认后设置。 

QUIC_PARAM_CONN_RESUMPTION_TICKET         必须在start之前在客户端上设置。