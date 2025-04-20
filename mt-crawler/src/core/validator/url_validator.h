#ifndef URL_VALIDATOR_H
#define URL_VALIDATOR_H

#include <stdbool.h>

//初始化url验证器，验证是否为合法url，是否被访问
void ini_validator_url();

//验证url是否合法
bool is_url_format_valid(const char* url);

//url是否被访问过
bool is_url_visited(const char* url);

//将url标记访问
void mark_url_visited(const char* url);

//释放资源
void url_validator_cleanup();
#endif
