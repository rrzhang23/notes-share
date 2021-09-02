
## socket
#### server  
```
1. int server_fd = soeket(...);     // 创建 socket
2. struct sockaddr_in server_addr;              // 初始化 socket intenet 地址信息
3. int bind(server_fd, server_addr...)     // socket fd 和监听地址绑在一起
4. listen(server_fd, ...)           // 在 fd 上监听
5. int conn = accept(server_fd, client_addr, ...)   // 客户端有连接请求时，建立连接
6. 读写等操作。
7. close(conn); close(server_fd);
```

#### client
```
1. int client_fd = socket(...);
2. struct sockaddr_in addr;  
3. int connect(client_fd, server_addr, ...);        // 和服务端 accept 对应。
4. 读写
5. close(client_fd);
```

> 当客户端请求连接时，服务端并不能知道客户端通过哪个端口发送的，只能建立一个通道，通过 fd 在通道中读/写。

## libevent

> 参考：  
> [libevent中的bufferevent原理](https://www.cnblogs.com/nengm1988/p/8203784.html)  
> [Libevent源码分析-----连接监听器evconnlistener](https://blog.csdn.net/luotuo44/article/details/38800363)
> 
#### 数据结构
```
struct event        // 可以是一个 accept 函数，也可以是 read/write 函数，可以绑定任何回调函数
struct event_base   // event 集合
struct evconnlistener   // socket listen 事件

```
#### 函数
```
// 结构体初始化
event_new()
event_base_new()

// 事件循环
event_base_dispatch()
event_base_loop()
```

##### evconnlistener
```
结构体：
struct evconnlistener；
将 socket 监听抽象为一个事件，称为 evconnlistener。  

函数：  
evconnlistener_new_bind();
一个函数取代了 socket 中 socket()、bind()、listen()、accept() 一系列函数。
客户端连接时，已经调用了accpet 函数，并传了 fd 给 listener_cb 回调函数。
```

##### bufferevent

通常每个 socket 监听/链接都会对应一个 bufferevent。server 端在调用 evconnlistener_new_bind() 后，只要有一个客户端来链接，listener_cb 函数就会创建一个 bufferevent 处理读写、错误事件；客户端也会在建立连接时，创建一个 bufferevent 进行读写。


### server
```
1. struct evconnlistener *listener;
2. listener = evconnlistener_new_bind(struct event_base, &listener_cb, sockaddr_in, ...);    // 进行监听，并传入了监听回调函数 listener_cb，客户端连接时，已经调用了accpet 函数，并传了 fd 给 listener_cb 回调函数。
3. 客户端连接时，listener_cb被调用，创建 bufferevent 处理请求。
```

### client
```

```

