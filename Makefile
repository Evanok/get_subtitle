CFLAGS += -Werror -W -Wall -pedantic
LDFLAGS += -lcurl
HEADERS += -I./include
CC=c99

all: video_lima

video_lima: src/video_lima.o src/os_hash.o
	$(CC) -g -gg -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -g -o $@ -c $< $(CFLAGS) $(HEADERS)
clean:
	find . -name "*.o" -print0 | xargs -0 rm -f

mrproper: clean
	rm -f video_lima
	rm -f ./test/breakdance.avi
	rm -f output

./test/breakdance.avi:
	cd ./test && wget http://www.opensubtitles.org/addons/avi/breakdance.avi

test_login:
	curl -H "Content-Type: application/xml" -X POST --data-binary  @test/test_login.xml http://api.opensubtitles.org:80/xml-rpc

test: video_lima ./test/breakdance.avi
	./video_lima ./test/breakdance.avi

valgrind: video_lima ./test/breakdance.avi
	valgrind --leak-check=full ./video_lima ./test/breakdance.avi

dedi_test:
	./video_lima ../download/How.I.Met.Your.Mother.S09E22.HDTV.x264-KILLERS.mp4

.PHONY: test
