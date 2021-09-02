三者容易混淆。

都是用来隔离不同项目的依赖环境的工具。

起初只是用 pip 来管理依赖包，但是该工具把所有的包都安装在了 python 安装目录下，但是各个项目都有不同的依赖包，版本相同还好，依赖包版本不同就很难搞了。

所以需要用到 virtualenv 来从系统 python 安装目录复制一份 python 到某个单独的目录中，这样项目就可以单独依赖自己的依赖库，项目之间互不干扰。（复制环境可以选择连同其他第三方包一起复制，也可以不选择）

pyenv、pipenv 是 virtualenv 的套壳，美其名曰简化 virtualenv 操作，使用起来多有不便的地方。

实操：

假设有个项目 `test-flask` ，需要运行在 python3 下，同时用到依赖包 `flask`。

### virtualenv
```
# 安装 python3（好像是3.6）、virtualenv
sudo yum install python3 python36-devel
sudo pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple virtualenv

# 从 /usr/bin/python3 复制一份环境到 cd ~/.local/virtualenv/test-flask-env
mkdir ~/.local/virtualenv
cd ~/.local/virtualenv
virtualenv --python=/usr/bin/python3 --no-site-packages test-flask-env
# 也可以使用一下方式：
# virtualenv -p /usr/bin/python3 --no-site-packages test-flask-env
# 注：--no-site-packages 在 virtualenv 20 版本后不再使用，默认就是 --no-site-packages

# 切换到 test-flask-env 目录，在激活后，在shell 提示符前会有括号，之后再当前 shell 下，默认都是执行 test-flask-env 下的 python，除非失效环境。
cd ~/.local/virtualenv/test-flask-env
source ./bin/activate   // 激活当前环境
deactivate              // 失效当前环境

# 我们安装依赖（激活后，默认安装在 test-flask-env 中），然后切换到项目目录，并运行程序
cd ~/projects/test-flask
nohup uwsgi --ini uwsgi.ini &
```
总结来说，安装 virtualenv，然后使用 virtualenv 复制一份 python 到新的环境目录，然后在新的环境目录下激活，之后就可以 pip 安装依赖、用新的环境运行项目。

### pyenv
pipenv 应该和 pyenv 类似，没有测试。
```
# 下载
git clone https://github.com/pyenv/pyenv.git ~/.local/pyenv

# 可以用 pyenv 命令安装 python，不过可能遇到报错，默认安装在 pyenv/versions 下
pyenv install -v 3.8.5
pyenv uninstall -v 3.8.5

# 安装 virtualenv 插件
git clone https://github.com/yyuu/pyenv-virtualenv.git ~/.local/pyenv/plugins/pyenv-virtualenv

# 也可以通过 pyenv 调用 virtualenv，比如创建新的环境：
pyenv virtualenv -p /usr/bin/python3 test-flask-env

# 激活和失效：
[~]$ pyenv activate test-flask-env
(test-flask-env) [~]$ pyenv deactivate



# 其他操作：
# 当前目录下使用某个python版本，即在目录下创建文件 .python-version，文件内容即版本号，也可以是 system
pyenv local 3.8.5
# 全局使用python版本：
pyenv global 3.8.5
```
`pyenv install` 下载速度太慢，可以把需要的东西下载好放到 `pyenv/cache` 下。 

修改 `` 可以使得当前用户下指定 python 版本。

`.bashrc`:
```
export PYENV_ROOT=$HOME/.local/pyenv
export PATH=$PYENV_ROOT/bin:$PATH
eval "$(pyenv init -)"
```

pyenv 相当于在 virtualenv 套了层壳。