CC ?= cc
CFLAGS = -Wall -Wextra  -Iinclude -std=c99 -O2 -pthread

SRC_DIR = src
OBJ_DIR = obj
TARGET = dns_proxy_filter_p2B9agE1
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

distclean: clean
	rm -f $(TARGET)