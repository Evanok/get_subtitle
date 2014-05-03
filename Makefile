CFLAGS += -Werror -W -Wall -pedantic
LDFLAGS += -lcurl
HEADERS += -I./include
CC=c99

all: get_subtitle

get_subtitle: src/get_subtitle.o src/os_hash.o
	$(CC) -g -gg -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -g -o $@ -c $< $(CFLAGS) $(HEADERS)
clean:
	find . -name "*.o" -print0 | xargs -0 rm -f

mrproper: clean
	rm -f get_subtitle
	rm -f ./test/breakdance.avi
	rm -f output

./test/breakdance.avi:
	cd ./test && wget http://www.opensubtitles.org/addons/avi/breakdance.avi

test_login:
	curl -H "Content-Type: application/xml" -X POST --data-binary  @test/test_login.xml http://api.opensubtitles.org:80/xml-rpc

test: get_subtitle ./test/breakdance.avi
	./get_subtitle ./test/breakdance.avi

valgrind: get_subtitle ./test/breakdance.avi
	valgrind --leak-check=full ./get_subtitle ./test/breakdance.avi

dedi_test:
	./get_subtitle ../download/How.I.Met.Your.Mother.S09E22.HDTV.x264-KILLERS.mp4

.PHONY: test
