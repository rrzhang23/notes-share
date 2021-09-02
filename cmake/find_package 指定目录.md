通常 find_package 会从系统目录下去找目标，一般 include 在 `/usr/include`、`/usr/local/include` 下，bin 在`/usr/bin` 下。

### 没有对应的 xxx-config.cmake
但是我们会在自己目录下安装一些版本，区别于系统版本，就很难找到。这种情况下我们需要显示指定库(.a, .so)和头文件(include)文件夹的位置，通过 set、find_library、find_program、find_path 等命令：

```
set(XXX_ROOT ${ENV}/.local/xxx)
set((XXX_INCLUDE_DIR ${XXX_ROOT}/include)
find_library(XXX_LIBRARY libxxx.a ${XXX_ROOT}/lib NO_DEFAULT_PATH)
find_program(XXX_EXECUTABLE NAMES xxx PATHS ${XXX_ROOT}/bin NO_DEFAULT_PATH)
```


### 有对应的 xxx-config.cmake
另外很多 google 的工具，在通过 cmake 安装的时候，会在 lib 目录生成 cmake 文件夹，这个文件夹下有很多 *.cmake 文件，用于 find_package。  
这样就可以在 CMakeLists.txt 里找到该安装目录：
```
find_package(GTest QUIET PATHS /home/zhangrongrong/.local/gooletest NO_DEFAULT_PATH)
if(GTEST_FOUND)
  set(HAVE_GTEST 1)
endif(GTEST_FOUND)

find_package(Gflags QUIET PATHS /home/zhangrongrong/.local/gflag NO_DEFAULT_PATH)
if(GFLAGS_FOUND)
  set(HAVE_GFLAGS 1)
endif(GFLAGS_FOUND)
```

或者指定 xxx_ROOT：
```
cmake_policy(SET CMP0074 NEW)
set(GTest_ROOT "$ENV{HOME}/.local/googletest")
find_package(GTest REQUIRED)
```
> 参考：[深入理解CMake(3):find_package()的使用](https://www.jianshu.com/p/39fc5e548310)