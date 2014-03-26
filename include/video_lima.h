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

typedef struct video_file
{
  char			name[255];
  char*			complete_name;
  unsigned int		size;
  FILE*			file;
  unsigned long long	hash;
  unsigned int		token;
} st_video_file;

void clean_video (st_video_file file);

# include "os_hash.h"

#define DEBUG_ENABLED
#ifdef DEBUG_ENABLED
#define log_debug(fmtstr, ...)					\
  {									\
    printf("[VIDEO_LIMA DEBUG]:%s:%d: " fmtstr "\n", __func__,__LINE__, __VA_ARGS__); \
  }
#else
#define log_debug(...)
#endif

struct MemoryStruct
{
  char *memory;
  size_t size;
};

#endif /* !VIDEO_LIMA_H_ */

