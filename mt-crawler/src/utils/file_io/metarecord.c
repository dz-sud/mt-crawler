#include "storage.h"
#include <stdint.h>
#include <errno.h>  // 提供errno定义
#include<stdbool.h>
#include "downloader.h"
#include "metarecord.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <openssl/md5.h>

void fill_meta_record(MetaRecord* metarecord, const char* url, const char* raw_path, const int size)
{
	strncpy(metarecord->file_path, raw_path, sizeof(metarecord->file_path) - 1);
	strncpy(metarecord->url, url, sizeof(metarecord->url) - 1);
	metarecord->file_path[sizeof(metarecord->file_path) - 1] = '\0';
	metarecord->url[sizeof(metarecord->url) - 1] = '\0';
	metarecord->raw_size = size;
	metarecord->timestamp = time(NULL);
}

void save_meta_record(StorageEngine* engine, const MetaRecord* metarecord)
{
	char url_hash[33];
	unsigned char digest[MD5_DIGEST_LENGTH];
	MD5((unsigned char*)metarecord->url, strlen(metarecord->url), digest);
	for(int i = 0; i < MD5_DIGEST_LENGTH; i++) 
    	   sprintf(&url_hash[i*2], "%02x", digest[i]);

	char meta_path[512];
	snprintf(meta_path, sizeof(meta_path), "%s/meta/%s.json", engine->data_dir, url_hash);

	pthread_mutex_lock(&engine->file_lock);

	FILE* file = fopen(meta_path, "w");
	if (!file) {
		pthread_mutex_unlock(&engine->file_lock);
		fprintf(stderr, "Failed to open %s: %s\n", meta_path, strerror(errno));
	}

	fprintf(file, "{\n");
	fprintf(file, "  \"url\": \"%s\",\n", metarecord->url);
	fprintf(file, "  \"file_path\": \"%s\",\n", metarecord->file_path);
	fprintf(file, "  \"size\": %zu,\n", metarecord->raw_size);
	fprintf(file, "  \"timestamp\": %ld\n", metarecord->timestamp);
	fprintf(file, "}");


	fclose(file);
	pthread_mutex_unlock(&engine->file_lock);
}
