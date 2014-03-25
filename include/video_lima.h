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


typedef struct video_file
{
  char			name[255];
  char*			complete_name;
  unsigned int		size;
  FILE*			file;
  unsigned long long	hash;
} st_video_file;

void clean_video (st_video_file file);

# include "os_hash.h"

#endif /* !VIDEO_LIMA_H_ */

