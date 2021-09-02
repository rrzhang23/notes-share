## 前言

之前写 C/C++ 程序都是直接新建 cmake 项目，通过 cmake 生成 Makefile，再由 gcc/g++，完成整个 `预处理、编译、链接的过程`，开发者只需要关注 CMakeList.txt 的编写，不用过多关注中间处理的细节。

# 1. C/C++ 编译过程
|步骤|未编译|预处理|编译|汇编|链接|
|--|:--:|:--:|:--:|:--:|:--:|
|文件|fun.h、fun.cpp、test.cpp|fun.i、test.i|fun.s、test.s|fun.o、test.o|something.exe|


```
# 预处理
g++ -E helloworld.cpp -o helloworld.i
# 仅编译，不汇编，不链接
g++ -S helloworld.i -o helloworld.s
# 编译和汇编，不链接
g++ -c helloworld.s -o helloworld.o
# 链接
g++ helloworld.o -o helloworld
```
实际上这四部并不都是由 gcc/g++ 一个程序完成的，具体对应如下：
> 注意 cpp 不是常见的后缀 `.cpp`，而是一个可执行文件 `/usr/bin/cpp`，as 用于从汇编生成目标文件，ld 用于最后的链接
```cpp
cpp    helloworld.cpp -o helloworld.i
g++ -S helloworld.i -o helloworld.s
as     helloworld.s -o helloworld.o
ld     -dynamic-linker /lib/ld-linux.so.2 /usr/lib/crt1.o /usr/lib/crti.o /usr/lib/crtn.o -lc helloworld.o -o helloworld
```
C 语言则是：
~~~cpp
cpp    helloworld.c -o helloworld.i
gcc -S helloworld.i -o helloworld.s
as     helloworld.s -o helloworld.o
ld     -dynamic-linker /lib/ld-linux.so.2 /usr/lib/crt1.o /usr/lib/crti.o /usr/lib/crtn.o -lc helloworld.o -o helloworld
~~~

## 1.1 gcc/g++ 参数
参考：[Makefile 编译与链接选项及CFLAGS与LDFLAGS示例说明](https://blog.csdn.net/zhaoyun_zzz/article/details/82466031)

gcc/g++ 通过不同的参数，可以完成 `cpp、gcc/g++、as、ld` 等一系列动作。
用于预处理、汇编、编译命令如下(可通过 `gcc --help` 查看)：

|选项|说明|
|:--:|:--:|
|-o|生成某个文件|
|-E|生成预处理后的文件 \*.i，\*.i 需要 -o 指定|
|-S|Compile only; do not assemble or link. \*.s，\*.s 文件自动生成|
|-c|Compile and assemble, but do not link. \*.o，\*.o 文件自动生成|
|-I|头文件包含目录 （"i" 的大写），如 "gcc -I/home/zhangsan/leveldb/include -I/home/zhangsan/protobuf/include"|
|-Wall|生成常见的所有告警信息，且停止编译|
|-Werror||
|-w|关闭所有告警信息|
|-O|表示编译优化选项，其后可跟优化等级0\1\2\3，默认是0，不优化，例 "gcc -O3 helloworld.cpp -o helloworld"|
|-fPIC|用于生成位置无关的代码|
|-v|(在标准错误)显示执行编译阶段的命令，同时显示编译器驱动程序,预处理器,编译器的版本号|

用于连接参数：

|选项|说明|
|:--:|:--:|
|-l|库名称，如 "gcc helloworld.cpp -o helloworld -lleveldb -lprotobuf"|
|-L|库目录，如 "gcc -L/home/zhangsan/leveldb/lib -L/home/zhangsan/protobuf/lib"|
|-Wl,option|把选项 option 传递给连接器，如果 option 中含有逗号,就在逗号处分割成多个选项，如 "gcc -Wl,--option1 -Wl,--option2"|
|-static|使用静态库链接生成目标文件，避免使用共享库，生成目标文件会比使用动态链接库大|

## 1.2 头文件、库文件搜索路径及顺序
参考：[gcc中使用 -I 和 -L 引入外部头文件和库文件时路径的搜索次序](https://blog.csdn.net/zklth/article/details/5974371)
《An Introduction to GCC》里描述，默认情况下，gcc 会从 `/usr/local/include、/usr/include`下查找头文件，从 `/usr/local/lib、/usr/lib`下查找库文件。

gcc/g++ 设置参数 `-I, -L -l` 可以优先从设置的参数里查找头文件、库。

另外还可以设置用户环境变量来设置查找位置：
```
# shell 可以通过 export 来设置：
export     C_INCLUDE_PATH=/home/zhangsan/include
export CPLUS_INCLUDE_PATH=/home/zhangsan/include
export    LD_LIBRARY_PATH=/home/zhangsan/lib
export       LIBRARY_PATH=/home/zhangsan/lib
export        LD_RUN_PATH=/home/zhangsan/lib
# 用 unset 来取消上面的 export
unset C_INCLUDE_PATH CPLUS_INCLUDE_PATH LD_LIBRARY_PATH LIBRARY_PATH LD_RUN_PATH
```
或者直接编辑 `~/.bashrc`，加入：
```
export     C_INCLUDE_PATH=/home/zhangsan/include:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=$C_INCLUDE_PATH:$CPLUS_INCLUDE_PATH
export    LD_LIBRARY_PATH=/home/zhangsan/lib:$LD_LIBRARY_PATH
export       LIBRARY_PATH=$LD_LIBRARY_PATH:$LIBRARY_PATH
export        LD_RUN_PATH=$LD_LIBRARY_PATH:$LD_RUN_PATH
```

> 总结。在 gcc/g++ 编译和链接时，查找头文件和库文件的顺序：`-I, -L -l` > `用户环境变量` > `默认目录`

## 1.3 链接路径和运行路径
使用GCC编译动态链接库的项目时，在其他目录下执行很可以出现找不到动态链接库的问题。  
这种情况多发生在动态链接库是自己开发的情况下，原因就是程序运行时找不到去何处加载动态链接库。

可能会说在编译时指定了链接的目录啊！**编译时指定的 -L 的目录，只是在程序链接成可执行文件时使用的。程序执行时动态链接库加载不到动态链接库。**
> 参考：[gcc 运行指定动态库的三种方法](https://blog.csdn.net/qq_38350702/article/details/106128030)

有多中方法指定这些路径：
### 1.3.1 gcc 参数
~~~cpp
// hello.h:
void hello();

// hello.cpp:
#include <iostream>
#include "hello.h"
void hello() {
    std::cout << "hello world!" << std::endl;
}

// main.cpp
#include "hello.h"

int main() {
    hello();
    return 0;
}
~~~

如下是不能成功 run 的：
~~~bash
[]$ unset LD_LIBRARY_PATH LIBRARY_PATH LD_RUN_PATH
[]$ source ~/.bashrc
[]$ g++ hello.cpp -fPIC -shared -o libhello.so
[]$ g++ main.cpp -L. -lhello -o main
[]$ ./main
./main: error while loading shared libraries: libhello.so: cannot open shared object file: No such file or directory
~~~
需要这么指定(相对路径或绝对路径)：
~~~bash
[]$ g++ main.cpp -L. -lhello -Wl,-rpath=./ -o main
~~~

### 1.3.2 ldconfig
将链接库的目录添加到/etc/ld.so.conf文件中或者添加到/etc/ld.so.conf.d/*.conf中，然后使用ldconfig进行更新，进行动态链接库的运行时动态绑定。如：
添加文件/etc/ld.so.conf.d/foo.conf，内容如下
~~~
/somepath/lib
~~~
然后执行如下命令：
~~~bash
[]$ ldconfig
~~~

### 1.3.3 .bashrc
举例, LIBRARY_PATH：
~~~bash
[]$ unset LD_LIBRARY_PATH LIBRARY_PATH LD_RUN_PATH
[]$ source ~/.bashrc
[]$ g++ hello.cpp -fPIC -shared -o libhello.so
[]$ export LIBRARY_PATH=.:$LIBRARY_PATH
# 不指定 -L=. 也可以成功编译：
[]$ g++ main.cpp -lhello -o main
# 但是不能成功运行：
[]$ ./main
./main: error while loading shared libraries: libhello.so: cannot open shared object file: No such file or directory
~~~

此时，继续 export LD_LIBRARY_PATH:
~~~bash
[]$ export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
# 成功
[]$ ./main 
hello world!
~~~

LD_RUN_PATH 比较复杂：
~~~bash
# 1. 先链接，后 LD_RUN_PATH, 再 run。失败
[]$ unset LD_LIBRARY_PATH LIBRARY_PATH LD_RUN_PATH
[]$ source ~/.bashrc
[]$ g++ hello.cpp -fPIC -shared -o libhello.so
[]$ g++ main.cpp -L=. -lhello -o main
[]$ export LD_RUN_PATH=.:$LD_RUN_PATH
[]$ ./main
./main: error while loading shared libraries: libhello.so: cannot open shared object file: No such file or directory


# 2. 先 LD_RUN_PATH，后链接，再 run。成功
[]$ unset LD_LIBRARY_PATH LIBRARY_PATH LD_RUN_PATH
[]$ source ~/.bashrc
[]$ g++ hello.cpp -fPIC -shared -o libhello.so
[]$ export LD_RUN_PATH=.:$LD_RUN_PATH
# 下面语句是失败的，LD_RUN_PATH 对编译时链接无效，只在特定情况下对运行有效
[]$ g++ main.cpp -lhello -o main
/usr/bin/ld: cannot find -lhello
collect2: error: ld returned 1 exit status

[]$ g++ main.cpp -L=. -lhello -o main
[]$ ./main
~~~
### 1.3.4 总结
|选项|说明|
|:--|:--:|
|LIBRARY_PATH| for gcc，仅对`编译时链接`有效|
|LD_LIBRARY_PATH| for run，仅对`运行时链接`有效|
|LD_RUN_PATH| 稍微特殊，类似 -Wl,-rpath 参数，需要在`编译链接`前设置，才能对`运行时链接`有效。对`编译时链接`无效|

> `LD_LIBRARY_PATH LIBRARY_PATH LD_RUN_PATH` 三个参数的含义可以参考：  
[LIBRARY_PATH 和 LD_LIBRARY_PATH 的区别；LD_LIBRARY_PATH and LD_RUN_PATH的区别](https://www.cnblogs.com/xuyaowen/p/linux-so-library-path.html)  


# Makefile

参考：[CFLAGS详解](https://blog.csdn.net/m0_37170593/article/details/78892913)

上述整个 shell 环境编译 C/C++ 的过程可以进一步通过 Makefile 文件 来描述。这样从源文件到可执行文件通过 `make` 命令异步即可生成。

一个例子如下：
```
# Makefile:
CC=gcc
CXX=g++
CFLAGS=-I/home/zhangsan/leveldb/include -I/home/zhangsan/protobuf/include -I. -g -O2 -Wall -Werror -Wno-unused
CXXFLAGS=$(CFLAGS)
LDFLAGS=-L/home/zhangsan/leveldb/lib -L/home/zhangsan/protobuf/lib
LIBS=-lleveldb -lprotobuf

objects=helloworld.o aaa.o bbb.o
project=helloworld

$(project): $(objects)
    $(CXX) -o $(project) $(objects) $(LDFLAGS) $(LIBS)

helloworld.o: helloworld.cpp aaa.h bbb.h
    $(CXX) $(CXXFLAGS) -c -o helloworld.o helloworld.cpp
aaa.o: aaa.cpp aaa.h
    $(CXX) $(CXXFLAGS) -c -o aaa.o aaa.cpp
bbb.o: bbb.cpp bbb.h
    $(CXX) $(CXXFLAGS) -c -o bbb.o bbb.cpp
```

Makefile 文件里，make 只认如下的格式:
```
target:  prerequisites
    command
```
其中，target 是最终生成的目标；prerequisites 是生成该目标所需要的依赖，多个依赖可以用空格隔开；command 是命令，后面可以跟参数。

其余的一切都是依赖或者变量定义，是上述格式所需要的东西。  
像 `CC、CFLAGS、CXXFLAGS、LDFLAGS、LIBS` 都是变量定义，可以改成其他名字，只不过这些名称都是某个规范标准使用的名称，沿用下来了。  
那么上述例子，则最终生成 `helloworld` 的可执行文件。

#### 特殊符号
对于上述的特定格式，有一些特殊符号：`$()`、`$@`、`$^`、`$<`

|符号|含义|
|:--:|:--:|
|$()|表示变量取值，括号中间有字符|
|$@|表示目标，即上面的 target|
|$^|所有的依赖，即 prerequisites|
|$<|第一个依赖，即上面 prerequisites 里第一个依赖|

上面例子可以简写成：
```
# Makefile:
objects=helloworld.o aaa.o bbb.o
project=helloworld

$(project): $(objects)
    $(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

helloworld.o: helloworld.cpp aaa.h bbb.h
    $(CXX) $(CXXFLAGS) -c -o $@ $<
aaa.o: aaa.cpp aaa.h
    $(CXX) $(CXXFLAGS) -c -o $@ $<
bbb.o: bbb.cpp bbb.h
    $(CXX) $(CXXFLAGS) -c -o $@ $<
```


#### 缺省规则 `.c.o`
```
.c.o:
    $(CXX) -c $<
```
表示对于所有的 .o，都有 `.c` 文件与之对应，也可以是 `.cpp`、`.cc`等。
则例子可以进一步简写：
```
# Makefile:
objects=helloworld.o aaa.o bbb.o
project=helloworld

$(project): $(objects)
    $(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
.c.o:
    $(CXX) $(CXXFLAGS) -c -o $@ $<
```


#### all、clean、install等
**all**:  

当一个 Makefile 包含多个目标时，例如：
```
target1:  prerequisites1
    command1
target2:  prerequisites2
    command2
```
默认在生成第一个 target1 后，make 退出，这是候需要加上 all，使得 target1，target2 都会被生成。
```
all: target1 target2
target1:  prerequisites1
    command1
target2:  prerequisites2
    command2
```

**install**:

```
install:
    mv helloworld ~/lib/
```

**clean**:

```
.PHONY: clean
clean:
	-rm  *.o helloworld
```
> clean 总是放在最后


#### 一些简写
```
src=helloworld.cpp aaa.cpp bbb.cpp
objects=$(src:.c=.o)
```
表示从源文件得到 .o 文件列表


最终的 Makefile 文件：
```
# Makefile:
CC=gcc
CXX=g++
CFLAGS=-I/home/zhangsan/leveldb/include -I/home/zhangsan/protobuf/include -I. -g -O2 -Wall -Werror -Wno-unused
CXXFLAGS=$(CFLAGS)
LDFLAGS=-L/home/zhangsan/leveldb/lib -L/home/zhangsan/protobuf/lib
LIBS=-lleveldb -lprotobuf


all: helloworld1 helloworld2

src1=helloworld1.cpp aaa.cpp bbb.cpp
objects1=$(src1:.c=.o)
project1=helloworld1
$(project1): $(objects1)
    $(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
    
src2=helloworld2.cpp aaa.cpp bbb.cpp
objects2=$(src2:.c=.o)
project2=helloworld2
$(project2): $(objects2)
    $(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
    
.c.o:
    $(CXX) $(CXXFLAGS) -c -o $@ $<
    
install:
    mv helloworld ~/lib/

.PHONY: clean
clean:
	-rm  *.o helloworld1 helloworld2
```


# cmake
make 的进一步抽象

变量 CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS

参考[gcc编译器、gdb调试的学习](https://sites.google.com/site/linux31family/home/home-1/--8/gccgdb?tmpl=%2Fsystem%2Fapp%2Ftemplates%2Fprint%2F&showPrintDialog=1)