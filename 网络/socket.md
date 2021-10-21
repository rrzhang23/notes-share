server:


~~~cpp
    int sock_fd = socket(PF_INET, SOCK_STREAM, 0);                  
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = 8888;
    bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    struct sockaddr_in client_addr;
    int conn = accept(sock_fd, (struct sockaddr *) &client_addr, ..);
~~~

以上是服务端的整个大致流程，socket() 创建一个 fd，bind() 把该fd 和一个地址 sockaddr_in 绑定到一起，然后 accept() 监听这个fd，监听的时候会传进去一个空的代表客户端的 sockaddr_in。

##### AF_INET 同 PF_INET

AF 表示 address family，PF 表示 Protocol Family，在 posix 中二者相同，见 `sys/socket.h` : `#define	AF_INET		PF_INET`。  
理论上建立 socket 时是指定协议，应该用 PF_xxxx，设置地址时应该用 AF_xxxx。
[关于PF_INET和AF_INET的区别](https://blog.csdn.net/xiongmaojiayou/article/details/7584211)

##### sockaddr_in
(_in 猜测是 Internet???)   
定义于 `#include<netinet/in.h>`，描述地址的协议、地址、端口等信息。是类似 sockaddr 的改进，unix 还有个 sockaddr_un。  
sockaddr:
~~~cpp
// sys/socket.h :
struct sockaddr
  {
    __SOCKADDR_COMMON (sa_);	/* Common data: address family and length.  */
    char sa_data[14];		/* Address data.  */
  };
~~~
__SOCKADDR_COMMON 是个宏，用于拼接字符串，在linux 中则替换成：`sa_family_t sa_family`，在unix中替换成 `sa_family_t sun_family`。  
`sa_family_t` 是个 unsigned short int，2字节。AF_INET、AF_INET6 等正好是个短整型。  

sockaddr_in:
~~~cpp
// <netinet/in.h>
struct sockaddr_in
  {
    __SOCKADDR_COMMON (sin_);
    in_port_t sin_port;			/* Port number.  */
    struct in_addr sin_addr;		/* Internet address.  uint32_t */

    /* Pad to size of `struct sockaddr'.  */
    unsigned char sin_zero[sizeof (struct sockaddr) -
			   __SOCKADDR_COMMON_SIZE -
			   sizeof (in_port_t) -
			   sizeof (struct in_addr)];
  };

/* Internet address.  */
typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
struct in_addr
  {
    in_addr_t s_addr;
  };
~~~
协议 2 字节，端口 2 字节，地址 4 字节，剩下的为 0。和 sockaddr 大小一致，所以传给 bind 时可以解析为 `sockaddr *`
> 为了兼容ipv6，除了 sockaddr、sockaddr_in,还有个 sockaddr_storage。
[struct sockaddr定义及延伸](https://blog.csdn.net/qinglinsan/article/details/51419548)


### 地址转换
<arpa/inet.h>：
~~~cpp
in_addr                     // uint32_t 
inet_addr("192.168.1.1")    // str to in_addr

// <arpa/inet.h>:
// n 表示 IN，为 in_addr 格式；a 表示 ASCII，为字符串，点分格式（比如"192.168.1.1"）
char * inet_ntoa(struct in_addr __in)
int inet_aton (const char *__cp, struct in_addr *__inp)

// 适用 IPv6, n 表示数值，p 为 presentation 表达式
inet_pton()
inet_ntop()
~~~

<netdb.h>：
~~~cpp
getaddrinfo 把字符串地址、端口 转换成 sockaddr_in 链表
~~~


### 字节序
字节序分为大端字节序和小端字节序：
大端字节序： 是指一个整数的高位字节（32-31bit）存储在内存的低地址处，低位字节（0-7bit）存储在内存的高地址处。
小端字节序： 是指一个整数的高位字节（32-31bit）存储在内存的高地址处，低位字节（0-7bit）存储在内存的低地址处。
现代PC大多采用小端字节序，所以小端字节序又被称为主机字节序。大端字节序也称为网络字节序。

C++写网络程序的时候，往往会遇到字节的网络顺序和主机顺序的问题。这是就可能用到htons(), ntohl(), ntohs()，htons()这4个函数。  
　　网络字节顺序与本地字节顺序之间的转换函数：  
　　　　htonl()–“Host to Network Long”  
　　　　ntohl()–“Network to Host Long”  
　　　　htons()–“Host to Network Short”  
　　　　ntohs()–“Network to Host Short”  
> 定义在 <netinet/in.h>

### 


### 头文件
<sys/socket.h> 处理套接字动作等，socket、bind、listen、accept、connect

<netinet/in.h> 定义了sockaddr、sockaddr_in、htonl 字节序转换函数等

<bits/socket.h> PF_INET 等宏

<arpa/inet.h> inet_pton 等地址转换

<netdb.h> gethostbyname 域名相关操作、getaddrinfo
