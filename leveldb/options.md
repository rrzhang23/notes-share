```cpp
#ifndef STORAGE_LEVELDB_INCLUDE_OPTIONS_H_
#define STORAGE_LEVELDB_INCLUDE_OPTIONS_H_

#include <stddef.h>
#include "leveldb/export.h"

namespace leveldb {

    class Cache;
    class Comparator;
    class Env;
    class FilterPolicy;
    class Logger;
    class Snapshot;

    // 存储数据的压缩方式, SnappyCompression 是 google 的一种压缩方法
    enum CompressionType {
        kNoCompression = 0x0,
        kSnappyCompression = 0x1
    };


    struct LEVELDB_EXPORT Options {
        // 数据库 entry.key 的比较方式, 默认为字典序
        // 要求：统一数据库需要在任何时间需要使用相同的
        const Comparator *comparator;

        // 若找不到 manifest 文件，是否重新新建 DB
        bool create_if_missing = false;

        bool error_if_exists = false;

        // paranoid 议为偏执狂, paranoid_checks 即进行严格的检查,
        // 哪怕是一个 entry 发生错误，则整个数据库停止运行
        bool paranoid_checks = false;

        Env *env;
		
        // 状态日志
        Logger *info_log = nullptr;

        // 缓冲区大小 4MB
        size_t write_buffer_size = 4 * 1024 * 1024;
		
        // 最多缓存 1000 个 SSTable 元信息
        int max_open_files = 1000;
    
        Cache *block_cache = nullptr;

        size_t block_size = 4 * 1024;
		
        int block_restart_interval = 16;

        size_t max_file_size = 2 * 1024 * 1024;

        CompressionType compression = kSnappyCompression;

        bool reuse_logs = false;

        const FilterPolicy *filter_policy = nullptr;

        Options();
    };

    struct LEVELDB_EXPORT ReadOptions {
        bool verify_checksums = false;
        bool fill_cache = true;
        const Snapshot *snapshot = nullptr;
        ReadOptions() = default;
    };

    struct LEVELDB_EXPORT WriteOptions {
        bool sync = false;
        WriteOptions() = default;
    };

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_OPTIONS_H_
```