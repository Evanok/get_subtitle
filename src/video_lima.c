#include <video_lima.h>

static void usage(const char* name)
{
  fprintf(stderr, "\nusage: %s [options]... file\n\n", name);
  fprintf(stderr, "options:\n");
  fprintf(stderr, " -d : enable verbose mode\n");
  exit(1);
}

void clean_video (st_video_file file)
{
  if (file.complete_name)
    free (file.complete_name);
}

int main (int argc , char **argv)
{
  char path[255];
  struct stat st_stat;
  st_video_file st_video;

  if (argc != 2)
    usage (argv[0]);

  strcpy (path, argv[1]);

  if (stat(path, &st_stat) != 0 || (S_ISDIR(st_stat.st_mode)))
  {
    perror ("stat");
    fprintf(stderr, "path is : %s\n", path);
    return 2;
  }

  st_video.complete_name = malloc (strlen(path) + 1 * sizeof(char));
  strcpy (st_video.complete_name, path);
  strcpy (st_video.name, basename (path));

  st_video.file = fopen(st_video.complete_name, "rb");

  if (!st_video.file)
  {
    perror ("fopen");
    fprintf(stderr, "file path is : %s\n", st_video.complete_name);
    clean_video (st_video);
    return 2;
  }

  st_video.hash = compute_hash(st_video.file);
  printf("%llu\n", st_video.hash);

  clean_video (st_video);
  fclose(st_video.file);

  return 0;
}
