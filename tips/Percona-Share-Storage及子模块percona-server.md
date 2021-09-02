本来在 github 用户 `PokemonWei` 下有一个仓库 `PokemonWei/Percona-Share-Storage` ，之前做开发一直是在该仓库新建分支下进行，即在该仓库下有个仓库 `zhang-dev`。原先的流程如下：  

1. local `zhang-dev` push--> remote `zhang-dev`
2. github 里发起 pull request 请求，将 remote `zhang-dev` 合并到 remote `master`.

现想把整个 `PokemonWei/Percona-Share-Storage` 仓库 fork 到自己账户 `rongzia` 下，然后自己本地仓库关联这个 `rongzia/Percona-Share-Storage` 仓库下分支 `master` 做开发，然后把改动合并到 `PokemonWei/Percona-Share-Storage`去，即下面的流程:

1. 自己仓库下： local `master` push--> remote `master`
2. github 里发起 pull request 请求。

> 设计到子模块的问题，比较复杂。  
> 只是将 `Percona-Share-Storage` fork 过来还不行，还要把 `percona-server` 也 fork 过来。

```
1. 先 fork Percona-Share-Storage、percona-server
2. 在 github 仓库下，新建对应的分支 zhang-dev，子模块也要
3. 下载 ftp 上的文件到服务器，然后：

$ cd percona-server
$ git branch zhang-dev
$ git checkout zhang-dev
$ git remote add rongzia git@github.com:rongzia/percona-server.git
// 以下二选一
1.
$ vim .gitignore    // 忽略 build 文件夹，加入:    /build
$ git add .
$ git commit -m "Updata .gitignore"
$ 建立远程仓库 rongzia/percona-server 联系。
$ git push rongzia zhang-dev:zhang-dev
2. 
$ git pull rongzia zhang-dev:zhang-dev


$ cd Percona-Share-Storage
$ git branch zhang-dev
$ git checkout zhang-dev
$ git remote add rongzias git@github.com:rongzia/Percona-Share-Storage.git
```
> 注意的地方：   
> 先动子模块，再动上层仓库。  
> 仓库和子模块仓库都要有自己的分支。  
> 本地嵌套的项目都要和上面的仓库建立 remote 关联。  
> .gitgnore 规则  

修改远程仓库联系：
```
// 利用:
$ git remote add name git@github.com...
$ git remote remove name
// 确保：
$ git remote -v
// 显示如下：
rongzia	git@github.com:rongzia/....git (fetch)
rongzia	git@github.com:rongzia/....git (push)

```

从远程 fork 源仓库同步：
```
git remote add upstream git@....
git fetch upstream          // 拉去远程分支
git merge upstream/master   // 合并到本地分支
git push rongzia master:master  // push 到自己的仓库
```