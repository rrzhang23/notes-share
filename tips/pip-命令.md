>linux下pip安装工具包默认存放路径:  
>`file:///usr/local/lib/python3.6/dist-packages`

## 更换国内pypi镜像
#### 国内pypi镜像源:  
阿里：https://mirrors.aliyun.com/pypi/simple  
中国科学技术大学：http://pypi.mirrors.ustc.edu.cn/simple/  
豆瓣：https://pypi.douban.com/simple  
清华：https://pypi.tuna.tsinghua.edu.cn/simple/

#### 指定单次安装源
```python
pip install <包名> -i https://pypi.tuna.tsinghua.edu.cn/simple/
```

#### 指定全局安装源
* 在unix和macos，配置文件为：$HOME/.pip/pip.conf
* 在windows上，配置文件为：%HOME%\pip\pip.ini
```python
[global]
timeout = 6000
index-url = https://pypi.tuna.tsinghua.edu.cn/simple
[install] 
trusted-host = pypi.tuna.tsinghua.edu.cn
```

## pip安装时ReadTimeoutError解决办法
1. `pip --default-timeout=100 install gevent`  
2. `pip --default-timeout=100 install -U pip`  
3. `更换国内pypi镜像 `  

>第三个貌似有用，之前每次pycharm新建pipenv项目有报错，  
>按照第三个做果然有效

## freeze  
#### 列出已安装的包：  
```
pip freeze 
or 
pip list
```

#### 导出到指定目录下 requirements.txt:  
```
pip freeze > <目录>/requirements.txt
```

>requirements.txt 保存了当前安装的所有包的名称  
>
>requirements.txt内容格式为：  
>```
>APScheduler==2.1.2  
>MySQL-Connector-Python==2.0.1  
>MySQL-python==1.2.3  
>```



## install & uninstall  
#### 在线安装
```
sudo pip install <包名> -i https://pypi.tuna.tsinghua.edu.cn/simple/
```
例：`pip install pipenv -i https://pypi.tuna.tsinghua.edu.cn/simple/`  

或批量安装
```
sudo pip install -r requirements.txt https://pypi.tuna.tsinghua.edu.cn/simple/
```

通过使用`==` `>=` `<=` `>` `<` 来指定版本，不写则安装最新版  

#### 指定安装路径
```
sudo pip install --install-option="--prefix=绝对路径" packageName -i https://pypi.tuna.tsinghua.edu.cn/simple/
```
例：  
`pip install --install-option="--prefix=/home/rrzhang/.local" pipenv -i https://pypi.tuna.tsinghua.edu.cn/simple/`  
lib 会安装在`/home/rrzhang/.local/lib/python3.6/site-packages/pipenv`下  
bin 会安装在`/home/rrzhang/.local/bin`下

[参考](https://www.cnblogs.com/lenmom/p/10189140.html): https://www.cnblogs.com/lenmom/p/10189140.html

#### 安装本地安装包  
1. `sudo pip install <目录>/<文件名> -i https://pypi.tuna.tsinghua.edu.cn/simple/` 或  
2. `sudo pip install --use-wheel --no-index --find-links=wheelhouse/ <包名> -i https://pypi.tuna.tsinghua.edu.cn/simple/`

><包名>前有空格  
>2.可简写为
`sudo pip install --no-index -f=<目录>/ <包名>`

#### 卸载包
```
sudo pip uninstall <包名>
或  
sudo pip uninstall -r requirements.txt
```

#### 升级包
```
sudo pip install -U <包名> -i https://pypi.tuna.tsinghua.edu.cn/simple/
或：  
sudo pip install <包名> --upgrade -i https://pypi.tuna.tsinghua.edu.cn/simple/
```


#### 下载包而不安装
```
sudo pip install <包名> -d <目录> -i https://pypi.tuna.tsinghua.edu.cn/simple/
或
sudo pip install -d <目录> -r requirements.txt -i https://pypi.tuna.tsinghua.edu.cn/simple/
```
---

## show 显示包所在的目录
`sudo pip show -f <包名>`

## search 搜索包
`sudo pip search <搜索关键字>`

## list 查询可升级的包
`sudo pip list -o`

## wheel 打包
`sudo pip wheel <包名>`

---

## pip 安装、卸载、升级
#### 安装  
```
wget https://bootstrap.pypa.io/get-pip.py
sudo python get-pip.py -i https://pypi.tuna.tsinghua.edu.cn/simple/
```

#### 卸载
```
sudo python -m pip uninstall pip
```

#### 升级pip
1. `pip install -U pip -i https://pypi.tuna.tsinghua.edu.cn/simple/` 或：  
2. `python -m pip install --upgrade pip -i https://pypi.tuna.tsinghua.edu.cn/simple/`
