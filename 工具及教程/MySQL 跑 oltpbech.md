#### 工具 ant、maven，仓库 oltpbench
```
cd ~/.local
wget https://mirrors.huaweicloud.com/apache/ant/binaries/apache-ant-1.9.13-bin.zip
wget https://mirrors.huaweicloud.com/apache/maven/maven-3/3.6.2/binaries/apache-maven-3.6.2-bin.zip
unzip apache-ant-1.9.13-bin.zip > /dev/null
unzip apache-maven-3.6.2-bin.zip > /dev/null

git clone https://github.com/oltpbenchmark/oltpbench.git
```
.bashrc:
```
# ant
export ANT_HOME=$HOME/.local/apache-ant-1.9.13
export PATH=$ANT_HOME/bin:$PATH

# maven
export M3_HOME=$HOME/.local/apache-maven-3.6.2
export PATH=$M3_HOME/bin:$PATH

[]$ source ~/.bashrc
```


#### 下载 oltpbench 依赖库，下载 mysql 驱动jar包。 
```
cd oltpbench

ant bootstrap
ant resolve
ant build

cd oltpbench/lib
wget https://repo1.maven.org/maven2/mysql/mysql-connector-java/8.0.18/mysql-connector-java-8.0.18.jar
```

更改 lib/sample_tpcc_config.xml 中 URL和密码:
```
vim oltpbench/config/sample_tpcc_config.xml
// 修改为 jdbc:mysql://localhost:3306/tpcc?useSSL=false
```

执行 `./oltpbenchmark -b tpcc -c config/sample_tpcc_config.xml --create=true --load=true --execute=true -s 5 -o outputfile` 貌似报错，说建库错误。自己先连上 mysql ,执行 `create database tpcc;` 建个库好了。然后再执行：
```
./oltpbenchmark -b tpcc -c config/sample_tpcc_config.xml --create=true --load=true --execute=true -s 5 -o outputfile
```

其他报错：
```
Exception in thread "main" java.lang.RuntimeException: Unexpected error when trying to load the TPCC database
	at com.oltpbenchmark.api.BenchmarkModule.loadDatabase(BenchmarkModule.java:318)
	at com.oltpbenchmark.DBWorkload.runLoader(DBWorkload.java:823)
	at com.oltpbenchmark.DBWorkload.main(DBWorkload.java:559)
Caused by: java.sql.SQLNonTransientConnectionException: Public Key Retrieval is not allowed
	at com.mysql.cj.jdbc.exceptions.SQLError.createSQLException(SQLError.java:110)
	at com.mysql.cj.jdbc.exceptions.SQLError.createSQLException(SQLError.java:97)
	at com.mysql.cj.jdbc.exceptions.SQLExceptionsMapping.translateException(SQLExceptionsMapping.java:122)
	at com.mysql.cj.jdbc.ConnectionImpl.createNewIO(ConnectionImpl.java:836)
	at com.mysql.cj.jdbc.ConnectionImpl.<init>(ConnectionImpl.java:456)
	at com.mysql.cj.jdbc.ConnectionImpl.getInstance(ConnectionImpl.java:246)
	at com.mysql.cj.jdbc.NonRegisteringDriver.connect(NonRegisteringDriver.java:199)
	at java.sql.DriverManager.getConnection(DriverManager.java:664)
	at java.sql.DriverManager.getConnection(DriverManager.java:247)
	at com.oltpbenchmark.api.BenchmarkModule.makeConnection(BenchmarkModule.java:114)
	at com.oltpbenchmark.api.Loader$LoaderThread.<init>(Loader.java:56)
	at com.oltpbenchmark.benchmarks.tpcc.TPCCLoader$1.<init>(TPCCLoader.java:80)
	at com.oltpbenchmark.benchmarks.tpcc.TPCCLoader.createLoaderThreads(TPCCLoader.java:80)
	at com.oltpbenchmark.api.BenchmarkModule.loadDatabase(BenchmarkModule.java:299)
	... 2 more
```
用另一个窗口链接以下mysql，不要退出。

