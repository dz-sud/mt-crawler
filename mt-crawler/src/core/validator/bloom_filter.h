#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    unsigned char *bit_array;  // 位数组
    size_t size;               // 位数组大小（bit数）
    size_t hash_func_count;    // 哈希函数数量
} BloomFilter;

// 初始化布隆过滤器
void bloom_filter_init(BloomFilter *bf, size_t expected_elements);

// 检查元素是否存在
bool bloom_filter_contains(const BloomFilter *bf, const char *element);

// 添加元素到过滤器
void bloom_filter_add(BloomFilter *bf, const char *element);

// 释放资源
void bloom_filter_free(BloomFilter *bf);

#endif

