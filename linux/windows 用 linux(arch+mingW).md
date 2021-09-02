arch(mysys2) 装上后和linux子系统差不多，或者说和cygwin也差不多。
系统里多了个文件夹，mysys64，而cygwin多了个cygwin64。
该文件夹下内容和linux差不多。

arch 用 pacman 管理包，cygwin用类似apt-get管理，大同小异。

不用管太多上面的东西。cygwin 安装在系统里的包（例如gcc, make, 都是可以直接用的。）  
但是，用类似的方法，在 mysys里直接用 `pacman -S gcc` 装的东西，用clion测试时，时有问题的（gcc就通不过）。

这里在用 `pacmac -S gcc`是装到linux系统目录/usr/bin下，clion 选择 mingw是用不了mysys /mysys64/usr/bin下的东西的（可以用cygwin /cygwin64/usr/bin下的东西）。

所以在 `gcc` 前需要加上前缀 ` mingw-w64-x86_64-`，例如 `pacman -S  mingw-w64-x86_64-gcc`，安装其他包也是如此，会把包安装到 /mysys64/mingW64/bin 下。

> clion 用 mysys+mingW，需要全套用 mingW 的（包括 cmake、make、gcc、g++）。

>用mingw32-make.exe 编译找不到 .dll 时，配置该路径到 PATH 变量，重新打开 cmd 就好了。会自动到exe 同目录下找 .dll 文件。

> pacman 下载出错，可以直接到原地址下载好，放到 /var/cache/pacman/pkg/ 下。

> pacman 指定版本   
> cd /var/cache/pacman/pkg  
> sudo pacman -U virtualbox-5.2.12-1-x86_64.pkg.tar.xz