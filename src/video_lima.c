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

static char* curl_login_os (void)
{
  CURL *curl;
  CURLcode res;

  struct curl_slist *headerlist=NULL;
  static const char buf[] = "Content-Type: application/xml";
  const char* postmess =
    "<?xml version=\"1.0\"?>\n<methodCall>\n <methodName>LogIn</methodName>\n <params>\n  <param>\n   <value><string></string></value>\n  </param>\n  <param>\n   <value><string></string></value>\n  </param>\n  <param>\n   <value><string></string></value>\n  </param>\n  <param>\n   <value><string>OS Test User Agent</string></value>\n  </param>\n </params>\n</methodCall>";
  struct MemoryStruct chunk;
  char* begin_ptr = NULL;
  char* end_ptr = NULL;
  char* token = NULL;

  chunk.memory = NULL; /* we expect realloc(NULL, size) to work */
  chunk.size = 0;      /* no data at this point */

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

    if(res != CURLE_OK)
    {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      if (chunk.memory)
	free(chunk.memory);
    }
    else
    {

      begin_ptr = strstr (chunk.memory, "token");

      if (begin_ptr == NULL)
      {
	fprintf(stderr, "Unable to find xml token\n");
	fprintf(stderr, "\n%s\n", chunk.memory);
      }

      begin_ptr = strstr (chunk.memory, "<string>");
      end_ptr = strstr (chunk.memory, "</string>");
      if (begin_ptr && end_ptr)
      {
	token = malloc (sizeof(char) * (end_ptr - begin_ptr - 8 + 1));
	strncpy (token, begin_ptr + 8, end_ptr - begin_ptr - 8);
	token[end_ptr - begin_ptr - 8] = 0;
	printf ("TOKEN %s\n", token);
      }
    }

    if (chunk.memory)
      free(chunk.memory);

    curl_easy_cleanup(curl);
    curl_slist_free_all (headerlist);
  }

  return token;
}

int main (int argc , char **argv)
{
  FILE* file;
  struct stat st_stat;
  unsigned long long	hash;
  char* token = NULL;

  if (argc != 2)
    usage (argv[0]);

  if (stat(argv[1], &st_stat) != 0 || (S_ISDIR(st_stat.st_mode)))
  {
    perror ("stat");
    fprintf(stderr, "path is : %s\n", argv[1]);
    return 2;
  }

  file = fopen(argv[1], "rb");

  if (!file)
  {
    perror ("fopen");
    fprintf(stderr, "file path is : %s\n", argv[1]);
    return 2;
  }

  hash = compute_hash(file);
  printf("%llu\n", hash);

  token = curl_login_os ();
  if (token == NULL)
    return 2;

  printf ("TOKEN %s\n", token);

  fclose(file);
  free (token);

  return 0;
}
