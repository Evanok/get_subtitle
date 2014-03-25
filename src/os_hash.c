#include "os_hash.h"

unsigned long long compute_hash(FILE* file)
{
  int i;
  unsigned long long t1 = 0;
  unsigned long long buffer1[8192 * 2];

  fread(buffer1, 8192, 8, file);
  fseek(file, -65536, SEEK_END);
  fread(&buffer1[8192], 8192, 8, file);
  for (i = 0; i < 8192 * 2; i++)
    t1 += buffer1[i];
  t1 += ftell(file);
  return t1;
}
