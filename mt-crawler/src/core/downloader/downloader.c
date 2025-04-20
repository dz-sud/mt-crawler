#include <stdlib.h>
#include "downloader.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include "url_validator.h"
#include "config_loader.h"

// 数据写入回调
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    DownloadResult *result = (DownloadResult*)userdata;
    size_t realsize = size * nmemb;
    
    // 扩容缓冲区
    result->data = realloc(result->data, result->size + realsize + 1);
    if(!result->data) return 0;
    
    // 追加数据
    memcpy(result->data + result->size, ptr, realsize);
    result->size += realsize;
    result->data[result->size] = '\0';

//
 	printf("data: %s",result->data);

    return realsize;
}

// 线程局部存储（每个线程独立CURL句柄）
static pthread_key_t curl_key;

// 初始化线程的CURL环境
static void init_thread_curl(const CrawlerConfig* config) {
    CURL *curl = curl_easy_init();
    if(curl) {
        // 通用配置
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);    // 跟随重定向
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);         // 最大重定向次数
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);          // 总超时30秒
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);   // 连接超时10秒
    }
    pthread_setspecific(curl_key, curl);
}

// 全局初始化
int downloader_init() {
    if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
        return -1;
    
    if(pthread_key_create(&curl_key, (void (*)(void*))curl_easy_cleanup) != 0) {
        curl_global_cleanup();
        return -1;
    }
    return 0;
}

// 执行下载（线程安全）
DownloadResult* download_url(const char *url, const CrawlerConfig *config) {
    CURL *curl = pthread_getspecific(curl_key);
    if(!curl) {
        init_thread_curl(config);
        curl = pthread_getspecific(curl_key);
        if(!curl) return NULL;
    }

    DownloadResult *result = calloc(1, sizeof(DownloadResult));
    if(!result) return NULL;

    // 配置本次请求参数
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, result);
    if (config->network.is_agent)
    	curl_easy_setopt(curl, CURLOPT_USERAGENT, config->network.usr_agent);
    if (config && config->network.is_proxy && config->network.usr_addr[0] != '\0') {
    	if (strstr(config->network.usr_addr, "http://") || 
            strstr(config->network.usr_addr, "https://")) {
        	// 如果是代理地址
            curl_easy_setopt(curl, CURLOPT_PROXY, config->network.usr_addr);
        }
        else {
        // 假设其他情况都是本地IP绑定
        curl_easy_setopt(curl, CURLOPT_INTERFACE, config->network.usr_addr);
        }
    }
    
    // 执行下载（带重试）
    int retry = 0;
    CURLcode res;
    while(retry < 3) {
        res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result->status_code);
            break;
        }
        retry++;
    }

    if(res != CURLE_OK) {
        free(result->data);
        free(result);
        return NULL;
    }
    return result;
}

// 释放资源
void download_result_free(DownloadResult *res) {
    if(res) {
        free(res->data);
        free(res);
    }
}
