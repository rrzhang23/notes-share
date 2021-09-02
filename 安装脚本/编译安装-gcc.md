# 编译安装 gcc
## 准备

gcc 依赖以下几个包：gmp、mpfr、mpc、isl，至少 gcc-7.4.0 版本依赖这几个包，其他的版本或许依赖的更多/更少。

| 包名 | 版本 |
| --------- | --------- |
| gcc | 7.4.0 |
| gmp | 6.1.0 |
| mpfr | 3.1.4 |
| mpc | 1.0.3 |
| isl | 0.16.1 |

```
# 下载 gcc-7.4.0.tar.xz
if [ ! -f "gcc-7.4.0.tar.xz" ];then
    wget https://mirrors.huaweicloud.com/gnu/gcc/gcc-7.4.0/gcc-7.4.0.tar.xz
fi

# 下载依赖
if [ ! -f "gmp-6.1.0.tar.bz2" ];then
    wget https://mirrors.huaweicloud.com/gnu/gmp/gmp-6.1.0.tar.bz2
fi
if [ ! -f "mpfr-3.1.4.tar.bz2" ];then
    wget https://mirrors.huaweicloud.com/gnu/mpfr/mpfr-3.1.4.tar.bz2
fi
if [ ! -f "mpc-1.0.3.tar.gz" ];then
    wget https://mirrors.huaweicloud.com/gnu/mpc/mpc-1.0.3.tar.gz
fi
if [ ! -f "isl-0.16.1.tar.bz2" ];then
    wget http://mirror.linux-ia64.org/gnu/gcc/infrastructure/isl-0.16.1.tar.bz2
fi
```

## 注意

在安装时可能遇到 `g++: braced spec '%:sanitize(address):%{!shared:libasan_preinit%O%s} ` 这样的问题，建议在安装前清楚几个变量：
```
unset CPLUS_INCLUDE_PATH C_INCLUDE_PATH LIBRARY_PATH
unset CPLUS_INCLUDE_PATH C_INCLUDE_PATH LIBRARY_PATH PKG_CONFIG_PATH INCLUDE CPATH
```

## 安装

官方建议不要在源码目录直接编译，所以我们的目录结构如下：  
```
- gcc-7.4.0-src                          // 源码目录
--- gmp-6.1.0.tar.bz2
--- mpfr-3.1.4.tar.bz2
--- mpc-1.0.3.tar.gz
--- isl-0.16.1.tar.bz2
--- contrib/download_prerequisites   // 可执行文件，检查依赖的几个包，要在编译之前先运行
- gcc-7.4.0-build                    // 编译目录
```

要先进入编译目录，再运行 configure，需要指定 `prefix`、`mandir`、`infodir` 等几个目录，`--enable-languages` 可以指定更多语言支持，但是编译时间会加长。
```
//
$ cd gcc-7.4.0-src
$ ./contrib/download_prerequisites

// 进入编译目录
$ cd gcc-7.4.0-build

$ ../gcc-7.4.0-src/configure \
--prefix=/home/zhangrongrong/.local/gcc-7.4.0 \
--mandir=/home/zhangrongrong/.local/gcc-7.4.0/share/man \
--infodir=/home/zhangrongrong/.local/gcc-7.4.0/share/info \
--enable-languages=c,c++,objc,obj-c++ \
--enable-bootstrap --enable-shared \
--enable-threads=posix \
--enable-checking=release \
--with-system-zlib --disable-multilib \
--enable-__cxa_atexit \
--disable-libunwind-exceptions \
--enable-gnu-unique-object \
--enable-linker-build-id \
--with-linker-hash-style=gnu \
--enable-plugin --enable-initfini-array \
--disable-libgcj \
--enable-gnu-indirect-function \
--with-tune=generic \
--with-arch_32=x86-64 \
--build=x86_64-pc-linux-gnu \

$ nohup time make -j32 &
$ time make install

$ cd ..
$ rm -rf gcc-7.4.0-build
```

安装完成后需要配置环境目录等，实例如下：
```
export MY_GCC_VERSION=7.4.0
export GCC_ROOT=/home/zhangrongrong/.local/gcc-$MY_GCC_VERSION
export               PATH=$GCC_ROOT/bin:$PATH
export    LD_LIBRARY_PATH=$GCC_ROOT/libexec/gcc/x86_64-pc-linux-gnu/$MY_GCC_VERSION:$GCC_ROOT/lib:$GCC_ROOT/lib64:$GCC_ROOT/libexec:$LD_LIBRARY_PATH
export       LIBRARY_PATH=$LD_LIBRARY_PATH:$LIBRARY_PATH
export        LD_RUN_PATH=$LD_LIBRARY_PATH:$LD_RUN_PATH
export                 CC=$GCC_ROOT/bin/gcc
export                CXX=$GCC_ROOT/bin/g++
export     C_INCLUDE_PATH=$GCC_ROOT/include:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=$C_INCLUDE_PATH:$CPLUS_INCLUDE_PATH

export LD_LIBRARY_PATH=$(echo $LD_LIBRARY_PATH | sed -E -e 's/^:*//' -e 's/:*$//' -e 's/:+/:/g')
export    LIBRARY_PATH=$(echo $LIBRARY_PATH    | sed -E -e 's/^:*//' -e 's/:*$//' -e 's/:+/:/g')
export     LD_RUN_PATH=$(echo $LD_RUN_PATH     | sed -E -e 's/^:*//' -e 's/:*$//' -e 's/:+/:/g')
最后这几个是防止在目录拼接后，路径中间出现 "::" 双冒号的问题，这在编译 PostgreSQL 等其他包时，会产生严重的 error。
```

> make 命令会自动补全一些参数，通过 ps -ef | grep make 可以看到，可以帮助排除一些编译错误的问题（主要是环境路径）。  
> make install 会打印一些信息，可以参照 make install 打印的信息，配置 .bashrc 环境。

#### ps -ef | grep make 打印出的参数信息：
```
 make DESTDIR= 
 RPATH_ENVVAR=LD_LIBRARY_PATH TARGET_SUBDIR=x86_64-pc-linux-gnu bindir=/home/zhangrongrong/.local/gcc-7.4.0/bin 
 datadir=/home/zhangrongrong/.local/gcc-7.4.0/share 
 exec_prefix=/home/zhangrongrong/.local/gcc-7.4.0 
 includedir=/home/zhangrongrong/.local/gcc-7.4.0/include 
 datarootdir=/home/zhangrongrong/.local/gcc-7.4.0/share 
 docdir=/home/zhangrongrong/.local/gcc-7.4.0/share/doc/ 
 infodir=/home/zhangrongrong/.local/gcc-7.4.0/share/info 
 pdfdir=/home/zhangrongrong/.local/gcc-7.4.0/share/doc/ 
 htmldir=/home/zhangrongrong/.local/gcc-7.4.0/share/doc/ 
 libdir=/home/zhangrongrong/.local/gcc-7.4.0/lib 
 libexecdir=/home/zhangrongrong/.local/gcc-7.4.0/libexec 
 lispdir= 
 localstatedir=/home/zhangrongrong/.local/gcc-7.4.0/var 
 mandir=/home/zhangrongrong/.local/gcc-7.4.0/share/man 
 oldincludedir=/usr/include 
 prefix=/home/zhangrongrong/.local/gcc-7.4.0 
 sbindir=/home/zhangrongrong/.local/gcc-7.4.0/sbin 
 sharedstatedir=/home/zhangrongrong/.local/gcc-7.4.0/com 
 sysconfdir=/home/zhangrongrong/.local/gcc-7.4.0/etc 
 tooldir=/home/zhangrongrong/.local/gcc-7.4.0/x86_64-pc-linux-gnu 
 build_tooldir=/home/zhangrongrong/.local/gcc-7.4.0/x86_64-pc-linux-gnu 
 target_alias=x86_64-pc-linux-gnu AWK=gawk BISON=bison 
 CC_FOR_BUILD=/home/zhangrongrong/.local/gcc-4.8.5/bin/gcc 
 CFLAGS_FOR_BUILD=-g -O2 
 CXX_FOR_BUILD=/home/zhangrongrong/.local/gcc-4.8.5/bin/
 g++ -std=gnu++98 EXPECT=expect FLEX=flex 
 INSTALL=/usr/bin/install -c 
 INSTALL_DATA=/usr/bin/install -c -m 644 
 INSTALL_PROGRAM=/usr/bin/install -c 
 INSTALL_SCRIPT=/usr/bin/install -c LDFLAGS_FOR_BUILD= 
 LEX=flex M4=m4 MAKE=make RUNTEST=runtest RUNTESTFLAGS= 
 SED=/bin/sed 
 SHELL=/bin/sh 
 YACC=bison -y XFOO= 
 ADA_CFLAGS= AR_FLAGS=rc BOOT_ADAFLAGS=-gnatpg 
 BOOT_CFLAGS=-g -O2 BOOT_LDFLAGS= CFLAGS=-g -O2 
 CXXFLAGS=-g -O2 LDFLAGS= LIBCFLAGS=-g -O2 LIBCXXFLAGS=-g -O2 -fno-implicit-templates STAGE1_CHECKING=--enable-checking=yes,types STAGE1_LANGUAGES=c,c++,lto GNATBIND=no GNATMAKE=no AR_FOR_TARGET=ar AS_FOR_TARGET=as 
 CC_FOR_TARGET=/home/zhangrongrong/download/gcc-7.4.0-build/./gcc/xgcc 
 -B/home/zhangrongrong/download/gcc-7.4.0-build/./gcc/ CFLAGS_FOR_TARGET=-g -O2 CPPFLAGS_FOR_TARGET= CXXFLAGS_FOR_TARGET=-g -O2 -D_GNU_SOURCE DLLTOOL_FOR_TARGET=dlltool 
 FLAGS_FOR_TARGET=-B/home/zhangrongrong/.local/gcc-7.4.0/x86_64-pc-linux-gnu/bin/ 
 -B/home/zhangrongrong/.local/gcc-7.4.0/x86_64-pc-linux-gnu/lib/ -isystem 
 /home/zhangrongrong/.local/gcc-7.4.0/x86_64-pc-linux-gnu/include -isystem 
 /home/zhangrongrong/.local/gcc-7.4.0/x86_64-pc-linux-gnu/sys-include GFORTRAN_FOR_TARGET= GOC_FOR_TARGET= GOCFLAGS_FOR_TARGET=-O2 -g LD_FOR_TARGET=ld LIPO_FOR_TARGET=lipo LDFLAGS_FOR_TARGET= LIBCFLAGS_FOR_TARGET=-g -O2 LIBCXXFLAGS_FOR_TARGET=-g -O2 -D_GNU_SOURCE -fno-implicit-templates NM_FOR_TARGET=nm OBJDUMP_FOR_TARGET=objdump OBJCOPY_FOR_TARGET= RANLIB_FOR_TARGET=ranlib READELF_FOR_TARGET=readelf STRIP_FOR_TARGET=strip WINDRES_FOR_TARGET=windres WINDMC_FOR_TARGET=windmc BUILD_CONFIG=bootstrap-debug XFOO= LEAN=false STAGE1_CFLAGS=-g STAGE1_CXXFLAGS=-g STAGE1_TFLAGS= STAGE2_CFLAGS=-g -O2 -gtoggle STAGE2_CXXFLAGS=-g -O2 -gtoggle STAGE2_TFLAGS= STAGE3_CFLAGS=-g -O2 STAGE3_CXXFLAGS=-g -O2 STAGE3_TFLAGS= STAGE4_CFLAGS=-g -O2 STAGE4_CXXFLAGS=-g -O2 STAGE4_TFLAGS= STAGEprofile_CFLAGS=-g -O2 -gtoggle -fprofile-generate STAGEprofile_CXXFLAGS=-g -O2 -gtoggle -fprofile-generate STAGEprofile_TFLAGS= STAGEfeedback_CFLAGS=-g -O2 -fprofile-use STAGEfeedback_CXXFLAGS=-g -O2 -fprofile-use STAGEfeedback_TFLAGS= STAGEautoprofile_CFLAGS=-g -O2 -gtoggle -g STAGEautoprofile_CXXFLAGS=-g -O2 -gtoggle -g STAGEautoprofile_TFLAGS= STAGEautofeedback_CFLAGS=-g -O2 STAGEautofeedback_CXXFLAGS=-g -O2 STAGEautofeedback_TFLAGS= TFLAGS= 
 CONFIG_SHELL=/bin/sh 
 MAKEINFO=makeinfo --split-size=5000000 --split-size=5000000 --split-size=5000000 CFLAGS=-g -O2 -gtoggle CXXFLAGS=-g -O2 -gtoggle LIBCFLAGS=-g -O2 -gtoggle CFLAGS_FOR_TARGET=-g -O2 CXXFLAGS_FOR_TARGET=-g -O2 -D_GNU_SOURCE LIBCFLAGS_FOR_TARGET=-g -O2 AR=ar AS=as 
 CC=/home/zhangrongrong/.local/gcc-4.8.5/bin/gcc 
 CXX=/home/zhangrongrong/.local/gcc-4.8.5/bin/g++ 
 -std=gnu++98 DLLTOOL=dlltool 
 GFORTRAN= GOC= LD=ld LIPO=lipo NM=nm OBJDUMP=objdump R
 ```
 
## 遇到的问题

### 1. 在 CentOS 6 上用系统的 gcc-4.4 编译 gcc-4.8.5 没有问题，但是在编译 gcc-7.4.0 时发生了如下的问题，是  `/usr/include/scsi/scsi.h` 缺少了。  
 
 复制 CentOS 7 系统下 `/usr/include/scsi/scsi.h` 里面的所有代码，替换`gcc-7.4.0/libsanitizer/sanitizer_common/sanitizer_platform_limits_posix.cc` 里 `#include <scsi/scsi.h>` 这一行。
 
 ```
Making install in sanitizer_common
make[3]: Entering directory '/home/zhangrongrong/download/gcc-7.4.0-build/x86_64-pc-linux-gnu/libsanitizer/sanitizer_common'
/bin/sh ../libtool --tag=CXX   --mode=compile /home/zhangrongrong/download/gcc-7.4.0-build/./gcc/xgcc -shared-libgcc -B/home/zhangrongrong/download/gcc-7.4.0-build/./gcc -nostdinc++ -L/home/zhangrongrong/download/gcc-7.4.0-build/x86_64-pc-linux-gnu/libstdc++-v3/src -L/home/zhangrongrong/download/gcc-7.4.0-build/x86_64-pc-linux-gnu/libstdc++-v3/src/.libs -L/home/zhangrongrong/download/gcc-7.4.0-build/x86_64-pc-linux-gnu/libstdc++-v3/libsupc++/.libs -B/home/zhangrongrong/.local/gcc-7.4.0/x86_64-pc-linux-gnu/bin/ -B/home/zhangrongrong/.local/gcc-7.4.0/x86_64-pc-linux-gnu/lib/ -isystem /home/zhangrongrong/.local/gcc-7.4.0/x86_64-pc-linux-gnu/include -isystem /home/zhangrongrong/.local/gcc-7.4.0/x86_64-pc-linux-gnu/sys-include    -D_GNU_SOURCE -D_DEBUG -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS  -DHAVE_RPC_XDR_H=1 -DHAVE_TIRPC_RPC_XDR_H=0 -I. -I../../../../gcc-7.4.0/libsanitizer/sanitizer_common -I..  -I ../../../../gcc-7.4.0/libsanitizer/include -isystem ../../../../gcc-7.4.0/libsanitizer/include/system  -Wall -W -Wno-unused-parameter -Wwrite-strings -pedantic -Wno-long-long -fPIC -fno-builtin -fno-exceptions -fno-rtti -fomit-frame-pointer -funwind-tables -fvisibility=hidden -Wno-variadic-macros -I../../libstdc++-v3/include     -I../../libstdc++-v3/include/x86_64-pc-linux-gnu     -I../../../../gcc-7.4.0/libsanitizer/../libstdc++-v3/libsupc++ -std=gnu++11 -DSANITIZER_LIBBACKTRACE -DSANITIZER_CP_DEMANGLE -I ../../../../gcc-7.4.0/libsanitizer/../libbacktrace -I ../libbacktrace -I ../../../../gcc-7.4.0/libsanitizer/../include -include ../../../../gcc-7.4.0/libsanitizer/libbacktrace/backtrace-rename.h -g -O2 -D_GNU_SOURCE -MT sanitizer_platform_limits_posix.lo -MD -MP -MF .deps/sanitizer_platform_limits_posix.Tpo -c -o sanitizer_platform_limits_posix.lo ../../../../gcc-7.4.0/libsanitizer/sanitizer_common/sanitizer_platform_limits_posix.cc
libtool: compile:  /home/zhangrongrong/download/gcc-7.4.0-build/./gcc/xgcc -shared-libgcc -B/home/zhangrongrong/download/gcc-7.4.0-build/./gcc -nostdinc++ -L/home/zhangrongrong/download/gcc-7.4.0-build/x86_64-pc-linux-gnu/libstdc++-v3/src -L/home/zhangrongrong/download/gcc-7.4.0-build/x86_64-pc-linux-gnu/libstdc++-v3/src/.libs -L/home/zhangrongrong/download/gcc-7.4.0-build/x86_64-pc-linux-gnu/libstdc++-v3/libsupc++/.libs -B/home/zhangrongrong/.local/gcc-7.4.0/x86_64-pc-linux-gnu/bin/ -B/home/zhangrongrong/.local/gcc-7.4.0/x86_64-pc-linux-gnu/lib/ -isystem /home/zhangrongrong/.local/gcc-7.4.0/x86_64-pc-linux-gnu/include -isystem /home/zhangrongrong/.local/gcc-7.4.0/x86_64-pc-linux-gnu/sys-include -D_GNU_SOURCE -D_DEBUG -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -DHAVE_RPC_XDR_H=1 -DHAVE_TIRPC_RPC_XDR_H=0 -I. -I../../../../gcc-7.4.0/libsanitizer/sanitizer_common -I.. -I ../../../../gcc-7.4.0/libsanitizer/include -isystem ../../../../gcc-7.4.0/libsanitizer/include/system -Wall -W -Wno-unused-parameter -Wwrite-strings -pedantic -Wno-long-long -fPIC -fno-builtin -fno-exceptions -fno-rtti -fomit-frame-pointer -funwind-tables -fvisibility=hidden -Wno-variadic-macros -I../../libstdc++-v3/include -I../../libstdc++-v3/include/x86_64-pc-linux-gnu -I../../../../gcc-7.4.0/libsanitizer/../libstdc++-v3/libsupc++ -std=gnu++11 -DSANITIZER_LIBBACKTRACE -DSANITIZER_CP_DEMANGLE -I ../../../../gcc-7.4.0/libsanitizer/../libbacktrace -I ../libbacktrace -I ../../../../gcc-7.4.0/libsanitizer/../include -include ../../../../gcc-7.4.0/libsanitizer/libbacktrace/backtrace-rename.h -g -O2 -D_GNU_SOURCE -MT sanitizer_platform_limits_posix.lo -MD -MP -MF .deps/sanitizer_platform_limits_posix.Tpo -c ../../../../gcc-7.4.0/libsanitizer/sanitizer_common/sanitizer_platform_limits_posix.cc  -fPIC -DPIC -o .libs/sanitizer_platform_limits_posix.o
../../../../gcc-7.4.0/libsanitizer/sanitizer_common/sanitizer_platform_limits_posix.cc:147:10: fatal error: scsi/scsi.h: No such file or directory
 #include <scsi/scsi.h>
          ^~~~~~~~~~~~~
compilation terminated.
 ```
### 2. g++: braced spec '%:sanitize(address):%{!shared:libasan_preinit%O%s}
```
g++: braced spec '%:sanitize(address):%{!shared:libasan_preinit%O%s} %{static-libasan:%{!shared:-Bstatic --whole-archive -lasan --no-whole-archive -Bdynamic}}%{!static-libasan:-lasan}}     %{%:sanitize(thread):%{static-libtsan:%{!shared:-Bstatic --whole-archive -ltsan --no-whole-archive -Bdynamic}}%{!static-libtsan:-ltsan}}     %{%:sanitize(leak):%{static-liblsan:%{!shared:-Bstatic --whole-archive -llsan --no-whole-archive -Bdynamic}}%{!static-liblsan:-llsan}}' is invalid at '%'
```
解决：编译之前：
```
unset CPLUS_INCLUDE_PATH C_INCLUDE_PATH LIBRARY_PATH
```
 
