TARGET = server 
YXSH_SRC_DIR = src
YXSH_INCLUDE_DIR = include
CFLAG = -Wall -g -O3 -I$(YXSH_INCLUDE_DIR)
LDFLAGS = -lrt
OBJ_DIR = object
CC = gcc

ALL_SRCS = $(wildcard $(YXSH_SRC_DIR)/*.c)
SRCS = $(filter-out %who.c %tell.c %yell.c %name.c, $(ALL_SRCS))
OBJS = $(patsubst $(YXSH_SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

.PHONY: all clean
all: perp_bin $(TARGET) bin/who bin/tell bin/yell bin/name

perp_bin:
	@mkdir -p bin
clean:
	rm -rf $(OBJ_DIR)/* $(TARGET) bin/who bin/tell bin/yell bin/name

bin/who: src/who.c
	$(CC) -O3 -Wall src/who.c -o bin/who $(LDFLAGS)
bin/tell: src/tell.c
	$(CC) -O3 -Wall src/tell.c -o bin/tell $(LDFLAGS)
bin/yell: src/yell.c
	$(CC) -O3 -Wall src/yell.c -o bin/yell $(LDFLAGS)
bin/name: src/name.c
	$(CC) -O3 -Wall src/name.c -o bin/name $(LDFLAGS)

$(TARGET):$(OBJS)
	$(CC) $(CFLAG) $^ -o $@ $(LDFLAGS)
$(OBJ_DIR)/%.o:$(YXSH_SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAG) -c $< -o $@
