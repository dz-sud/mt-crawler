#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H
#include <stdbool.h>
#include <stddef.h>

//网络配置
typedef struct {
	int timeout_sec;   //每次请求的超时时间、
	int max_retries;    //最大重试次数
	char usr_agent[256];  //用户代理
	bool is_agent;      //是否用户代理
	bool is_proxy;
	char usr_addr[256];   //用户地址
}NetworkConfig;

//存储配置
typedef struct {
	char data_dir[256];   //数据存储目录
	size_t max_size;      //最大存储大小
	bool is_compress;     //是否压缩保存
}StorageConfig;

//线程配置
typedef struct {
	int max_threads;   //同时运行的最大线程数
	int queue_size;     //最大队列
}ThreadConfig;

//主配置
typedef struct {
	NetworkConfig network;
	StorageConfig storage;
	ThreadConfig thread;
}CrawlerConfig;

//函数申明
bool load_config(const char* filename, CrawlerConfig* config);
const char* get_config_error();



#endif
