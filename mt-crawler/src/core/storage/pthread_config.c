#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "pthread_config.h"
#include "url_validator.h"
#include "work.h"
#include "config_loader.h"

void push_task(TaskQueue* queue,Task task);
static bool init_Task_Queue(TaskQueue* queue);
void init_thread_pool(ThreadPool* pool,CrawlerConfig* config);
static bool is_task_queue_empty(TaskQueue* queue);
static bool is_task_queue_full(TaskQueue* queue) ;
Task pop_task(TaskQueue* queue) ;

static bool init_Task_Queue(TaskQueue* queue)
{
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->cond_not_empty, NULL);
    pthread_cond_init(&queue->cond_not_full, NULL);
    return true;
}

//
void init_thread_pool(ThreadPool* pool,CrawlerConfig* config)
{
    init_Task_Queue(&pool->task_queue);
    pool->shutdown = false;
    Task task;
    memset(&task, 0, sizeof(Task)); // 使用 memset 初始化
    task.depth = 0;
    //strncpy(task.url, config->url.seed_url, sizeof(config->url.seed_url) - 1);
   // task.url[sizeof(config->url.seed_url) - 1] = '\0';  // 确保终止
    task.url=config->url.seed_url;
    push_task(&pool->task_queue, task);
    pool->config = config;
    //printf("url = %s , pool url = %s",task.url,pool->task_queue.tasks[0].url);
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_create(&pool->threads[i], NULL, worker, (void*)pool); 
    }
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(pool->threads[i], NULL); 
        printf("thread%dhave finished! \n",i);
    }
}



static bool is_task_queue_empty(TaskQueue* queue) {
    return queue->count == 0;
}

static bool is_task_queue_full(TaskQueue* queue) {
    return queue->count == MAX_URLS;
}

void push_task(TaskQueue* queue,Task task)
{
    pthread_mutex_lock(&queue->lock);
    while (is_task_queue_full(queue)) {
        pthread_cond_wait(&queue->cond_not_full, &queue->lock);
    }
    queue->tasks[queue->tail] = task;

    queue->tail = (queue->tail + 1) % MAX_URLS;
    queue->count++;

    pthread_cond_signal(&queue->cond_not_empty);
    pthread_mutex_unlock(&queue->lock);
}

Task pop_task(TaskQueue* queue) {
    Task task;
    pthread_mutex_lock(&queue->lock);
    while (is_task_queue_empty(queue)) {
        pthread_cond_wait(&queue->cond_not_empty, &queue->lock);
    }
    task = queue->tasks[queue->head];
    queue->head = (queue->head + 1) % MAX_URLS;
    queue->count--;
    pthread_cond_signal(&queue->cond_not_full);
    pthread_mutex_unlock(&queue->lock);
    return task;
}
