/**
**

** \file os_hash.h
** \brief define os_hash header
** \author arthur lambert
** \date 25/03/14
**
**/

#ifndef OS_HASH_H_
# define OS_HASH_H_

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>

unsigned long long compute_hash(FILE* file);

#endif /* !OS_HASH_H_ */
