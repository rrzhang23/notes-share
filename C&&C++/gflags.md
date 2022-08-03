testgflags.h:
~~~cpp
#include "gflags/gflags.h"

DECLARE_string(ip);
DECLARE_int32(port);
~~~
testgflags2.cpp
~~~cpp
#include "testgflags.h"

DEFINE_string(ip, "0.0.0.0", "ip addr string");
DEFINE_int32(port, 1, "port");
~~~
testgflags.cpp
~~~cpp
#include <iostream>
#include "testgflags.h"
using namespace std;

static bool Vliad(const char* port, gflags::int32 value) {
    if(value >=0 && value < 65536) {
        return true;
    }
    return false;
}

// DEFINE_validator(port, &Vliad);

int main(int argc, char** argv) {
    for(int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    cout << endl << "start parse" << endl;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    // 解析后的 argc、argv中对应的值将会删除，剩下的参数顺序可能打乱。
    for(int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    cout << "end parse" << endl << endl;

    cout << "FLAGS_ip and FLAGS_port:   " << FLAGS_ip << ":" << FLAGS_port << endl;

    gflags::ShutDownCommandLineFlags();
    return 0;
}
~~~