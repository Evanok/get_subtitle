#include <video_lima.h>

static void usage(const char* name)
{
  fprintf(stderr, "\nusage: %s [options]... file\n\n", name);
  fprintf(stderr, "options:\n");
  fprintf(stderr, " -d : enable verbose mode\n");
  exit(1);
}

static void *myrealloc(void *ptr, size_t size)
{
  if(ptr)
    return realloc(ptr, size);
  else
    return malloc(size);
}

static size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;

  mem->memory = (char *)myrealloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory)
  {
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
  }
  return realsize;
}

static int curl_login_os (void)
{
  CURL *curl;
  CURLcode res;

  struct curl_slist *headerlist=NULL;
  static const char buf[] = "Content-Type: application/xml";
  const char* postmess =
    "<?xml version=\"1.0\"?>\n<methodCall>\n <methodName>LogIn</methodName>\n <params>\n  <param>\n   <value><string></string></value>\n  </param>\n  <param>\n   <value><string></string></value>\n  </param>\n  <param>\n   <value><string></string></value>\n  </param>\n  <param>\n   <value><string>OS Test User Agent</string></value>\n  </param>\n </params>\n</methodCall>";

  struct MemoryStruct chunk;
  chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
  chunk.size = 0;    /* no data at this point */
  log_debug ("%s\n", postmess);

  curl_global_init(CURL_GLOBAL_ALL);

  curl = curl_easy_init();

  if(curl)
  {
    headerlist = curl_slist_append(headerlist, buf);
    curl_easy_setopt(curl, CURLOPT_URL, OS_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postmess);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(postmess));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    res = curl_easy_perform(curl);

    if (chunk.memory)
      free(chunk.memory);

    if(res != CURLE_OK)
    {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      return 1;
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all (headerlist);
  }
  return 0;
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

  if (curl_login_os () != 0)
    return 2;

  clean_video (st_video);
  fclose(st_video.file);

  return 0;
}
