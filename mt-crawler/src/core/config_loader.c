#include "config_loader.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <ctype.h>

//错误信息缓存
static char last_error[256];

//去处空白字符,static使其只能在本文件使用
static void trim(char* str) {
	if (str == NULL || *str == '\0') return;  // 安全检查
	char* end;
	//找到开头
	char* start = str;
	while (isspace((unsigned char)*start)) start++;
	//去除开头空白字符
	if (start != str) {
		size_t len = strlen(start) + 1;  // +1 包含 '\0'
		memmove(str, start, len);
	}
	//找到结尾
	end = str + strlen(str);
	if (end > str) end--;  // 避免空字符串或全空白字符串的问题
	while (end > str && isspace((unsigned char)*end)) end--;
	*(end + 1) = 0;  //添加结束符
}

//初始化配置
static void ini_config(CrawlerConfig* config)
{
	memset(config, 0, sizeof(CrawlerConfig));
	config->network.is_agent = false;
	config->network.is_proxy = false;
	config->network.timeout_sec = 30;
	config->network.max_retries = 5;
	config->thread.max_threads = 10;
	config->thread.queue_size = 100;
	strcpy(config->storage.data_dir, "./data");   //设置默认数据目录
	config->storage.max_size = 1048576; // 1MB
	config->storage.is_compress = false; 
}

//转换字符串为整数
static bool parse_int(const char* value, int* result, int min, int max) {
	char* end;
	long num = strtol(value, &end, 10);
	if (*end != '\0' || num<min || num>max) {
		snprintf(last_error, sizeof(last_error),
			"Invalid integer value: %s (range %d-%d)", value, min, max);
		return false;
	}
	if (num < INT_MIN || num > INT_MAX) {
		snprintf(last_error, sizeof(last_error), "Integer out of range: %s", value);
		return false;
	}
	*result = (int)num;
	return true;
}

//布尔值转换
static bool parse_bool(const char* value, bool* result) {
	if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) {
		*result = true;
		return true;
	}
	else if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0) {
		*result = false;
		return true;
	}
	else {
		snprintf(last_error, sizeof(last_error),
			"Invalid boolean value: %s (expected 0/1/true/false)", value);
		return false;
	}
}


//解析Ini文件
bool load_config(const char* filename, CrawlerConfig* config) {
	FILE* file = fopen(filename, "r");
	//检查文件是否打开成功
	if (!file) {
		snprintf(last_error, sizeof(last_error), "Failed to open the config : %s", strerror(errno));
		return false;
	}

	//初始化配置
	char line[256];
	int line_num = 0;
	char section[256] = { 0 };  //当前部分

	//初始化配置
	ini_config(config);

	//逐行读取文件
	while (fgets(line, sizeof(line), file))
	{
		//检查行长度
		size_t len = strlen(line);
		if (len > 0 && line[len - 1] != '\n') {
			snprintf(last_error, sizeof(last_error), "Line %d too long", line_num);
			fclose(file);
			return false;
		}
		line_num++;
		trim(line);  //去除空白字符

		if (line[0] == '#' || line[0] == '\n' || line[0] == ';') continue;  //跳过注释空行

		//处理头
		if (line[0] == '[') {
			char* end = strchr(line, ']');
			if (!end) {
				snprintf(last_error, sizeof(last_error),
					"Invalid section header at line %d", line_num);
				fclose(file);
				return false;
			}
			*end = '\0';   //添加结束符
			strncpy(section, line + 1, sizeof(line) - 1);
			continue;
		}

		//处理键值对
		char* sep = strchr(line, '=');
		if (!sep) continue;
		*sep = '\0';
		char* key = line;
		char* value = sep + 1;
		trim(key);
		trim(value);

		//分为：整数类型，bool类型，字符串类型
		if (strcmp(section, "network") == 0) {
			if (strcmp(key, "timeout_sec") == 0) {
				if (!parse_int(value, &config->network.timeout_sec, 0, 300)) {
					fclose(file);
					return false;
				}
			}
			else if (strcmp(key, "max_reties") == 0) {
				if (!parse_int(value, &config->network.max_retries, 0, 50))
				{
					fclose(file);
					return false;
				}
			}
			else if (strcmp(key, "usr_agent") == 0) {
				strncpy(config->network.usr_agent, value, sizeof(config->network.usr_agent) - 1);
				config->network.usr_agent[sizeof(config->network.usr_agent) - 1] = '\0';  // 确保终止
			}
			else if (strcmp(key, "usr_addr") == 0) {
				strncpy(config->network.usr_addr, value, sizeof(config->network.usr_addr) - 1);
				config->network.usr_addr[sizeof(config->network.usr_addr) - 1] = '\0';  // 确保终止
			}
			else if (strcmp(key, "is_agent") == 0) {
				if (!parse_bool(value, &config->network.is_agent))
				{
					fclose(file);
					return false;
				}
			}
			else if (strcmp(key, "is_proxy") == 0) {
                                if (!parse_bool(value, &config->network.is_proxy))
                                {
                                        fclose(file);
                                        return false;
                                }
                        }
		}
		else if (strcmp(section, "storage") == 0) {
			if (strcmp(key, "data_dir") == 0) {
				strncpy(config->storage.data_dir, value, sizeof(config->storage.data_dir) - 1);
				config->storage.data_dir[sizeof(config->storage.data_dir) - 1] = '\0';  // 确保终止
			}
			else if (strcmp(key, "max_size") == 0) {
				long size;
				if (!parse_int(value, (int*)&size, 1024, 104857600)) // 1KB-100MB
				{
					fclose(file);
					return false;
				}
				config->storage.max_size = (size_t)size;
			}
			else if (strcmp(key, "is_compress") == 0) {
				if (!parse_bool(value, &config->storage.is_compress))
				{
					fclose(file);
					return false;
				}
			}
		}
		else if (strcmp(section, "thread") == 0) {
			if (strcmp(key, "max_threads") == 0) {
				if (!parse_int(value, &config->thread.max_threads, 1, 64))
				{
					fclose(file);
					return false;
				}
			}
			else if (strcmp(key, "queue_size") == 0) {
				if (!parse_int(value, &config->thread.queue_size, 10, 10000))
				{
					fclose(file);
					return false;
				}
			}
		}
	}
	fclose(file);
	return true;
}

//获取错误信息
const char* get_config_error() {
	return last_error;
}
