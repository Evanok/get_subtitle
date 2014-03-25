#include <video_lima.h>

static void usage(const char* name)
{
  fprintf(stderr, "usage: %s [options]... directory/file\n\n", name);
  fprintf(stderr, "options:\n");
  fprintf(stderr, " -d : enable verbose mode\n");
  exit(1);
}

int main (int argc , char **argv)
{
  char path[255];

  if (argc != 2)
    usage (argv[0]);

  strcpy (path, argv[1]);

  printf ("path to compute is %s\n", path);

  return 0;
}
