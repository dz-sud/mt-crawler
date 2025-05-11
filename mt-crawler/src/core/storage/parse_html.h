#ifndef PARSE_HTML_H
#define PARSE_HTML_H

#include "work.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "pthread_config.h"
#include "url_validator.h"
#include "downloader.h"
#include "config_loader.h"

void parse_html(DownloadResult* result, Task* new_tasks, int* new_task_count,int depth,const char* url);


#endif


