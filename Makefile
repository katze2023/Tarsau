CC = gcc
# Hata ayıklama ve optimize etme bayraklarını modüler hale getirdik
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
DEBUG_FLAGS = -g -O0
RELEASE_FLAGS = -O3

TARGET = tarsau
SRCS = sau_main.c sau_utils.c sau_archive.c sau_extract.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean debug release

all: $(TARGET)

# Release modu: derleme sonrası optimize edilmiş sürüm
release: CFLAGS += $(RELEASE_FLAGS)
release: $(TARGET)

# Debug modu: hata ayıklama sembolleriyle derler
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c sau_common.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *.sau