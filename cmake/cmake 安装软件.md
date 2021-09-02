指定目录：
```
cmake -DCMAKE_INSTALL_PREFIX=/topath

or 编辑 CMakeList.txt，加入：
SET(CMAKE_INSTALL_PREFIX < install_path >)
```

> 参考[CMake 指定安装目录](https://blog.csdn.net/whatday/article/details/84104306)