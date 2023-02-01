QUIC_PARAM_CONN_DISABLE_1RTT_ENCRYPTION   应用程序必须#define QUIC_API_ENABLE_INSECURE_FEATURES在包含 msquic.h 之前。
QUIC_PARAM_CONN_DATAGRAM_RECEIVE_ENABLED  指示/查询对 QUIC 数据报扩展的支持。必须在start前设置。

QUIC_PARAM_CONN_SHARE_UDP_BINDING         仅在客户端上设置。必须在start前调用。
QUIC_PARAM_CONN_REMOTE_ADDRESS            仅在客户端上设置。必须在start前设置。  
QUIC_PARAM_CONN_LOCAL_ADDRESS             仅在客户端上设置。必须在开始前或握手确认后设置。 

QUIC_PARAM_CONN_RESUMPTION_TICKET         必须在start之前在客户端上设置。

关闭时close

回调执行的顺序：对于给定的连接，事件回调将在同一个线程上按顺序发生。流事件也在连接的工作线程上处理，所以它们也是按顺序到达的

Linux(centos7)下安装OpenSSL 安装详解

一、查看主机openssl版本信息
1、查看路径
which openssl
2、查看版本
openssl version
3、查看CentOS版本
cat /etc/redhat-release
P.S. CentOS 7.6 默认版本：openssl-1.0.2k



二、安装Openssl
方法一、直接安装
yum install openssl openssl-devel
方法二、下载源码编译安装
访问OpenSSL官网资源，查看是否有最新的版本发布。

1、下载
quic 分支对应的openssl:  https://github.com/quictls/openssl/
2、解压并切换目录
tar -zxvf openssl-OpenSSL_1_1_1q-quic1.tar.gz

3、设定Openssl 安装，( --prefix )参数为欲安装之目录，也就是安装后的档案会出现在该目录下
./config --prefix=/usr/local/openssl
4、执行命令
./config -t
5、执行make、make install，编译Openssl
make & make install
P.S. 若CentOS7中没有GCC编译器，执行命令 yum -y install gcc 安装GCC。


6、切换openssl版本
mv /usr/bin/openssl /usr/bin/openssl.bak
mv /usr/include/openssl /usr/include/openssl.bak


ln -s /usr/local/openssl/bin/openssl /usr/bin/openssl
ln -s /usr/local/openssl/include/openssl /usr/include/openssl

echo "/usr/local/openssl/lib" >> /etc/ld.so.conf

ldconfig -v

ln -sf /usr/local/openssl/lib/libssl.so.81.1.1 /lib64/libssl.so.1.1
ln -sf /usr/local/openssl/lib/libcrypto.so.81.1.1 /lib64/libcrypto.so.1.1
注意：不能直接删除软链接

如需使用新版本开发，则需替换原来的软链接指向，即替换原动态库，进行版本升级。

替换/lib(lib64)和/usr/lib(lib64)和/usr/local/lib(lib64)存在的相应动态库：

ln -sf /usr/local/openssl/lib/libssl.so.81.1.1 /lib64/libssl.so
ln -sf /usr/local/openssl/lib/libcrypto.so.81.1.1 /lib64/libcrypto.so



