CC = gcc
CFLAGS = -Wall -Wextra $(shell pkg-config --cflags gtk+-3.0) -I./include
LDFLAGS = $(shell pkg-config --libs gtk+-3.0) -lsqlite3 -lcurl -ljson-c

# Add curl to CFLAGS and LDFLAGS
CFLAGS += $(shell pkg-config --cflags libcurl)
LDFLAGS += $(shell pkg-config --libs libcurl)

SRCS = src/main.c src/database.c src/gui.c src/gemini_api.c src/config.c
OBJS = $(SRCS:.c=.o)
TARGET = medical_stock_manager

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)