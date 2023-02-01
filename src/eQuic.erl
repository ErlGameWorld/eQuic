-module(eQuic).

-include("eQuic.hrl").

-nifs ([
	paramCfgs/0
	, listenerStart/1			%% 开始监听
	, listenerStart/2			%% 开始监听
	, listenerStart/3       %% 开始监听
	, listenerAccept/2		%% enable接受
	, listenerClose/1       %% 关闭监听

	, cfgOpen/3					%% 注册配置 以提供相同配置策略的模块复用

	, connOpen/3            %% 建立连接
	, connClose/1           %% 关闭连接
	, test/1
]).

-on_load(init/0).

-export([
		paramCfgs/0
		, listenerStart/1			%% 开始监听
		, listenerStart/2			%% 开始监听
		, listenerStart/3       %% 开始监听
		, listenerAccept/2		%% enable接受
		, listenerClose/1       %% 关闭监听

		, cfgOpen/3					%% 注册配置 以提供相同配置策略的模块复用
		, nativeCredentialFile/1

		, connOpen/3            %% 建立连接
		, connClose/1           %% 关闭连接
%%
%% 	, streamOpen/3          %% 打开一个传输流
%% 	, streamEnable/3        %% 开启传输流
%% 	, streamClose/3         %% 关闭一个传输流
%%
%% 	, streamRecv/2          %% 从流主动接受
%% 	, streamSend/3          %% 使用流发送数据
%% 	, datagramSend/3        %% 使用流发送不可靠数据报
%%
%% 	, controlling_process/2 %% 设置流的拥有进程
%%
%% 	, sockname/1
%% 	, getParam/3
%% 	, setParam/4
%%
%% 	, getConnId/1
%% 	, getStreamId/1
		, test/1
]).

nativeCredentialFile(CredentialMap) ->
	CPKCredentialMap = ?IIF(maps:get(eCertificateFile_PrivateKeyFile, CredentialMap, undefined), undefined, CredentialMap, CPKFile, CredentialMap#{eCertificateFile_PrivateKeyFile := filename:nativename(CPKFile)}),
	CCCredentialMap = ?IIF(maps:get(eCertificateFile_CertificateFile, CPKCredentialMap, undefined), undefined, CPKCredentialMap, CCFile, CPKCredentialMap#{eCertificateFile_CertificateFile := filename:nativename(CCFile)}),
	CPPKCredentialMap = ?IIF(maps:get(eCertificateFileProtected_PrivateKeyFile, CCCredentialMap, undefined), undefined, CCCredentialMap, CPPKFile, CCCredentialMap#{eCertificateFileProtected_PrivateKeyFile := filename:nativename(CPPKFile)}),
	?IIF(maps:get(eCertificateFileProtected_CertificateFile, CPPKCredentialMap, undefined), undefined, CPPKCredentialMap, CPCFile, CPPKCredentialMap#{eCertificateFileProtected_CertificateFile := filename:nativename(CPCFile)}).

init() ->
	case code:priv_dir(?MODULE) of
		{error, _} ->
			case code:which(?MODULE) of
				Filename when is_list(Filename) ->
					SoName = filename:join([filename:dirname(Filename), "../priv", atom_to_list(?MODULE)]);
				_ ->
					SoName = filename:join("../priv", atom_to_list(?MODULE))
			end;
		Dir ->
			SoName = filename:join(Dir, atom_to_list(?MODULE))
	end,
	ExecutionProfile = application:get_env(eQuic, eExecutionProfile, ?DefeExecutionProfile),
	?IF(lists:member(ExecutionProfile, ?LimitExecutionProfile) /= true, throw({error, {eExecutionProfile, ExecutionProfile, ?LimitExecutionProfile}})),

	Alpn = application:get_env(eQuic, eAlpn, ?DefAlpn),
	Settings = application:get_env(eQuic, eSettings, ?DefeSettings),
	CredentialSrv = application:get_env(eQuic, eCredentialSrv, ?DefeCredentialSrv),
	CredentialCli = application:get_env(eQuic, eCredentialCli, ?DefeCredentialCli),
	Ret = erlang:load_nif(SoName, {ExecutionProfile, Alpn, Settings, CredentialSrv, CredentialCli}),
	case Ret of
		ok ->
			ParamCfgs = paramCfgs(),
			eqKvsToBeam:load(?paramCfg, ParamCfgs);
		_ ->
			ignore
	end,
	io:format("IMY*****load ret ~p~n", [Ret]),
	Ret.

-define(NOT_LOADED, erlang:nif_error({not_loaded, {module, ?MODULE}, {line, ?LINE}})).

paramCfgs() ->
	?NOT_LOADED.

-spec cfgOpen([string(), ...], map(), map()) -> {ok,reference()} | {error, string()}.
cfgOpen(_Alpns, _SettingsMap, _CredentialMap) ->
	?NOT_LOADED.

-spec listenerStart(listenOpts()) -> {ok, reference()} | {error, term()}.
listenerStart(ListenOpts) ->
	listenerStart("", 0, ListenOpts).

-spec listenerStart(listenPort(), listenOpts()) -> {ok, reference()} | {error, term()}.
listenerStart(ListenPort, ListenOpts) ->
	listenerStart("", ListenPort, ListenOpts).

-spec listenerStart(listenAddr(), listenPort(), listenOpts()) -> {ok, reference()} | {error, term()}.
listenerStart(_ListenAddr, _ListenPort, _ListenOpts) ->
	?NOT_LOADED.

-spec listenerAccept(reference(), pos_integer()) -> ok | {error, term()}.
listenerAccept(_ListenRef, _Id) ->
	?NOT_LOADED.

-spec listenerClose(reference()) -> ok | {error, term()}.
listenerClose(_ListenRef) ->
	?NOT_LOADED.

 -spec connOpen(listenAddr(), listenPort(), connOpts()) -> {ok, reference()} | {error, term()}.
connOpen(_ConnAddr, _ConnPort, _ConnOpts) ->
 	?NOT_LOADED.

-spec connClose(reference()) ->  ok | {error, term()}.
connClose(_ConnRef) ->
	?NOT_LOADED.

%% -spec async_shutdown_connection(connection_handler(), conn_shutdown_flag(), app_errno()) ->
%% 	ok | {error, badarg}.
%% async_shutdown_connection(_Conn, _Flags, _ErrorCode) ->
%% 	?NOT_LOADED.
%%
%% -spec async_accept_stream(connection_handler(), stream_opts()) ->
%% 	{ok, connection_handler()} |
%% 	{error, badarg | internal_error | bad_pid | owner_dead}.
%% async_accept_stream(_Conn, _Opts) ->
%% 	?NOT_LOADED.
%%
%% -spec start_stream(connection_handler(), stream_opts()) ->
%% 	{ok, stream_handler()} |
%% 	{error, badarg | internal_error | bad_pid | owner_dead | not_enough_mem} |
%% 	{error, stream_open_error, atom_reason()} |
%% 	{error, stream_start_error, atom_reason()}.
%% start_stream(_Conn, _Opts) ->
%% 	?NOT_LOADED.
%%
%% -spec send(stream_handler(), iodata(), send_flags()) ->
%% 	{ok, BytesSent :: pos_integer()}          |
%% 	{error, badarg | not_enough_mem | closed} |
%% 	{error, stream_send_error, atom_reason()}.
%% send(_Stream, _Data, _Flags) ->
%% 	?NOT_LOADED.
%%
%% -spec recv(stream_handler(), non_neg_integer()) ->
%% 	{ok, binary()}     |
%% 	{ok, not_ready}     |
%% 	{error, badarg | einval | closed}.
%% recv(_Stream, _Len) ->
%% 	?NOT_LOADED.
%%
%% -spec send_dgram(connection_handler(), iodata(), send_flags()) ->
%% 	{ok, BytesSent :: pos_integer()} |
%% 	{error, badarg | not_enough_memory | closed} |
%% 	{error, dgram_send_error, atom_reason()}.
%% send_dgram(_Conn, _Data, _Flags) ->
%% 	?NOT_LOADED.
%%
%% -spec async_shutdown_stream(stream_handler(), stream_shutdown_flags(), app_errno()) ->
%% 	ok |
%% 	{error, badarg | atom_reason()}.
%% async_shutdown_stream(_Stream, _Flags, _ErrorCode) ->
%% 	?NOT_LOADED.
%%
%% -spec sockname(connection_handler() | stream_handler()) ->
%% 	{ok, {inet:ip_address(), inet:port_number()}} |
%% 	{error, badarg | sockname_error}.
%% sockname(_Conn) ->
%% 	?NOT_LOADED.
%%
%% -spec getopt(handler(), optname(), optlevel()) ->
%% 	not_found | %% `optname' not found, or wrong `optlevel' must be a bug.
%% 	{ok, conn_settings()}   | %% when optname = param_conn_settings
%% 	{error, badarg | param_error | internal_error | not_enough_mem} |
%% 	{error, atom_reason()}.
%%
%% getopt(_Handle, _Optname, _IsRaw) ->
%% 	?NOT_LOADED.
%%
%% -spec setopt(handler(), optname(), any(), optlevel()) ->
%% 	ok |
%% 	{error, badarg | param_error | internal_error | not_enough_mem} |
%% 	{error, atom_reason()}.
%% setopt(_Handle, _Opt, _Value, _Level) ->
%% 	?NOT_LOADED.
%%
%% -spec getConnId(connection_handler()) ->
%% 	{ok, non_neg_integer()} |
%% 	{error, badarg | internal_error}.
%% getConnId(_Handle) ->
%% 	?NOT_LOADED.
%%
%% -spec getStreamId(stream_handler()) ->
%% 	{ok, non_neg_integer()} |
%% 	{error, badarg | internal_error}.
%% getStreamId(_Handle) ->
%% 	?NOT_LOADED.
%%
%% -spec controlling_process(connection_handler() | stream_handler(), pid()) ->
%% 	ok |
%% 	{error, closed | badarg | owner_dead | not_owner}.
%% controlling_process(_H, _P) ->
%% 	erlang:nif_error(nif_library_not_loaded).

test(_Args) ->
	?NOT_LOADED.

