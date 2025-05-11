#include "work.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "pthread_config.h"
#include "url_validator.h"
#include "downloader.h"
#include "config_loader.h"
#include "parse_html.h"
#include "storage.h"

void* worker(void* arg)
{
    ThreadPool* pool = (ThreadPool*)arg;
    while (1) {
        Task task = pop_task(&pool->task_queue);
        char* url = task.url;

        if (!is_url_visited(url) && is_url_format_valid(url)) {
	        mark_url_visited(url);

            DownloadResult* result;
            result = download_url(url, pool->config);

            if (!result || !result->data || strlen(result->data) == 0) {
		    printf("Invalid HTML input in work.c");
                //log_error("Invalid HTML input");
                return NULL;
            }

            StorageEngine engine;
	        if(!init_storage_engine(&engine, pool->config))
	        {
		        printf("存储引擎初始化失败\n");
	        }
	        if(storage_save(&engine,url,result))
		    printf("存储成功！\n");

            Task new_tasks[MAX_URLS];
            int new_task_count = 0;
            parse_html(result, new_tasks, &new_task_count,task.depth,url); // 假设 parse_html 是一个函数，用于解析 HTML 并提取新的 URL
            
            
            for (int i = 0; i < new_task_count; i++) {
                if (task.depth <= pool->config->url.max_depth)
                    push_task(&pool->task_queue, new_tasks[i]);
            }
            
            //download_result_free(result);
            // 清理
            curl_global_cleanup();
            return NULL;
        }

    }
}

