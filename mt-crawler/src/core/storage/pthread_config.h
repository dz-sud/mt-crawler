#ifndef PTHREAD_CONFIG_H
#define PTHREAD_CONFIG_H

#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "config_loader.h"

#define MAX_URLS 1000
#define MAX_THREADS 100

typedef struct {
    char* url;
    int depth; //该url深度
} Task;

typedef struct {
    Task tasks[MAX_URLS];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t cond_not_empty;
    pthread_cond_t cond_not_full;
} TaskQueue;

typedef struct {
    pthread_t threads[MAX_THREADS];
    TaskQueue task_queue;
    bool shutdown;
    CrawlerConfig* config;
} ThreadPool;

void init_thread_pool(ThreadPool* pool, CrawlerConfig* config);
void push_task(TaskQueue* task_queue,Task task);
Task pop_task(TaskQueue* task_queue);
	
#endif
