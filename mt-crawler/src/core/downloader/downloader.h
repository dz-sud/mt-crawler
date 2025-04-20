#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <curl/curl.h>
#include "config_loader.h"

typedef struct {
    char *data;      // 下载数据缓冲区
    size_t size;     // 数据长度
    int status_code; // HTTP状态码
} DownloadResult;

// 初始化下载模块（全局调用一次）
int downloader_init();

// 下载指定URL（线程安全）
DownloadResult* download_url(const char *url, const CrawlerConfig *config);

// 释放下载结果内存
void download_result_free(DownloadResult *res);

#endif
