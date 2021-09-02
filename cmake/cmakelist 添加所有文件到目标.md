```
file(GLOB NAME_SRC folder/*.h folder/*.c)
add_executable(sqlite-src ${NAME_SRC})
or
add_library(sqlite-src ${NAME_SRC})
```