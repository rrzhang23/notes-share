
~~~
cmake_minimum_required(VERSION 2.8)
project(Dbx1000)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_definitions(-DNOGRAPHITE=1)

# include header files
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

# add_library(a a.cpp)
# add_library(a SHARED a.cpp)
# add_library(b b.cpp)
# add_library(b SHARED b.cpp)

link_directories(${PROJECT_SOURCE_DIR}/lib)
link_directories(${PROJECT_SOURCE_DIR}/libs)

add_executable(main main.cpp)
# target_link_libraries(main a)          # ok
# target_link_libraries(main liba.so)    # ok
# target_link_libraries(main liba.a)     # ok

# target_link_libraries(main -l:a)       # false
# target_link_libraries(main -lliba.a)   # falase
# target_link_libraries(main -l:liba.a)  # ok
# target_link_libraries(main -l:liba.so) # ok


# target_link_libraries(main liba.a.2)     # false
# target_link_libraries(main liba.so.1)    # ok
# target_link_libraries(main -l:liba.a.2)  # ok
# target_link_libraries(main -l:liba.so.1) # ok



# target_link_libraries(main a b)                   # ok
# target_link_libraries(main liba.so;libb.so)       # ok
# target_link_libraries(main liba.a;libb.a)         # ok
# target_link_libraries(main -l:a -l:b)             # false

# target_link_libraries(main -lliba.a -llibb.a)     # false
# target_link_libraries(main -l:liba.a -l:libb.a)   # ok
# target_link_libraries(main -l:liba.so -l:libb.so) # ok
# target_link_libraries(main -l:liba.a;libb.a)      # ok
# target_link_libraries(main -l:liba.so;libb.so)    # ok
# target_link_libraries(main -l:liba.so;-l:libb.so) # ok

# target_link_libraries(main -l:liba.so.1 -l:libb.so.1) # ok
# target_link_libraries(main -l:liba.so.1;-l:libb.so.1) # ok
# target_link_libraries(main -l:liba.a.2 -l:libb.a.2)   # ok
# target_link_libraries(main -l:liba.a.2;-l:libb.a.2)   # ok
~~~


> 可以使用 libxxx.so/a 的简写 xxx，可以使用全名，可以使用 -l:libxxx.so/a，但必须后面是全名，后缀加版本号也行  
> 可以使用 libxxx.so/a.1.1 这样的后缀，-l:libxxx.so/a.1.1 也行  
> 链接多个库时，空格可以用分号代替，-l: 也可以加在所有库名字前  
> 即使可以使用 -l:libxxx.a 代替，但是 cmake 会自动在库名字前加上 -l，cmake::target_link_libraries 中 -llibxxx.a 写法是不行的  



