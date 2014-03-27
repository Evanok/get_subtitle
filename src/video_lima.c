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

char* curl_perform_os (const char* postmess)
{
  CURL *curl;
  CURLcode res;

  char* begin_ptr = NULL;
  char* end_ptr = NULL;
  char* status = NULL;

  struct curl_slist *headerlist=NULL;
  static const char buf[] = "Content-Type: application/xml";
  struct MemoryStruct chunk;

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

    printf ("\n\n[DEBUG] request : \n%s\n\n", postmess);

    res = curl_easy_perform(curl);

    if(res != CURLE_OK)
    {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      if (chunk.memory)
	free(chunk.memory);
    }

    if (chunk.memory)
    {
      printf ("\n\n[DEBUG] answer : \n%s\n\n", chunk.memory);

      begin_ptr = strstr (chunk.memory, "status");

      if (begin_ptr == NULL)
      {
	begin_ptr = strstr (chunk.memory, "title");

	if (begin_ptr == NULL)
	{
	  fprintf(stderr, "[ERROR] Unable to find status or title tag in answer from os api\n");
	  fprintf(stderr, "\n%s\n", chunk.memory);
	  free(chunk.memory);
	  curl_easy_cleanup(curl);
	  curl_slist_free_all (headerlist);
	  return NULL;
	}

	end_ptr = strstr (begin_ptr, "</title>");
	begin_ptr = strstr (begin_ptr, "<title>");

	if (begin_ptr && end_ptr)
	{
	  status = malloc (sizeof(char) * (end_ptr - begin_ptr - 7 + 1));
	  strncpy (status, begin_ptr + 7, end_ptr - begin_ptr - 7);
	  status[end_ptr - begin_ptr - 7] = 0;
	  fprintf(stderr, "[ERROR] %s\n", status);
	  free (status);
	}
	free(chunk.memory);
	curl_easy_cleanup(curl);
	curl_slist_free_all (headerlist);
	return NULL;
      }

      end_ptr = strstr (begin_ptr, "</string>");
      begin_ptr = strstr (begin_ptr, "<string>");

      if (begin_ptr && end_ptr)
      {
	status = malloc (sizeof(char) * (end_ptr - begin_ptr - 8 + 1));
	strncpy (status, begin_ptr + 8, end_ptr - begin_ptr - 8);
	status[end_ptr - begin_ptr - 8] = 0;
	printf ("[DEBUG] STATUS '%s'\n", status);

	if (strcmp (status, "200 OK") != 0)
	{
	  fprintf(stderr, "[ERROR] Http status : '%s'\n", status);
	  free(chunk.memory);
	  chunk.memory = NULL;
	}
	free (status);
      }
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all (headerlist);
  }

  return chunk.memory;
}


int logout_os (char* token)
{
  char* postmess =
    "<?xml version=\"1.0\"?>\n<methodCall>\n <methodName>LogOut</methodName>\n <params>\n  <param>\n   <value><string>XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX</string></value>\n  </param>\n </params>\n</methodCall>";

  char* result;
  char* tmp_ptr = NULL;

  char* finalmess = malloc (sizeof(char) * (strlen(postmess) + 1));
  strncpy (finalmess, postmess, strlen(postmess));
  finalmess[strlen(postmess)] = 0;

  tmp_ptr = strstr (finalmess, "X");

  if (tmp_ptr)
  {
    memset (tmp_ptr, ' ', 32);
    strncpy (tmp_ptr, token, strlen(token));
  }
  result = curl_perform_os (finalmess);
  free (finalmess);

  if (result)
  {
    printf ("LOGOUT SUCESS\n");
    free (result);
    return 0;
  }

  /* error */
  return 1;
}

char* login_os (void)
{
  const char* postmess =
    "<?xml version=\"1.0\"?>\n<methodCall>\n <methodName>LogIn</methodName>\n <params>\n  <param>\n   <value><string></string></value>\n  </param>\n  <param>\n   <value><string></string></value>\n  </param>\n  <param>\n   <value><string></string></value>\n  </param>\n  <param>\n   <value><string>OS Test User Agent</string></value>\n  </param>\n </params>\n</methodCall>";
  char* result;
  char* begin_ptr = NULL;
  char* end_ptr = NULL;
  char* token = NULL;

  result = curl_perform_os (postmess);

  if (result)
  {

    begin_ptr = strstr (result, "token");

    if (begin_ptr == NULL)
    {
      fprintf(stderr, "Unable to find xml token in asnwer from os api\n");
      fprintf(stderr, "\n%s\n", result);
    }

    begin_ptr = strstr (result, "<string>");
    end_ptr = strstr (result, "</string>");
    if (begin_ptr && end_ptr)
    {
      token = malloc (sizeof(char) * (end_ptr - begin_ptr - 8 + 1));
      strncpy (token, begin_ptr + 8, end_ptr - begin_ptr - 8);
      token[end_ptr - begin_ptr - 8] = 0;
      printf ("[DEBUG] TOKEN %s\n", token);
    }
  }

  if (result)
    free(result);

  return token;
}

char* check_hash_os (char* token, unsigned long long hash)
{
  char str_hash[32];
/*
  char* postmess =
  "<?xml version=\"1.0\"?>\n<methodCall>\n <methodName>CheckMovieHash</methodName>\n <params>\n  <param>\n   <value><string>XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX</string></value>\n  </param>\n <param>\n   <value><array><data><value><string>ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ</string></value></data></array></value>\n  </param>\n </params>\n</methodCall>";
*/
  char* postmess =
    "<?xml version=\"1.0\"?>\n<methodCall>\n <methodName>CheckMovieHash</methodName>\n <params>\n  <param>\n   <value><string>XXXXXXXXXXXXXXXXXXXXXXXXXX</string></value>\n  </param>\n <param>\n   <value><array><data><value><string>ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ</string></value></data></array></value>\n  </param>\n </params>\n</methodCall>";

  char* result;
  char* tmp_ptr = NULL;

  char* finalmess = malloc (sizeof(char) * (strlen(postmess) + 1));
  strncpy (finalmess, postmess, strlen(postmess));
  finalmess[strlen(postmess)] = 0;

  tmp_ptr = strstr (finalmess, "X");

  if (tmp_ptr)
  {
    //memset (tmp_ptr, '\n', 32);
    strncpy (tmp_ptr, token, strlen(token));
  }

  tmp_ptr = strstr (finalmess, "Z");

  if (tmp_ptr)
  {
    memset (tmp_ptr, ' ', 32);
    sprintf (str_hash, "%llu", hash);
    //strncpy (tmp_ptr, str_hash, strlen(str_hash));
    strncpy (tmp_ptr, "dab462412773581c", strlen("dab462412773581c"));
  }


  result = curl_perform_os (finalmess);
  free (finalmess);

  if (result)
  {
    printf ("MOVIE FOUND !\n");
    free (result);
    return 0;
  }

  /* error */
  return NULL;
}

int main (int argc , char **argv)
{
  FILE* file;
  struct stat st_stat;
  unsigned long long	hash = 0;
  char* token = NULL;
  char* imdb = NULL;
  int error = 0;

  if (argc != 2)
    usage (argv[0]);

  if (stat(argv[1], &st_stat) != 0)
  {
    perror ("stat");
    fprintf(stderr, "path is : %s\n", argv[1]);
    return 1;
  }

  if (S_ISDIR(st_stat.st_mode))
  {
    fprintf(stderr, "This is a directory !\n");
    fprintf(stderr, "path is : %s\n", argv[1]);
    return 1;
  }

  file = fopen(argv[1], "rb");

  if (!file)
  {
    perror ("fopen");
    fprintf(stderr, "file path is : %s\n", argv[1]);
    return 1;
  }

  hash = compute_hash(file);
  fclose(file);
  printf("[DEBUG] hash : '%llu'\n", hash);

  token = login_os ();
  if (token == NULL)
    return 1;

  imdb = check_hash_os(token, hash);

  if (imdb)
  {
    printf("[DEBUG] imdb : '%s'\n", imdb);
    free (imdb);
  }

  if (logout_os(token))
  {
    free (token);
    return 1;
  }
  free (token);

  return error;
}
