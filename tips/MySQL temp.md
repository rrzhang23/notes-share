```
./oltpbenchmark -b tpcc -c config/sample_tpcc_config.xml --create=true --load=true --execute=true -s 5 -o outputfile
./oltpbenchmark -b tpcc -c config/sample_tpcc_config.xml --create=true --load=true
./oltpbenchmark -b tpcc -c config/sample_tpcc_config.xml --execute=true -s 5 -o outputfile

~/mysql2/install/bin/mysqld_safe --skip-external-locking  --defaults-file=~/mysql2/my.cnf --user=zhangrongrong &
~/mysql1/install/bin/mysql -P3307 --socket=$HOME/mysql1/tmp/mysql.sock -uroot -p'123456'
~/mysql2/install/bin/mysql -P3308 --socket=$HOME/mysql2/tmp/mysql.sock -uroot -p'123456'
alter user 'root'@'localhost' identified by '123456';
```