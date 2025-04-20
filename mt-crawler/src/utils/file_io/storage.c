#include "storage.h"
#include <stdlib.h>
#include <openssl/md5.h>
#include <stdio.h>
#include "downloader.h"
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <zlib.h>
#include "config_loader.h"
#include <string.h>
#include <errno.h>
#include "metarecord.h"

static bool touch_p(const char* path) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "touch %s", path);

    if (system(cmd) == -1) {
        fprintf(stderr, "Error creating file %s: %s\n", path, strerror(errno));
        return false;
    }
    memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "chmod 777 %s", path);

    if (system(cmd) == -1) {
        fprintf(stderr, "Error setting permissions for file %s: %s\n", path, strerror(errno));
        return false;
    }
    return true;
}

static bool mkdir_p(const char* path) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", path);

    if (system(cmd) == -1) {
        fprintf(stderr, "Error creating directory %s: %s\n", path, strerror(errno));
        return false;
    }
    return true;
}

// 拼接路径辅助函数
static char* strcat_dup(const char* dir, const char* subpath) {
    char* path = malloc(strlen(dir) + strlen(subpath) + 1);
    strcpy(path, dir);
    strcat(path, subpath);
    return path;
}

bool init_storage_engine(StorageEngine* engine, const CrawlerConfig* config) {
    strncpy(engine->data_dir, config->storage.data_dir, sizeof(engine->data_dir));
    engine->max_file_size = config->storage.max_size;  // 修正字段名
    engine->compress_data = config->storage.is_compress;  // 修正字段名
    pthread_mutex_init(&engine->file_lock, NULL);

    char* raw_path = strcat_dup(engine->data_dir, "/raw");
    char* meta_path = strcat_dup(engine->data_dir, "/meta");

    bool ret = mkdir_p(engine->data_dir) &&
        mkdir_p(raw_path) &&
        mkdir_p(meta_path);

    free(raw_path);
    free(meta_path);
    return ret;
}

bool storage_save(StorageEngine* engine, const char* url, DownloadResult* result) {
    char file_hash[65];
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((unsigned char*)url, strlen(url), digest);
    for(int i = 0; i < MD5_DIGEST_LENGTH; i++)
	sprintf(&file_hash[i*2], "%02x", digest[i]);

    char raw_path[512];
    snprintf(raw_path, sizeof(raw_path), "%s/raw/%s.html", engine->data_dir, file_hash);

    //if (result->size > engine->max_file_size) {
    //    fprintf(stderr, "File too large: %zu bytes\n", result->size);
    //    return false;
    //}
    
    printf("data : %s",result->data);

    touch_p(raw_path);	
    pthread_mutex_lock(&engine->file_lock);

    FILE* fp = fopen(raw_path, "wb");
    if (!fp) {
        pthread_mutex_unlock(&engine->file_lock);
	fprintf(stderr, "Failed to open %s: %s\n", raw_path, strerror(errno));
        return false;
    }
    //
    //	printf("Failed to open %s\n", raw_path);
    bool success = false;
    /*if (engine->compress_data) {
        size_t compressed_size = 0;
        void* compressed = zlib_compress(result->data, result->size, &compressed_size);
        if (compressed) {
            success = (fwrite(compressed, 1, compressed_size, fp) == compressed_size);
            free(compressed);
        }
    }
    else {*/
        success = (fwrite(result->data, 1, result->size, fp) == result->size);
    //}

    fclose(fp);
    pthread_mutex_unlock(&engine->file_lock);

    if (success) {
	printf("success!");
    	    MetaRecord record;
        fill_meta_record(&record, url, raw_path, result->size);
        save_meta_record(engine, &record);
    }
    else {
        remove(raw_path);  // 删除不完整的文件
    }

    return success;
}
