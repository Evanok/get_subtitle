/**
 **

 ** \file video_lima.h
 ** \brief define video lima header
 ** \author arthur lambert
 ** \date 25/03/14
 **
 **/

#ifndef VIDEO_LIMA_H_
# define VIDEO_LIMA_H_

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <sys/stat.h>
# include <libgen.h>

# include <curl/curl.h>

#define OS_URL "http://api.opensubtitles.org:80/xml-rpc"
#define RESULT_OS "output"
#define MAX_SIZE 8589934592

# include "os_hash.h"

char* curl_perform_os (const char* postmess);
int logout_os (char* token);
char* login_os (void);
char* check_hash_os (char* token, unsigned long long hash);
char* get_subtitile_os (char* token, unsigned long long hash, unsigned long long size);
char* get_imdb (char* data);

struct MemoryStruct
{
  char *memory;
  size_t size;
};
#define DEBUG 1
#define DEBUG_PRINT(...) \
            do { if (DEBUG) fprintf(stderr, __VA_ARGS__); } while (0)

#endif /* !VIDEO_LIMA_H_ */

