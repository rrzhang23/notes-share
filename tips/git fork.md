各个项目都是 fork 过来的，所以共有三个 git 仓库：  
上游远程仓库、自己的远程仓库、本地仓库。

上游分支：  
Pokemon/master、Pokemon/other  
自己远程分支：  
rongzia/master、rongzia/zhang-dev  
本地分支:   
master、zhang-dev

同步上游分支流程：
```
git fetch Pokemon master
git checkout master
git merge Pokemon/master        // 同步到本地
git push rongzia master:master  // 同步到自己的远程
```

```
git checkout zhang-dev
git merge master
git push rongzia zhang-dev:zhang-dev
```

另外，自己在 zhang-dev作修改，  
所有的 commit、merge 用前缀来区分，  
message 以 rongzi 或 rongzia 开头，表示 zhang-dev 内不需要合并到 master 的提交