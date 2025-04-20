#include "bloom_filter.h"
#include "MurmurHash3.h"  // 使用MurmurHash3作为基础哈希
#include <string.h>

// 计算最优哈希函数数量 (m/n ≈ 10时，k ≈ 7)
#define HASH_FUNC_COUNT 7

void bloom_filter_init(BloomFilter *bf, size_t expected_elements) {
    // 计算位数组大小（10倍于预期元素，降低误判率）
    bf->size = expected_elements * 10;
    bf->bit_array = calloc((bf->size + 7) / 8, sizeof(unsigned char));
    bf->hash_func_count = HASH_FUNC_COUNT;
}

static void get_hashes(const BloomFilter *bf, const char *element, uint32_t *hashes) {
    // 使用不同的seed生成多个哈希值
    for (int i = 0; i < HASH_FUNC_COUNT; i++) {
        MurmurHash3_x86_32(element, strlen(element), i, &hashes[i]);
        hashes[i] %= bf->size;  // 确保不越界
    }
}

bool bloom_filter_contains(const BloomFilter *bf, const char *element) {
    uint32_t hashes[HASH_FUNC_COUNT];
    get_hashes(bf, element, hashes);
    
    for (int i = 0; i < bf->hash_func_count; i++) {
        size_t pos = hashes[i];
        if (!(bf->bit_array[pos / 8] & (1 << (pos % 8)))) {
            return false;
        }
    }
    return true;
}

void bloom_filter_add(BloomFilter *bf, const char *element) {
    uint32_t hashes[HASH_FUNC_COUNT];
    get_hashes(bf, element, hashes);
    
    for (int i = 0; i < bf->hash_func_count; i++) {
        size_t pos = hashes[i];
        bf->bit_array[pos / 8] |= (1 << (pos % 8));
    }
}

void bloom_filter_free(BloomFilter *bf) {
    free(bf->bit_array);
    bf->bit_array = NULL;
    bf->size = 0;
}
