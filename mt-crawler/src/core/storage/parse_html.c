#include "work.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <gumbo.h>
#include <stdatomic.h>
#include <uriparser/Uri.h>
#include "pthread_config.h"
#include "url_validator.h"
#include "downloader.h"
#include "config_loader.h"

static const char* find_base_href(GumboNode* node,int depth);
static void search_links(GumboNode* node, const char* base_url, Task* new_tasks, int* new_task_count, int depth);


void parse_html(DownloadResult* result, Task* new_tasks, int* new_task_count,int depth,const char* url)
{
    // 创建解析器选项
    GumboOptions options = kGumboDefaultOptions;
    GumboOutput* output = gumbo_parse_with_options(&options, result->data, strlen(result->data));

    if (!output || !output->root) {
        printf("Parse failed for URL: %s", url);
        return;
    }

    // 获取根节点
    GumboNode* root = output->root;

    const char* base;// = find_base_href(root, 0);

    //if (!base)
    base = url;  // 回退使用原始URL作为base
    search_links(root, base, new_tasks, new_task_count, depth);

    gumbo_destroy_output(&kGumboDefaultOptions, output);

    return;
}

static const char* find_base_href(GumboNode* node, int depth) {
    // 递归深度限制
    if (depth > 10 || node == NULL) {
        return NULL;
    }

    // 仅处理元素节点
    if (node->type == GUMBO_NODE_ELEMENT) {
        GumboElement* element = &node->v.element;
        if (element->tag == GUMBO_TAG_BASE) {
            GumboAttribute* href = gumbo_get_attribute(&element->attributes, "href");
            if (href != NULL) {
                return href->value;
            }
        }

        // 遍历子节点（仅对元素节点有效）
        GumboVector* children = &element->children;
        for (unsigned int i = 0; i < children->length; ++i) {
            GumboNode* child = (GumboNode*)children->data[i];
            if (child == NULL) continue;  // 跳过空子节点
            const char* result = find_base_href(child, depth + 1);
            if (result != NULL) {
                return result;
            }
        }
    }

    return NULL;
}



static char* resolve_path(const char* base, const char* rel) {
    if (!base || !rel)
    {
        printf("error in resolve_path");
        return NULL;
    }
    UriUriA base_uri, rel_uri;
    if (uriParseSingleUriA(&base_uri, base, NULL) != URI_SUCCESS) return NULL;
    if (uriParseSingleUriA(&rel_uri, rel, NULL) != URI_SUCCESS) {
        uriFreeUriMembersA(&base_uri);
        return NULL;
    }

    UriUriA resolved;
    if (uriAddBaseUriA(&resolved, &rel_uri, &base_uri) != URI_SUCCESS) {
        uriFreeUriMembersA(&base_uri);
        uriFreeUriMembersA(&rel_uri);
        return NULL;
    }

    // 转换为字符串
    int required;
    char* result;
    uriToStringCharsRequiredA(&resolved, &required);
    required++;
    result = malloc(required);
    if (!result) {
        uriFreeUriMembersA(&base_uri);
        uriFreeUriMembersA(&rel_uri);
        uriFreeUriMembersA(&resolved);
        return NULL;
    }
    if (!uriToStringA(result, &resolved, required, NULL)) {
        free(result);
        uriFreeUriMembersA(&base_uri);
        uriFreeUriMembersA(&rel_uri);
        uriFreeUriMembersA(&resolved);
        return NULL;
    }

    uriFreeUriMembersA(&base_uri);
    uriFreeUriMembersA(&rel_uri);
    uriFreeUriMembersA(&resolved);
    return result;
}

// 检查并压入URL任务队列
static void process_url(const char* url, const char* base_url, Task* new_tasks, int* new_task_count, int depth) {
    if (!url || url[0] == '#') return;  // 忽略空链接和锚点

    // URL标准化,静态化
    char* full_url = malloc(512);
    if (strstr(url, "http://") == url || strstr(url, "https://") == url) {
        full_url = strdup(url);
    }
    else {
        full_url = resolve_path(base_url, url);  // 使用uriparser解析
    }
    if (is_url_format_valid(full_url)  && *new_task_count<MAX_URLS) {
        new_tasks[*new_task_count].url = full_url;
        new_tasks[*new_task_count].depth = depth + 1;
        (*new_task_count)++;
        return;
    }
    else
        free(full_url);
    return;
}

// 递归搜索链接
static void search_links(GumboNode* node, const char* base_url, Task* new_tasks, int* new_task_count,int depth) {
    if (node->type != GUMBO_NODE_ELEMENT) return;
    if (!node || !base_url || !new_tasks || !new_task_count) return;
    // 处理不同标签的URL属性
    const char* attr_name = NULL;
    switch (node->v.element.tag) {
    case GUMBO_TAG_A:
    case GUMBO_TAG_LINK:
        attr_name = "href";
        break;
    case GUMBO_TAG_IMG:
    case GUMBO_TAG_SCRIPT:
    case GUMBO_TAG_IFRAME:
        attr_name = "src";
        break;
    default:
        break;
    }

    // 提取属性值
    if (attr_name) {
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, attr_name);
        if (attr) {
            process_url(attr->value, base_url,new_tasks,new_task_count,depth);
        }
    }

    // 递归子节点
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        search_links((GumboNode*)children->data[i], base_url, new_tasks, new_task_count, depth);
    }
}
