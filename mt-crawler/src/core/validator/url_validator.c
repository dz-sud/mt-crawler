#include "url_validator.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include "MurmurHash3.h"  // 第三方哈希库
#include <stdint.h>
#include "bloom_filter.h"

#define URL_PATTERN "^https?://([a-z0-9-]+\\.)+[a-z]{2,6}(:[0-9]{1,5})?(/.*)?$"

static regex_t url_regex;
static pthread_mutex_t lock;
static BloomFilter bloom_filter;

// 初始化验证器
void ini_validator_url() {
	regcomp(&url_regex, URL_PATTERN, REG_EXTENDED | REG_ICASE);
	pthread_mutex_init(&lock, NULL);
	bloom_filter_init(&bloom_filter, 1000000); // 百万级容量
}

//验证url是否合法
bool is_url_format_valid(const char* url)
{
	// 基础检查
	if (!url || strlen(url) > 2048) return false;

	// 正则匹配
	return regexec(&url_regex, url, 0, NULL, 0) == 0;
}

//url是否被访问过
bool is_url_visited(const char* url)
{
	pthread_mutex_lock(&lock);
	bool exists = bloom_filter_contains(&bloom_filter, url);
	pthread_mutex_unlock(&lock);

	return exists;
}

//将url标记访问
void mark_url_visited(const char* url)
{
	pthread_mutex_lock(&lock);
	bloom_filter_add(&bloom_filter, url);
	pthread_mutex_unlock(&lock);
}

//释放资源
void url_validator_cleanup() {
	regfree(&url_regex);
	pthread_mutex_destroy(&lock);
	bloom_filter_free(&bloom_filter);
}
