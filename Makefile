CFLAGS += -Werror -W -Wall -pedantic
HEADERS += -I./include
CC=c99

all: video_lima

video_lima: src/video_lima.o src/os_hash.o
	$(CC) -g -gg -o $@ $^

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS) $(HEADERS)
clean:
	find . -name "*.o" -print0 | xargs -0 rm -f

mrproper:
	rm -f video_lima
	rm -f ./test/breakdance.avi

./test/breakdance.avi:
	cd ./test && wget http://www.opensubtitles.org/addons/avi/breakdance.avi

valgrind: video_lima ./test/breakdance.avi
	valgrind ./video_lima ./test/breakdance.avi
