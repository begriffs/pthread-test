CFLAGS = -std=c99 -g -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wshadow
LDFLAGS = -pthread

all : alarms thread_error

alarms : alarm alarm_fork alarm_thread alarm_mutex
