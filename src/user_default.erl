-module(user_default).

-compile([export_all, nowarn_export_all]).

cfg() ->
   eQuic:cfgOpen(["h3"], #{}, #{eType => 4, eFlags => 4 , eCertificateFile_PrivateKeyFile => "./priv/key.pem", eCertificateFile_CertificateFile => "./priv/cert.pem"}).

ll() ->
   eQuic:listenerStart(8888, #{eAlpn => ["h3"], eAcceptorCnt => 10}).

ll1(P) ->
   eQuic:listenerStart(P, #{eAlpn => ["h3"], eAcceptorCnt => 10}).

ll(Ref) ->
   eQuic:listenerStart(8888, #{eAlpn => ["h3"], eCfgRef => Ref, eAcceptorCnt => 10}).

ac(Ref, Id) ->
   eQuic:listenerAccept(Ref, Id).