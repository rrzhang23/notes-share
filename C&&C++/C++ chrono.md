# chrono
C++ 标准库

C++ chrono 包含了三个概念的抽象：duration(时间段)、time_point(时间点)、clock(时钟)

1. duration 用来表示一个时间段，原型是一个模板类：  
`template<typename _Rep, typename _Period> struct duration{}`
_Rep 表示类型，可以使 int, double 等类型  
_Period 是 ratio 型，表示以何种分数计数  

    一个 duration 表示 _Rep 个 _Period 时间，比如 1.02 秒可以这样表示：  
    `chrono::duration<double, ratio<1, 1>> du(1.02);`

    已经定义的几种 duration：  
    ```
    typedef duration<int64_t, nano> 	    nanoseconds;
    typedef duration<int64_t, micro> 	    microseconds;
    typedef duration<int64_t, milli> 	    milliseconds;
    typedef duration<int64_t> 		        seconds;
    typedef duration<int64_t, ratio< 60>>   minutes;
    typedef duration<int64_t, ratio<3600>>  hours;
    简写：
    duration<int64_t>、duration<double>
    ```
    转换：`duration_cast<duration>`  
    取值：`duration.count()`

    常用于 `std::this_thread::sleep_for()、std::timed_mutex::try_lock_for()` 等函数，作为入参。


2. time_point 表示某个从 1970年1月1日 开始经过的时间  
    `template<typename _Clock, typename _Dur> struct time_point{}`
    _Clock 表示使用何种时钟，C++ 实现了三种时钟：`system_clock、steady_clock、high_resolution_clock`  
    _Dur 表示以何种方式计算 1970年1月1日 到现在的间隔

    定义 time_point 有两种方式：  
    ```
    chrono::time_point<chrono::system_clock, chrono::nanoseconds> foo;  // 可简写成：chrono::time_point<chrono::system_clock>, system_clock 内部有一个 chrono::nanoseconds 对象，所以默认是以纳秒计算
    system_clock::time_point bar;
    ```  
    
    转换：`time_point_cast<duration>`  
    取值：`time_point.time_since_epoch().count()   // time_since_epoch` 返回 duration

3. clock 时钟的抽象  
    C++ 实现了三种时钟：system_clock、steady_clock、high_resolution_clock  

    steady_clock 和另外两个 clock 的time_point 相差很大  
    
    实现时间 - 字符转的转换等
 


```
void func1() {
    typedef chrono::duration<double, ratio<1, 1>> seconds_in_double;

    steady_clock::time_point start = steady_clock::now();
    std::this_thread::sleep_for(seconds_in_double(1));
    steady_clock::time_point end = steady_clock::now();

    //! duration<int64_t, ratio<1, 1000000000>>
    auto elapsed = end - start;

    cout << "this thread sleep for: " << elapsed.count() << " nanoseconds" << endl;
    cout << "this thread sleep for: " << duration_cast<seconds_in_double>(elapsed).count() << " nanoseconds" << endl;   //! 不丢失精度
    cout << "this thread sleep for: " << duration_cast<chrono::seconds>(elapsed).count() << " nanoseconds" << endl;     //! 丢失精度
}

void func2() {
    typedef duration<int64_t, ratio<3600 * 24 * 365>> years;
    chrono::time_point<chrono::system_clock, chrono::nanoseconds> foo = chrono::system_clock::now();
    cout << "now is: " << time_point_cast<seconds>(foo).time_since_epoch().count() << " seconds after 1970...";
    cout << ", about:" << time_point_cast<years>(foo).time_since_epoch().count() << " years after 1970" << endl;
    foo += seconds(1);
    this_thread::sleep_until(foo);
    chrono::system_clock::time_point bar = system_clock::now();
    cout << "this thread has sleep for 1 second, until:" << time_point_cast<seconds>(bar).time_since_epoch().count() << endl;
}

void func3() {

    //! steady_clock 和另外两个 clock 的time_point 相差很大
    cout << "system_clock:" << system_clock::now().time_since_epoch().count() << endl;
    cout << "steady_clock:" << steady_clock::now().time_since_epoch().count() << endl;
    cout << "high_resolution_clock:" << high_resolution_clock::now().time_since_epoch().count() << endl;

    system_clock::time_point now = system_clock::now();
    time_t tt = system_clock::to_time_t(system_clock::now());
    tm *p_tm = std::localtime(&tt);
    // 格式同 ctime
    cout << put_time(p_tm, "%c") << endl;
    cout << put_time(p_tm, "%Y-%m-%d %H:%M:%S") << std::endl;
    // 更精确的时间：
    auto difnanoseconds = now.time_since_epoch().count() - duration_cast<nanoseconds>(duration_cast<seconds>(now.time_since_epoch())).count();
    cout << put_time(p_tm, "%Y-%m-%d %H:%M:%S ") << difnanoseconds << std::endl;

}

int main() {
    func1();
    cout << endl;
    func2();
    cout << endl;
    func3();
    cout << endl;


    time_t t_now = time(NULL);
    tm *tm_now = localtime(&t_now);
    cout << ctime(&t_now);
    cout << asctime(tm_now);

    clock_t clock1 = clock();
    this_thread::sleep_for(seconds(1));
    clock_t clock2 = clock();
    cout << clock2 - clock1 << endl;

    return 0;
}
```

# time
C 中日期-字符串的转换。

`time_t` long int 类型，表示从 1970 到现在的秒数。  
`struct tm` 描述结构体，包含秒分时天等变量。

获取系统时间：
```
time_t tt;
time(&tt);

or

time_t tt = time(NULL);
```
time_t 转 tm:
```
time_t tt = time(NULL);
tm *ptm = localtime(&tt);
```
time_t、tm 转 char*，默认后面带 '\n'
```
time_t tt = time(NULL);
tm *tm_now = localtime(&tt);
char *time = ctime(&tt);
char *time = asctime(ptm);
```

> 另外有 clock() 函数用于计时，并不是很好用，推荐用 chrono 来计时。


## chrono::system_clock 和 time 联系:
time_point、time_t互转:
```
time_t tt = system_clock::to_time_t(system_clock::now());
system_clock::time_point now = system_clock::from_time_t(tt)；
```

C++ 也有标准输出时间字符串的函数：`put_time`，在头文件 <iomanip> 中。
```
system_clock::time_point now = system_clock::now();
time_t tt = system_clock::to_time_t(system_clock::now());
tm *p_tm = std::localtime(&tt);
// 格式同 ctime：
cout << put_time(p_tm, "%c") << endl;
cout << put_time(p_tm, "%Y-%m-%d %H:%M:%S") << std::endl;
// 更精确的时间：
auto difnanoseconds = now.time_since_epoch().count() - duration_cast<nanoseconds>(duration_cast<seconds>(now.time_since_epoch())).count();
cout << put_time(p_tm, "%Y-%m-%d %H:%M:%S ") << difnanoseconds << std::endl;
```