#ifndef WORK_H
#define WORK_H

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

void* worker(void* arg);

#endif
