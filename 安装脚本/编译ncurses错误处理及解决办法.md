编译ncurses错误处理及解决办法


编译ncurses是编译linux内核时用到的一个工具，在make时遇到了如下错误：
~~~bash
In file included from ../ncurses/curses.priv.h:283:0,
from ../ncurses/lib_gen.c:19:
_46863.c:835:15: error: expected ‘)’ before ‘int’
../include/curses.h:1594:56: note: in definition of macro ‘mouse_trafo’
#define mouse_trafo(y,x,to_screen) wmouse_trafo(stdscr,y,x,to_screen)  ^
Makefile:790: recipe for target '../objects/lib_gen.o' failed
make[1]: *** [../objects/lib_gen.o] Error 1
make[1]: Leaving directory '/home/ran/workspace/ncurses-5.9/ncurses'
Makefile:109: recipe for target 'all' failed
make: *** [all] Error 2
~~~
根据出错提示，找到ncurses目录下，include文件夹里的curses.h文件，查找mouse_trafo，锁定在1584行：
~~~bash
1583 extern NCURSES_EXPORT(bool)    wmouse_trafo (const WINDOW*, int*, int*, bool);
1584 extern NCURSES_EXPORT(bool)    mouse_trafo (int*, int*, bool);   /* generated */
~~~
应该是由后面的一行注释引起的，具体原因不详。
由于代码的部分是由另一个文件curses.tail导入的，所以要修改curses.tail：
~~~bash
sudo vim curses.tail
~~~

查找mouse_trafo，定位到104行，去除104行后面的注释：
~~~bash
103 extern NCURSES_EXPORT(bool)    wmouse_trafo (const WINDOW*, int*, int*, bool);
104 extern NCURSES_EXPORT(bool)    mouse_trafo (int*, int*, bool);   /* generated */
~~~

~~~bash
保存后退出，重新make即可。
sudo make clean
sudo make
sudo make install
~~~