## Introduction
这部分简单介绍了其他论文的工作，大致介绍自己架构做了什么，然后直接抛出论文的贡献点：
1. 呈现了 GD2L，不同于其他已实现的，该算法具有成本感知特性。 
2. CAC。
3. 实验对比。


## OVERVIEW
这部分解释了 buffer pool、SSD、HDD 之间的关系，数据如何在这三个部分流动。

## GD2L
个人觉得这部分没什么太创新的东西，主要改造了 InnoDB 的 LRU 链表，原来 InnoDB 的 LRU 链表是一个链表，但是GD2L 中，认为分了两个链表，一个链表对应 SSD，另一个对应 HDD。SSD中页面驱逐到 HDD，或者 HDD 中页面要放到 SSD ，会造成两个链表的部分结构体转移。

更多的还是讲 InnoDB 中的链表结构。


## 贡献2 - SSD管理

该部分主要解决的问题是，SSD 容量有限，在快要满时，选择那些页面替换进 HDD。

最笨的方法，基于历史访问成本（实验中提到的 CC 方法）。公式如下：

![](https://latex.codecogs.com/png.download?%5Cinline%20B%28p%29%3Dr%28p%29%28R_%7BD%7D-R_%7BS%7D%29+w%28p%29%28W_%7BD%7D-W_%7BS%7D%29)

> 该公式计算某个页面的收益，其 r(p) 为统计的读次数，w(p) 为写次数，R 为单词读写磁盘的时间。


![](https://latex.codecogs.com/png.download?%5Cinline%20B%28p%29%3D%28%5Cwidehat%7Br_%7BD%7D%7D%28p%29R_%7BD%7D-%5Cwidehat%7Br_%7BS%7D%7D%28p%29R_%7BS%7D%29+%28%5Cwidehat%7Bw_%7BD%7D%7D%28p%29W_%7BD%7D-%5Cwidehat%7Bw_%7BS%7D%7D%28p%29W_%7BS%7D%29)

![](https://latex.codecogs.com/png.download?%5Cinline%20%5Cwidehat%7Br_%7BS%7D%7D%3Dr_%7BS%7D+%7B%5Calpha%7Dr_%7BD%7D)

![](https://latex.codecogs.com/png.download?%5Cinline%20%5Cwidehat%7Br_%7BD%7D%7D%3Dr_%7BD%7D+%5Cfrac%20%7B1%7D%7B%5Calpha%7Dr_%7BS%7D)

CAC 改进了公式，页面的访问次数为估计次数，具体做法是通过未命中率来估计。
读写次数加上了一个因式，因子 alpha 则由未命中率来确定。
具体来讲：

![](https://latex.codecogs.com/png.download?%5Cinline%20%5Calpha%3D%5Cfrac%7Bm_%7BS%7D%7D%7Bm_%7BD%7D%7D)

mS 为物理访问 SSD 的未命中次数，mD为物理访问 HDD 的未命中次数，但是这样太粗糙，CAC 又根据表格和和时间片对这个未命中次数做的分组，这样一来，估计值就更加精确。

## 实验



实验主要回答两个问题：

1）：GD2L 和非成本感知的对比有多好？结合 CAC 方法有多重要？
2）：和其他数据库里的相关方案对比。

缓冲区管理：
1）：InnoDB 的 LRU; 2) GD2L

SSD 管理：
1）CC，基于历史成本，CAC 的最原始算法；2）LRU2，实现在 SQLServer 中的一个算法；3）MV-FIFO, 这个优势是顺序写。

方法及参数：
三个场景：1）数据库大小远大于 SSD 容量；2）稍大于； 3）小于
SSD 固定为 10 G，数据库大小根据仓库数不同分别为 （80， 150， 300）个，即（8， 15， 300）G大小。

buffer pool 大小为 SSD 的 10%， 20%， 40%。

CAC outqueue 链表为 SSD 的 10%。

### 第一组实验对比 : LRU+CC,GD2L+CC, GD2L+CAC 下的吞吐量。
数据库大小 8G, 15G, 30G，buffer pool 大小 1G, 2G, 4G。
同时用了磁盘利用率和读写速度来佐证上面柱状图结果。

用两小节回答了两个问题：
1）GD2L 优于 LRU
2）CAC 优于 CC

### 第二组实验对比
方法和第一组类似，但是着重比较了 GD2L+CAC 比其他的组合。