# 基本配置段
CC = gcc
CFLAGS = -Wall -I./include -I./src/core/downloader -I./src/core/storage -I./src/core/validator -I./src/utils/logger -I./src/utils/file_io -I./src/core -I./third_party
LDFLAGS = -lcurl -lpthread -lssl -lcrypto
TARGET = crawler

# 源文件定义
SRC_DIR = src
SRCS = $(wildcard $(SRC_DIR)/*.c) \
       $(wildcard $(SRC_DIR)/core/*.c) \
       $(wildcard $(SRC_DIR)/core/downloader/*.c) \
       $(wildcard $(SRC_DIR)/core/storage/*.c) \
       $(wildcard $(SRC_DIR)/core/validator/*.c) \
       $(wildcard $(SRC_DIR)/utils/*.c)\
       $(wildcard $(SRC_DIR)/utils/logger/*.c) \
       $(wildcard $(SRC_DIR)/utils/file_io/*.c) 
OBJS = $(SRCS:.c=.o)
OBJS += third_party/MurmurHash3.o

# 构建规则
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
# 添加第三方库编译
third_party/%.o: third_party/%.c
	$(CC) $(CFLAGS) -c -o $@ $<



# 伪目标
.PHONY: clean run

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)
