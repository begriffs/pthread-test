CFLAGS = -std=c99 -g -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wshadow
LDFLAGS = -pthread

.SUFFIXES :
.SUFFIXES : .o .c

all : alarm
