#ifndef STORAGE_H
#define STORAGE_H

#include "downloader.h"
#include<stdbool.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include "config_loader.h"

typedef struct {
    char data_dir[256];         // 数据存储根目录
    size_t max_file_size;       // 单个文件最大字节数
    bool compress_data;         // 是否启用压缩
    pthread_mutex_t file_lock;  // 文件操作互斥锁
}StorageEngine;

typedef struct {
    char url[256];         //数据来源
    time_t timestamp;           // 抓取时间戳
    size_t raw_size;            // 原始数据大小
    char file_path[512];        // 存储相对路径
}MetaRecord;

bool init_storage_engine(StorageEngine* engine, const CrawlerConfig* config);
bool storage_save(StorageEngine* engine, const char* url, DownloadResult* result);

#endif

