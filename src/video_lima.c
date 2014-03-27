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

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postmess);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(postmess));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    DEBUG_PRINT ("\n\n[DEBUG] request : \n%s\n\n", postmess);

    res = curl_easy_perform(curl);

    if(res != CURLE_OK)
    {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      if (chunk.memory)
	free(chunk.memory);
    }

    if (chunk.memory)
    {
      DEBUG_PRINT ("\n\n[DEBUG] answer : \n%s\n\n", chunk.memory);

      begin_ptr = strstr (chunk.memory, "status");

      if (begin_ptr == NULL)
      {
	begin_ptr = strstr (chunk.memory, "head");

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
	DEBUG_PRINT ("[DEBUG] STATUS '%s'\n", status);

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

char* process_data (char* data)
{
  FILE* file = NULL;
  char* begin_ptr = NULL;
  char* end_ptr = NULL;
  char* imdb = NULL;
  char value[256];
  int find = 0;

  file = fopen (RESULT_OS, "w+");
  if (file == NULL)
  {
    perror ("fopen");
    fprintf(stderr, "Path is %s\n", RESULT_OS);
    free (data);
    return NULL;
  }

  begin_ptr = strstr (data, "MovieHash");

  if (begin_ptr == NULL)
  {
    fclose (file);
    free (data);
    return NULL;
  }

  end_ptr = strstr (begin_ptr, "</string>");
  begin_ptr = strstr (begin_ptr, "<string>");

  if (begin_ptr && end_ptr)
  {
    strncpy (value, begin_ptr + 8, end_ptr - begin_ptr - 8);
    value[end_ptr - begin_ptr - 8] = 0;
    DEBUG_PRINT ("[DEBUG] MovieHash %s\n", value);

    fwrite ("MovieHash : ", 12, 1, file);
    fwrite (value, strlen(value), 1, file);
    fwrite ("\n", 1, 1, file);
  }

  while (value[0])
  {
    memset (value, 0, 256);
    begin_ptr = strstr (end_ptr, "<name>");
    end_ptr = strstr (begin_ptr, "</name>");

    if (begin_ptr && end_ptr)
    {
      strncpy (value, begin_ptr + 6, end_ptr - begin_ptr - 6);
      value[end_ptr - begin_ptr - 6] = 0;
      DEBUG_PRINT ("[DEBUG] Name %s\n", value);

      if (strcmp(value, "not_processed") == 0)
	break;

      if (strcmp(value, "MovieImdbID") == 0)
	find = 1;

      fwrite (value, strlen(value), 1, file);
      fwrite (" : ", 3, 1, file);

      memset (value, 0, 256);
      begin_ptr = strstr (end_ptr, "<string>");
      if (begin_ptr)
	end_ptr = strstr (begin_ptr, "</string>");

      if (begin_ptr && end_ptr)
      {
	strncpy (value, begin_ptr + 8, end_ptr - begin_ptr - 8);
	value[end_ptr - begin_ptr - 8] = 0;
	DEBUG_PRINT ("[DEBUG] String %s\n", value);

	if (find)
	{
	  imdb = malloc (sizeof(char) * (strlen(value) + 1));
	  strcpy (imdb, value);
	  find = 0;
	}

	fwrite (value, strlen(value), 1, file);
	fwrite ("\n", 1, 1, file);
      }
    }
  }

  fclose (file);
  free (data);
  return imdb;
}


int logout_os (char* token)
{
  char* begin_postmess =
    "<?xml version=\"1.0\"?>\n<methodCall>\n <methodName>LogOut</methodName>\n <params>\n  <param>\n   <value><string>";

  char* end_postmess =
    "</string></value>\n  </param>\n </params>\n</methodCall>";

  char* result = NULL;
  size_t len_mess = strlen(end_postmess) + strlen(begin_postmess) + strlen(token) + 1;

  char* finalmess = malloc (sizeof(char) * len_mess + 1);
  sprintf (finalmess, "%s%s%s", begin_postmess, token, end_postmess);
  finalmess[len_mess] = 0;

  result = curl_perform_os (finalmess);
  free (finalmess);

  if (result)
  {
    DEBUG_PRINT ("[DEBUG] LOGOUT SUCCESS\n");
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
      DEBUG_PRINT ("[DEBUG] TOKEN %s\n", token);
    }
  }

  if (result)
    free(result);

  return token;
}

char* check_hash_os (char* token, unsigned long long hash)
{
  char str_hash[32];

  char* part1 =
    "<?xml version=\"1.0\"?>\n<methodCall>\n <methodName>CheckMovieHash</methodName>\n <params>\n  <param>\n   <value><string>";
  char* part2 =
    "</string></value>\n  </param>\n <param>\n   <value><array><data><value><string>";
  char* part3 =
    "</string></value></data></array></value>\n  </param>\n </params>\n</methodCall>";

  char* result = NULL;
  char* finalmess = NULL;
  char* tmp_ptr = NULL;

  size_t len;

  /* MUST ADD SANITY CHECK ! */
  sprintf (str_hash, "%llx", hash);
  len = strlen(part1) + strlen(part2) + strlen(part3) + strlen(str_hash) + strlen(token) + 1;

  finalmess = malloc (sizeof(char) * (len + 1));
  sprintf (finalmess, "%s%s%s%s%s", part1, token, part2, str_hash, part3);
  finalmess[len] = 0;

  result = curl_perform_os (finalmess);
  free (finalmess);

  if (result)
  {
    tmp_ptr = strstr (result, "MovieImdbID");

    if (tmp_ptr)
    {
      DEBUG_PRINT ("[DEBUG] MOVIE FOUND !\n");
      return process_data (result);
    }
    else
    {
      fprintf(stderr, "Movie does not exist in OpenSubtitile DataBase \n");
    }
    free (result);
    return NULL;
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
  DEBUG_PRINT("[DEBUG] hash : '%llx'\n", hash);
  fclose(file);

  token = login_os ();
  if (token == NULL)
    return 1;

  imdb = check_hash_os(token, hash);

  if (imdb)
  {
    DEBUG_PRINT("[DEBUG] imdb : '%s'\n", imdb);
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
