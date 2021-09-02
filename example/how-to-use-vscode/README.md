task 和 launch


task 描述了一些任务，可以通过这些配置，快捷地运行一些程序，比如构建项目、编译、运行等都可以。
launch 则是 debug 需要的配置文件。

直接在面板搜索 `task`, 或者在快捷键配置文件里，添加以下配置快速调起 task 执行命令：
```
{
        "key": "ctrl+shift+b",
        "command": "workbench.action.tasks.runTask"
}
```

test 简单演示了如何使用 task 以及 launch 调试，task 通过 gcc 编译出目标文件，launch 则直接调试 task 编译出的可执行程序。

test-cmake 进一步在 task 中构建了 cmake、make 等步骤。
