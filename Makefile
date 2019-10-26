CFLAGS = -std=c99 -g -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wshadow
LDFLAGS = -pthread

all : alarm alarm_fork alarm_thread
