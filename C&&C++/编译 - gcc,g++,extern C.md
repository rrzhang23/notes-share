## gcc 和 g++
两个不同的编译器，一个用于构建 C，另一个构建 C++ 程序。  
路子都是 预处理 --> 编译 --> 汇编 --> 链接。  
对于 C++ 项目，`g++ -E` --> `g++ -S` -- > `g++ -c` --> `g++`, 或 `cpp` --> `g++ -S` --> `as` --> `ld`  
对于 C 项目，`gcc -E` --> `gc -S` -- > `gcc -c` --> `gcc`, 或 `cpp` --> `gcc -S` --> `as` --> `ld`  

> cc、gcc 是一样，c++/g++也是同一个工具。  
> `.c` 会被识别为 C 语言文件，而 `.cc`、`.cxx`、`.cpp`、`.C` 会被识别为 C++ 文件。

g++ 一般会兼容 C 项目，而 gcc 则不能兼容 C++。  
比如文件 `hello.c`：
~~~cpp
#include <iostream>
int main() { std::cout << "hello" << std::endl; return 0; }
~~~
使用 C 工具链编译时，会产生错误：

1. 在预处理阶段发生错误
~~~bash
[]$ gcc hello.c -o hello
[]$ gcc cout.c -E -o cout.i
[]$ cpp hello.c -o hello.i
In file included from hello.c:2:0:
hello.h:3:8: error: expected identifier or ‘(’ before string constant
 extern "C"

~~~

2. 把 `hello.c` 改为 `hello.cpp` 后，在编译阶段错误：
~~~bash
[]$ cpp cout.cpp -o cout.i
# 上一步能成功执行，因为 cpp 预处理把它当做 C++ 文件处理了。下面会在编译阶段出问题：
[]$ gcc cout.c -E -o cout.i
[]$ gcc cout.i -o cout
~~~

3. 把 `hello.c` 改为 `hello.cpp` 后，在链接阶段出问题：
~~~bash
[]$ gcc cout.cpp -o cout
/tmp/cc9pJzgI.o: In function `main':
cout.cpp:(.text+0xa): undefined reference to `std::cout'
cout.cpp:(.text+0xf): undefined reference to `std::basic_ostream<char, std::char_traits<char> >& std::operator<< <std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*)'
cout.cpp:(.text+0x14): undefined reference to `std::basic_ostream<char, std::char_traits<char> >& std::endl<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&)'
cout.cpp:(.text+0x1c): undefined reference to `std::ostream::operator<<(std::ostream& (*)(std::ostream&))'
/tmp/cc9pJzgI.o: In function `__static_initialization_and_destruction_0(int, int)':
cout.cpp:(.text+0x4a): undefined reference to `std::ios_base::Init::Init()'
cout.cpp:(.text+0x59): undefined reference to `std::ios_base::Init::~Init()'
collect2: error: ld returned 1 exit status
~~~


## extern "C"
C++ 程序为了兼容 C 引入的，通常搭配使用的还有一个宏 `__cplusplus`。

先上代码：
~~~cpp
// hello.h:
void hello();

// hello.c
#include <stdio.h>
#include "hello.h"
void hello() { printf("hello world!\n"); }

// hello.cpp
#include <iostream>
#include "hello.h"
void hello() { std::cout << "hello world!" << std::endl; }


// main.c && main.cpp
#include "hello.h"
int main() { hello(); return 0; }
~~~

分别使用 C 编译出动态库给 C++ 项目使用，和 C++ 编译出动态库给 C 项目使用：
1. 
~~~bash
[]$ gcc hello.c   -fPIC -shared -o libhello.so
[]$ gcc main.c    -L. -lhello   -o main         # 成功 run
[]$ g++ main.cpp  -L. -lhello   -o main		    # fail
/tmp/ccNCeKBo.o: In function `main':
main.cpp:(.text+0x5): undefined reference to `hello()'
collect2: error: ld returned 1 exit status
~~~
2.
~~~bash
g++ hello.cpp -fPIC -shared -o libhello.so
g++ main.cpp  -L. -lhello   -o main         # 成功 run
gcc main.c    -L. -lhello   -o main         # fail
/tmp/ccMLl1Yh.o: In function `main':
main.c:(.text+0xa): undefined reference to `hello'
collect2: error: ld returned 1 exit status
~~~

原因是，为了函数重载等，C++ 通常改写编译后的函数名(即目标文件 .o 中的符号)。在例 1 中，libhello.so 中的函数 `hello` 对应符号为 `_hello`，而构建 C++ 程序 main 时，g++ 尝试寻找 `__Z12hello` 之类的符号，所以就报未定义错误；同样地，g++ 编译出的 libhello.so 中 `hello` 对应符号为 `__Z12hello`，而 gcc 尝试寻找 `__hello` 之类的符号，就会报未定义错误。

正确的做法，修改 `hello.h`：
~~~cpp
#ifdef __cplusplus
extern "C"
{
#endif

void hello();

#ifdef __cplusplus
}
#endif
~~~

> 一个有意思的问题 `#include` 应该放在 extern "C" 内还是外？建议放在外面，放在里面会产生嵌套，有些编译器不支持。

> 参考[关于extern "C"（详细剖析）](https://blog.csdn.net/u010639500/article/details/87885421)
