/**
 **

 ** \file get_subtitle.h
 ** \brief define get subtitle header
 ** \author arthur lambert
 ** \date 25/03/14
 **
 **/

#ifndef GET_SUBTITLE_H_
# define GET_SUBTITLE_H_

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
#define TIMEOUT_SECONDS 5

# include "os_hash.h"

char* curl_perform_os (const char* postmess);
int logout_os (char* token);
char* login_os (void);
char* check_hash_os (char* token, unsigned long long hash);
int get_subtitle_os (char* token, unsigned long long hash, unsigned long long size);
int get_subtitle_info (char* data);
char* get_imdb (char* data);

struct MemoryStruct
{
  char *memory;
  size_t size;
};
/* change value here to enable/disable debug */

#ifndef DEBUG
#define DEBUG 0
#endif

#define DEBUG_PRINT(...) \
            do { if (DEBUG) fprintf(stderr, __VA_ARGS__); } while (0)

#endif /* !GET_SUBTITLE_H_ */

