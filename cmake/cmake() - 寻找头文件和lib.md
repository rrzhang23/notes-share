以 protobuf 举例


### 1. 直接 set
```
set(Protobuf_INCLUDE_DIRS /home/zhangrongrong/.local/protobuf/include)
set(Protobuf_LIBRARIES /home/zhangrongrong/.local/protobuf/lib64/libprotobuf.a)
include_directories(Protobuf_INCLUDE_DIRS)
target_link_libraries(main Protobuf_LIBRARIES)
```

### 2. find_file、find_path、find_library、find_program
```
find_path(   Protobuf_INCLUDE_DIRS      NAMES google/protobuf/service.h PATHS /home/zhangrongrong/.local/protobuf/include NO_DEFAULT_PATH)
find_library(Protobuf_LIBRARIES         NAMES protobuf                  PATHS /home/zhangrongrong/.local/protobuf/lib64 NO_DEFAULT_PATH)
find_program(Protobuf_PROTOC_EXECUTABLE NAMES protoc                    PATHS /home/zhangrongrong/.local/protobuf/bin NO_DEFAULT_PATH)
message(STATUS "Protobuf_INCLUDE_DIRS: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf_LIBRARIES: ${Protobuf_LIBRARIES}")
message(STATUS "Protobuf_PROTOC_EXECUTABLE: ${Protobuf_PROTOC_EXECUTABLE}")

include_directories(Protobuf_INCLUDE_DIRS)
target_link_libraries(main Protobuf_LIBRARIES)
```
参考[find_path](https://cmake.org/cmake/help/v3.4/command/find_path.html)  
参考[find_library](https://cmake.org/cmake/help/v3.4/command/find_library.html)  
参考[find_program](https://cmake.org/cmake/help/v3.4/command/find_program.html)  
find_file 类似

其中 NAMES、PATHS 后可以跟多个目标，find_library 默认查找动态库，例如 `libprotobuf.so` 可直接简写成 `protobuf` 跟在 NAMES 后。


### 3. include(FindProtobuf)
```
include(FindProtobuf)
message(STATUS "Protobuf_INCLUDE_DIRS: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf_LIBRARIES: ${Protobuf_LIBRARIES}")
message(STATUS "Protobuf_PROTOC_EXECUTABLE: ${Protobuf_PROTOC_EXECUTABLE}")
```
一般 `FindProtobuf` 指代 `FindProtobuf.cmake`，这个文件一般在 `cmake_install_path/share/cmake-3.14/Modules` 下面。  
FindProtobuf.cmake 会从 `/usr/bin /usr/lib /usr/include` 下面去找我们的目标，然后设置一下几个值：  
`Protobuf_FOUND`  
`Protobuf_INCLUDE_DIRS or Protobuf_INCLUDES`  
`Protobuf_LIBRARIES or Protobuf_LIBRARIES or Protobuf_LIBS`  

想要指定自定义 Xxx.cmake，需要 set CMAKE_MODULE_PATH


### 4. find_package
Xxx.cmake 高一级的抽象，这个命令去寻找 `FindXxx.cmake` 或 `ProtobufConfig.cmake` 或 `protobuf-config.cmake`。

find_package 分为两种模式：  
- module  
  默认模式，会查找 `${CMAKE_MODULE_PATH}` 以及 `cmake_install_path/share/cmake-3.14/Modules` 下的 `FindProtobuf.cmake`。
- config
  比 module 高级用法，寻找的目标是 `ProtobufConfig.cmake` 或 `protobuf-config.cmake`。  
  会从三两个个地方找:  
  1. `~/.cmake/packages/sometxt`，`sometxt` 记录了软件安装库时，同时生成的 `*.cmake` 的位置  
  2. /usr/local/share/
```
find_package(Protobuf REQUIRED)
message(STATUS "Protobuf_INCLUDE_DIRS: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf_LIBRARIES: ${Protobuf_LIBRARIES}")
message(STATUS "Protobuf_PROTOC_EXECUTABLE: ${Protobuf_PROTOC_EXECUTABLE}")
```

对于自定义安装的库，可以在find_package 之前指定 `Protobuf_ROOT`：
```
set(Protobuf_ROOT "/home/zhangrongrong/.local/protobuf")
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Protobuf_INCLUDE_DIRS: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf_LIBRARIES: ${Protobuf_LIBRARIES}")
message(STATUS "Protobuf_PROTOC_EXECUTABLE: ${Protobuf_PROTOC_EXECUTABLE}")

or 

set(Protobuf_ROOT "/home/zhangrongrong/.local/protobuf")
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Protobuf_INCLUDE_DIRS: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf_LIBRARIES: ${Protobuf_LIBRARIES}")
message(STATUS "Protobuf_PROTOC_EXECUTABLE: ${Protobuf_PROTOC_EXECUTABLE}")

or

set(protobuf_MODULE_COMPATIBLE TRUE)
include(/home/zhangrongrong/.local/protobuf/lib/cmake/protobuf/protobuf-config.cmake)
message(STATUS "Protobuf_INCLUDE_DIRS: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf_LIBRARIES: ${Protobuf_LIBRARIES}")
message(STATUS "Protobuf_PROTOC_EXECUTABLE: ${Protobuf_PROTOC_EXECUTABLE}")
```

参考 [深入理解CMake(3):find_package()的使用](https://www.jianshu.com/p/39fc5e548310)

### 几个变量
无论是 `include(FindProtobuf)`，还是 `find_package`，都是通过 `find_path、find_library` 等去找头文件目录和库的。

`find_package` 首先找 `*.cmake`，`*.cmake` 再通过 `find_path、find_library` 找目标。

以下变量有的影响 `find_package` 查找 `*.cmake`，有的则是直接影响查找  `find_path、find_librar`。

- 1. `CMAKE_MODULE_PATH` 供 include() 或 find_package()使用，初始化为空。for `*.cmake`
- 2. `CMAKE_PREFIX_PATH` 供五种 find_* 使用。`for find_*`
- 3. `Xxx_ROOT`，为 find_package() 指定查找 .cmake 的目录，可以遍历子目录。for `*.cmake`
- 4. `Xxx_DIR` ，为 find_package() 指定查找 .cmake 的目录，不能遍历子目录。for `*.cmake`
- 5. `CMAKE_INCLUDE_PATH`、`CMAKE_LIBRARY_PATH`，为后续查找头文件和库指定位置。`for find_*`

`CMAKE_INCLUDE_PATH`、`CMAKE_LIBRARY_PATH` 用法：
```
list(APPEND CMAKE_INCLUDE_PATH "/home/zhangrongrong/.local/protobuf/include")
list(APPEND CMAKE_LIBRARY_PATH "/home/zhangrongrong/.local/protobuf/lib")
set(protobuf_MODULE_COMPATIBLE TRUE)

or 

list(APPEND CMAKE_INCLUDE_PATH "/home/zhangrongrong/.local/protobuf/include")
list(APPEND CMAKE_LIBRARY_PATH "/home/zhangrongrong/.local/protobuf/lib")
include(FindProtobuf)
```
`CMAKE_INCLUDE_PATH`、`CMAKE_LIBRARY_PATH` 效果应该和 `set Protobuf_ROOT` 类似。


### 安装软件时，为 cmake 指定参数
安装 brpc 时，需要用到 protobuf，但是怎么修改 CMakeList.txt 都有问题。  
这时候可以直接：
```
cmake -DCMAKE_INCLUDE_PATH="/home/zhangrongrong/.local/protobuf/include" -DCMAKE_LIBRARY_PATH="/home/zhangrongrong/.local/protobuf/lib64" ..
```