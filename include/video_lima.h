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

# include "os_hash.h"

struct MemoryStruct
{
  char *memory;
  size_t size;
};

#endif /* !VIDEO_LIMA_H_ */

