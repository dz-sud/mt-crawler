#ifndef	METARECORD_H
#define METARECORD_H

#include"storage.h"
#include "downloader.h"

void fill_meta_record(MetaRecord* metarecord, const char* url, const char * raw_path,const int size);
void save_meta_record(StorageEngine* engine, const MetaRecord* metarecord);

#endif
