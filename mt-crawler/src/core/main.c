#include "downloader.h"
#include "url_validator.h"
#include "config_loader.h"
#include "storage.h"
#include "metarecord.h"

int main() {
    // 初始化
    CrawlerConfig config;
    if(!load_config("config/setting.ini",&config))
    {
	fprintf(stderr, "Config error: %s\n", get_config_error());
        return 1;
    }

    if(downloader_init() != 0) {
        fprintf(stderr, "下载器初始化失败\n");
        return 1;
    }
    ini_validator_url();

    // 下载示例
    const char *url = "https://www.luogu.com.cn/problem/list?difficulty=3&tag=3,139,141,144,152,323,444,464&page=1";
    if(!is_url_visited(url)) {
        DownloadResult *res = download_url(url, &config);
        if(res) {
            printf("下载成功! 状态码: %d\n", res->status_code);
            mark_url_visited(url);
            //download_result_free(res);
        }
	StorageEngine engine;
	if(!init_storage_engine(&engine, &config))
	{
		printf("存储引擎初始化失败\n");
	}
	if(storage_save(&engine,url,res))
		printf("存储成功！\n");

    }

    // 清理
    url_validator_cleanup();
    curl_global_cleanup();
    return 0;
}
