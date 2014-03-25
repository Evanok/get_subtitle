CFLAGS += -Werror -W -Wall -ansi -pedantic
HEADERS += -I./include
CC=gcc

all: video_lima

video_lima: src/video_lima.o
	$(CC) -g -gg -o $@ $^

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS) $(HEADERS)
clean:
	find . -name "*.o" -print0 | xargs -0 rm -f
	rm -f video_lima
