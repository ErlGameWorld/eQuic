-ifndef(eQuic_H).
-define(eQuic_H, true).

%% IF-DO表达式
-define(IF(IFTure, DoThat), (IFTure) andalso (DoThat)).

%% 三目元算符
-define(IIF(Cond, Then, That), case Cond of true -> Then; _ -> That end).
-define(IIF(Expr, Expect, Then, ExprRet, That), case Expr of Expect -> Then; ExprRet -> That end).

-define(paramCfg, paramCfg).

-define(LimitExecutionProfile, [0, 1, 2, 3]).
-define(DefeExecutionProfile, 0).
-define(DefAlpn, ["eQuic"]).
-define(DefeSettings, #{}).
-define(DefeCredentialSrv, #{eType => 4, eFlags =>0, eCertificateFile_PrivateKeyFile => "./priv/key.pem", eCertificateFile_CertificateFile => "./priv/cert.pem"}).
-define(DefeCredentialCli, #{eType => 0, eFlags =>1}).

-export_type([quicSettings/0, quicCredential/0]).

-type uint8() :: 0 .. 255.
-type uint16() :: 0 .. 65535.
-type uint32() :: 0 .. 294967295.
-type uint64() :: 0 .. 18446744073709551615.

%% quic配置 key 为 QUIC_SETTINGS 字段加了前缀 'e'
%% msquic 配置版本 2.1.0,  quic配置详情请看: https://github.com/microsoft/msquic/blob/main/docs/Settings.md https://github.com/microsoft/msquic/blob/main/docs/api/QUIC_SETTINGS.md
-type quicSettings() :: #{
   eMaxBytesPerKey => uint64()                              %%在启动密钥更新之前使用单个 1-RTT 加密密钥加密的最大字节数。 默认值： 274,877,906,944
   , eHandshakeIdleTimeoutMs => uint64()                    %% 握手在被丢弃之前可以空闲多长时间。 默认值： 10,000
   , eIdleTimeoutMs => uint64()                             %% 连接在正常关闭之前可以空闲多长时间。0 禁用超时。默认值： 30,000
   , eMtuDiscoverySearchCompleteTimeoutUs => uint64()       %% 如果未达到最大值，则在重新尝试 MTU 探测之前等待的时间（以微秒为单位）。 默认值： 600000000
   , eTlsClientMaxSendBuffer => uint32()                    %% 要缓冲多少客户端 TLS 数据。如果应用程序需要较大的客户端证书或较长的客户端证书链，则应增加此值。默认值： 4,096
   , eTlsServerMaxSendBuffer => uint32()                    %% 要缓冲多少服务器 TLS 数据。如果应用程序需要非常大的服务器证书或很长的服务器证书链，则应增加此值。默认值： 8,192
   , eStreamRecvWindowDefault => uint32()                   %% 初始流接收窗口大小。默认值： 32,768
   , eStreamRecvBufferDefault => uint32()                   %% 流初始缓冲区大小。 默认值： 4,096
   , eConnFlowControlWindow => uint32()                     %% 连接范围的流量控制窗口。 默认值： 16,777,216
   , eMaxWorkerQueueDelayUs => uint32()                     %% 工作线程允许的最大队列延迟（以微秒为单位）。这会影响丢失检测和探测超时。 默认值： 250,000
   , eMaxStatelessOperations => uint32()                    %% 任何时候可以在工作人员上排队的无状态操作的最大数量。 默认值： 16
   , eInitialWindowPackets => uint32()                      %% 连接的初始拥塞窗口的大小（以数据包为单位）。  默认值： 10
   , eSendIdleTimeoutMs => uint32()                         %% 空闲毫秒后重置拥塞控制。  默认值： 1,000
   , eInitialRttMs => uint32()                              %% 初始 RTT 估计。 默认值： 333
   , eMaxAckDelayMs => uint32()                             %% 收到数据后等待多长时间再发送 ACK。这控制批量发送 ACK，以更少的开销获得更高的吞吐量。太长会导致对端重传，太短会浪费发送 ACK。 默认值： 25
   , eDisconnectTimeoutMs => uint32()                       %% 在宣布路径死亡和断开连接之前等待 ACK 的时间。 默认值： 16,000
   , eKeepAliveIntervalMs => uint32()                       %% 多久发送一次 PING 帧以保持连接处于活动状态。这也有助于防止 NAT 表条目过期。 默认值： 0（禁用）
   , eDestCidUpdateIdleTimeoutMs => uint32()                %% 同一端点的操作之间的时间限制，以毫秒为单位。
   , eCongestionControlAlgorithm => uint16()                %% QUIC_CONGESTION_CONTROL_ALGORITHM
   , ePeerBidiStreamCount => uint16()                       %% 允许对等方打开的双向流的数量。必须非零以允许对等方完全打开任何流。默认值： 0
   , ePeerUnidiStreamCount => uint16()                      %% 允许对等方打开的单向流的数量。必须非零以允许对等方完全打开任何流。默认值： 0
   , eMaxBindingStatelessOperations => uint16()             %% 任何时候可以在绑定上排队的无状态操作的最大数量。 默认值： 100
   , eStatelessOperationExpirationMs => uint16()            %% 同一端点的操作之间的时间限制，以毫秒为单位。 默认值： 100
   , eMinimumMtu => uint16()                                %% 连接支持的最小 MTU。这将用作起始 MTU。 默认值： 1248
   , eMaximumMtu => uint16()                                %% 连接支持的最大 MTU。这将是最大探测值。 默认值： 1500
   , eSendBufferingEnabled => uint8()                       %% 在 MsQuic 中缓冲发送数据，而不是在确认发送数据之前保留应用程序缓冲区。  默认值： 1 ( TRUE)
   , ePacingEnabled => uint8()                              %% 加快发送速度以避免路径上的缓冲区过满。 默认值： 1 ( TRUE)
   , eMigrationEnabled => uint8()                           %% 使客户端能够迁移 IP 地址和元组。要求服务器位于协作负载均衡器之后，或者没有负载均衡器。 默认值： 1 ( TRUE)
   , eDatagramReceiveEnabled => uint8()                     %% 宣传对 QUIC 数据报扩展的支持。连接的双方都需要将其设置TRUE为DatagramSend才能正常工作并受支持。 默认值： 0 ( FALSE)
   , eServerResumptionLevel => uint8()  %% QUIC_SERVER_RESUMPTION_LEVEL  %% 仅限服务器。控制恢复票证和/或 0-RTT 服务器支持。QUIC_SERVER_RESUME_ONLY启用发送和接收 TLS 恢复票证。服务器应用程序必须调用ConnectionSendResumptionTicket向客户端发送恢复票证。QUIC_SERVER_RESUME_AND_ZERORTT启用发送和接收 TLS 恢复票证并生成 0-RTT 密钥和接收 0-RTT 有效负载。服务器应用程序可以单独决定接受/拒绝每个 0-RTT 有效负载。 默认值：（ QUIC_SERVER_NO_RESUME禁用）
   , eMaxOperationsPerDrain => uint8()                      %% 每个连接量子消耗的最大操作数。 默认值： 16
   , eMtuDiscoveryMissingProbeCount => uint8()              %% 在退出 MTU 探测之前要重试的 MTU 探测次数。 默认值： 3
   , eGreaseQuicBitEnabled => uint8()
}.

%% typedef enum QUIC_CREDENTIAL_TYPE {
%%    QUIC_CREDENTIAL_TYPE_NONE,
%%    QUIC_CREDENTIAL_TYPE_CERTIFICATE_HASH,
%%    QUIC_CREDENTIAL_TYPE_CERTIFICATE_HASH_STORE,
%%    QUIC_CREDENTIAL_TYPE_CERTIFICATE_CONTEXT,
%%    QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE,
%%    QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE_PROTECTED,
%%    QUIC_CREDENTIAL_TYPE_CERTIFICATE_PKCS12,
%%} QUIC_CREDENTIAL_TYPE;
%%
%%typedef enum QUIC_CREDENTIAL_FLAGS {
%%    QUIC_CREDENTIAL_FLAG_NONE                                   = 0x00000000,
%%    QUIC_CREDENTIAL_FLAG_CLIENT                                 = 0x00000001, // Lack of client flag indicates server.
%%    QUIC_CREDENTIAL_FLAG_LOAD_ASYNCHRONOUS                      = 0x00000002,
%%    QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION              = 0x00000004,
%%    QUIC_CREDENTIAL_FLAG_ENABLE_OCSP                            = 0x00000008, // Schannel only currently
%%    QUIC_CREDENTIAL_FLAG_INDICATE_CERTIFICATE_RECEIVED          = 0x00000010,
%%    QUIC_CREDENTIAL_FLAG_DEFER_CERTIFICATE_VALIDATION           = 0x00000020,
%%    QUIC_CREDENTIAL_FLAG_REQUIRE_CLIENT_AUTHENTICATION          = 0x00000040,
%%    QUIC_CREDENTIAL_FLAG_USE_TLS_BUILTIN_CERTIFICATE_VALIDATION = 0x00000080, // OpenSSL only currently
%%    QUIC_CREDENTIAL_FLAG_REVOCATION_CHECK_END_CERT              = 0x00000100, // Schannel only currently
%%    QUIC_CREDENTIAL_FLAG_REVOCATION_CHECK_CHAIN                 = 0x00000200, // Schannel only currently
%%    QUIC_CREDENTIAL_FLAG_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT    = 0x00000400, // Schannel only currently
%%    QUIC_CREDENTIAL_FLAG_IGNORE_NO_REVOCATION_CHECK             = 0x00000800, // Schannel only currently
%%    QUIC_CREDENTIAL_FLAG_IGNORE_REVOCATION_OFFLINE              = 0x00001000, // Schannel only currently
%%    QUIC_CREDENTIAL_FLAG_SET_ALLOWED_CIPHER_SUITES              = 0x00002000,
%%    QUIC_CREDENTIAL_FLAG_USE_PORTABLE_CERTIFICATES              = 0x00004000,
%%    QUIC_CREDENTIAL_FLAG_USE_SUPPLIED_CREDENTIALS               = 0x00008000, // Schannel only
%%    QUIC_CREDENTIAL_FLAG_USE_SYSTEM_MAPPER                      = 0x00010000, // Schannel only
%%    QUIC_CREDENTIAL_FLAG_CACHE_ONLY_URL_RETRIEVAL               = 0x00020000, // Windows only currently
%%    QUIC_CREDENTIAL_FLAG_REVOCATION_CHECK_CACHE_ONLY            = 0x00040000, // Windows only currently
%%} QUIC_CREDENTIAL_FLAGS;

%% typedef struct QUIC_CREDENTIAL_CONFIG {
%%    QUIC_CREDENTIAL_TYPE Type;
%%    QUIC_CREDENTIAL_FLAGS Flags;
%%    union {
%%        QUIC_CERTIFICATE_HASH* CertificateHash;
%%        QUIC_CERTIFICATE_HASH_STORE* CertificateHashStore;
%%        QUIC_CERTIFICATE* CertificateContext;
%%        QUIC_CERTIFICATE_FILE* CertificateFile;
%%        QUIC_CERTIFICATE_FILE_PROTECTED* CertificateFileProtected;
%%        QUIC_CERTIFICATE_PKCS12* CertificatePkcs12;
%%    };
%%    const char* Principal;
%%    void* Reserved; // Currently unused
%%    QUIC_CREDENTIAL_LOAD_COMPLETE_HANDLER AsyncHandler; // Optional
%%    QUIC_ALLOWED_CIPHER_SUITE_FLAGS AllowedCipherSuites;// Optional
%%} QUIC_CREDENTIAL_CONFIG;

%% Principal 通过主体名称选择证书的主体名称字符串。仅 Schannel 支持。
%% AsyncHandler 接收异步凭据加载完成的可选回调。仅与QUIC_CREDENTIAL_FLAG_LOAD_ASYNCHRONOUS标志一起使用。
%% AllowedCipherSuites 一组标志，指示哪些密码套件可用于协商。必须与QUIC_CREDENTIAL_FLAG_SET_ALLOWED_CIPHER_SUITES.

%% 配置详情请看：https://github.com/microsoft/msquic/blob/main/docs/api/QUIC_CREDENTIAL_CONFIG.md
-type quicCredential() :: #{
   eType := uint8()                                         %% 指示表示哪种类型的凭据。 具体说明如下
      %% QUIC_CREDENTIAL_TYPE_NONE 0  仅对客户有效。不提供客户端身份验证。
      %%QUIC_CREDENTIAL_TYPE_CERTIFICATE_HASH 1  %%在 Windows 当前用户（内核模式的本地计算机）我的证书存储中搜索成员指向的证书指纹CertificateHash。仅在带有 Schannel 的 Windows 上有效。
      %%QUIC_CREDENTIAL_TYPE_CERTIFICATE_HASH_STORE 2 在 Windows 当前用户（内核模式的本地计算机）证书存储中搜索提供的存储名称和成员指向的证书指纹CertificateHashStore。仅在带有 Schannel 的 Windows 上有效。
      %%QUIC_CREDENTIAL_TYPE_CERTIFICATE_CONTEXT 3 提供带有证书的 Windows CAPICERTIFICATE_CONTEXT以在CertificateContext成员中使用。仅在用户模式下的 Windows 上有效。
      %%QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE 4 提供成员指向的 PEM 格式的私钥文件和 PEM 或 CER 格式的证书文件的文件路径CertificateFile。仅对 OpenSSL 有效。
      %%QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE_PROTECTED 5 提供受保护的私钥文件和证书文件的文件路径，以及用于取消保护私钥的密码，由CertificateFileProtected成员指定。仅对 OpenSSL 有效。
      %%QUIC_CREDENTIAL_TYPE_CERTIFICATE_PKCS12 6  提供 PKCS12 (PFX) 证书和私有的内存中 ASN.1 blob，以及成员指向的可选私钥密码CertificatePkcs12。目前不支持。

   , eFlags := uint64()                                      %% 以下标志的任意组合会更改凭证行为。 具体说明如下
      %% QUIC_CREDENTIAL_FLAG_NONE 在默认配置中与服务器一起使用。
      %%QUIC_CREDENTIAL_FLAG_CLIENT  此标志的存在表明这是一个客户端。缺席表示服务器。
      %%QUIC_CREDENTIAL_FLAG_LOAD_ASYNCHRONOUS  QUIC_STATUS_PENDING立即从ConfigurationLoadCredential返回并异步加载凭据。通过AsyncHandler回调指示完成。
      %%QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION  向 TLS 层指示不执行服务器证书验证。这很危险；不要在生产中使用
      %%QUIC_CREDENTIAL_FLAG_ENABLE_OCSP  为此连接启用 OCSP 装订。仅对 Schannel 有效。
      %%QUIC_CREDENTIAL_FLAG_INDICATE_CERTIFICATE_RECEIVED
      %%QUIC_CONNECTION_EVENT_PEER_CERTIFICATE_RECEIVED从对等方（客户端或服务器）收到证书时接收事件。
      %%QUIC_CREDENTIAL_FLAG_DEFER_CERTIFICATE_VALIDATION  请求 TLS 层对收到的证书进行验证，并通过QUIC_CONNECTION_EVENT_PEER_CERTIFICATE_RECEIVED事件将结果提供给应用程序，并允许应用程序覆盖失败的验证。仅 Schannel 支持。也需要QUIC_CREDENTIAL_FLAG_INDICATE_CERTIFICATE_RECEIVED设置。
      %%QUIC_CREDENTIAL_FLAG_REQUIRE_CLIENT_AUTHENTICATION  要求客户端提供身份验证以使握手成功。客户端不支持。
      %%QUIC_CREDENTIAL_FLAG_USE_TLS_BUILTIN_CERTIFICATE_VALIDATION 使用内置 TLS 库的证书验证，而不是平台的证书验证。这在非 Windows 系统上默认启用，并且仅在使用 OpenSSL 时对 Windows 有效。
      %%QUIC_CREDENTIAL_FLAG_REVOCATION_CHECK_END_CERT  仅检查叶证书以进行吊销。仅在 Windows 上有效。
      %%QUIC_CREDENTIAL_FLAG_REVOCATION_CHECK_CHAIN  检查链中的每个证书是否被吊销。仅在 Windows 上有效。
      %%QUIC_CREDENTIAL_FLAG_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT  检查链中的每个证书（根证书除外）是否被吊销。仅在 Windows 上有效。
      %%QUIC_CREDENTIAL_FLAG_IGNORE_NO_REVOCATION_CHECK  忽略不执行吊销检查的错误。仅在 Windows 上有效。
      %%QUIC_CREDENTIAL_FLAG_IGNORE_REVOCATION_OFFLINE 忽略吊销脱机失败。仅在 Windows 上有效。
      %%QUIC_CREDENTIAL_FLAG_SET_ALLOWED_CIPHER_SUITES  启用哪些密码套件可通过AllowedCipherSuites成员进行协商。
      %%QUIC_CREDENTIAL_FLAG_USE_PORTABLE_CERTIFICATES 在事件中将远程 X.509 证书作为 DER（二进制）blob 和整个证书链作为 PKCS #7 DER blob 提供给应用程序QUIC_CONNECTION_EVENT_PEER_CERTIFICATE_RECEIVED。内核模式不支持。
      %%QUIC_CREDENTIAL_FLAG_USE_SUPPLIED_CREDENTIALS  告诉 TLS 层（目前仅受 Schannel 支持）仅使用提供的客户端证书，如果服务器要求证书但客户端应用程序未提供证书，则不自行寻找证书。更多信息可以在这里找到。
      %%QUIC_CREDENTIAL_FLAG_USE_SYSTEM_MAPPER  告诉 TLS 层（仅 Schannel 服务器支持）使用系统凭据映射器将客户端提供的凭据映射到系统上的用户帐户。
      %%QUIC_CREDENTIAL_FLAG_CACHE_ONLY_URL_RETRIEVAL  仅在进行 URL 检索时使用已缓存的证书来构建证书链。仅在 Windows 上有效。
      %%QUIC_CREDENTIAL_FLAG_REVOCATION_CHECK_CACHE_ONLY  检查证书链时仅使用缓存的吊销信息。仅在 Windows 上有效。
      %%QUIC_CREDENTIAL_FLAG_INPROC_PEER_CERTIFICATE 0x00004000  使用更快的进程内 API 调用获取对等证书。仅在最新 Windows 11 版本中的 Schannel 上可用。

   %% CertificateHash  只能与类型一起使用。QUIC_CREDENTIAL_TYPE_CERTIFICATE_HASH
   , eCertificateHash_ShaHash => [uint8(), ...] %% len 20

   %% CertificateContext 只能与类型一起使用。QUIC_CREDENTIAL_TYPE_CERTIFICATE_CONTEXT
   %% 不支持

   %% CertificateHashStore 只能与类型一起使用。QUIC_CREDENTIAL_TYPE_CERTIFICATE_HASH_STORE
   , eCertificateHashStore_Flags => uint32() %% len 20
   , eCertificateHashStore_ShaHash => [uint8(), ...] %% len 20
   , eCertificateHashStore_StoreName => [char(), ...]

   %% CertificateFile 只能与类型一起使用。QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE
   , eCertificateFile_PrivateKeyFile => string()
   , eCertificateFile_CertificateFile => string()

   %% CertificateFile 只能与类型一起使用。QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE_PROTECTED
   , eCertificateFileProtected_PrivateKeyFile => string()
   , eCertificateFileProtected_CertificateFile => string()
   , eCertificateFileProtected_PrivateKeyPassword => string()

   %% CertificateFile 只能与类型一起使用。QUIC_CREDENTIAL_TYPE_CERTIFICATE_PKCS12
   , eCertificatePkcs12_Asn1Blob => [uint8(), ...]
   , eCertificatePkcs12_Asn1BlobLength => uint32()
   , eCertificatePkcs12_PrivateKeyPassword => string()
}.

%% quic status see: // https://github.com/microsoft/msquic/blob/main/docs/api/QUIC_STATUS.md

-type quciMsg() ::
   {quicLStart, Ref :: reference()} |
   {quicLClose, Ref :: reference()} |
   {quicCNew, Ref :: reference()} |
   {quicCInfo, Type :: atom(), Term :: term()} |
   {quicSNew, Ref :: reference()}.

-type listenAddr() :: string().
-type listenPort() :: 0 .. 65536.
-type listenOpts() :: #{eCfgRef => reference(), eAcceptorCnt := pos_integer(), eAlpn := [string(), ...]}.

-type connOpts() :: #{}.

-endif.