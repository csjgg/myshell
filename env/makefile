CC = gcc
CFLAGS = -Wall 

# 获取所有的.c文件列表
SRCS = $(wildcard *.c)

# 通过替换后缀名来生成所有的可执行文件列表
PROGS = $(patsubst %.c,%,$(SRCS))

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(PROGS)