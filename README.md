eQuic
=====

An OTP application

Build
-----
    linux: 第一步 下载最新linux版本的msquic  然后将头文件复制到/usr/include 将 bin下面的文件复制到 /lib64 然后执行 ln -sv libmsquic.so.xxx   libmsquic.so 再执行一下 ldconfig
    windows: 第一步 下载最新windows版本的msquic  然后将头文件复制到INCLUDE_PATH, 将.lib .dll .pdb 文件赋值到你想复制的目录 然后将你的目录添加到PATH环境变量去
    $ rebar3 compile
