#include "hs_finder.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define READBUFFERSIZE 128

struct count_data_struct {
  size_t count;
  size_t* patterncounts;
};

static int when_found (unsigned int id, unsigned long long from, unsigned long long to, unsigned int flags, struct hs_finder* finder)
{
  struct count_data_struct* countdata = (struct count_data_struct*)hs_finder_get_callbackdata(finder);
  countdata->count++;
  countdata->patterncounts[id]++;
  return 0;
}

void show_help()
{
  printf(
    "Usage:  hs_finder_count [[-?|-h] -c] [-i] [-f file] [-t text] [-p <pattern>] <pattern> ...\n" \
    "Parameters:\n" \
    "  -? | -h     \tshow help\n" \
    "  -c          \tcase sensitive matching for next pattern(s) (default)\n" \
    "  -i          \tcase insensitive matching for next pattern(s)\n" \
    "  -f file     \tinput file (default is to use standard input)\n" \
    "  -t text     \tuse text as search data (overrides -f)\n" \
    "  -n          \tcreate new search instance\n" \
    "  -p pattern  \tpattern to search for (can be used if pattern starts with \"-\")\n" \
    "  pattern     \tpattern to search for\n" \
    "Version: " HS_FINDER_VERSION_STRING "\n" \
    "\n"
  );
}

int main (int argc, char** argv)
{
  struct hs_finder* finder;
  struct count_data_struct countdata;
  int flags = HS_FLAG_SOM_LEFTMOST | HS_FLAG_DOTALL;
  const char* srcfile = NULL;
  const char* srctext = NULL;
  size_t* patterncounts = NULL;
  size_t patterns = 0;
  //initialize
  if ((patterncounts = (size_t*)malloc((argc - 1) * sizeof(size_t))) == NULL) {
    fprintf(stderr, "Memory allocation error\n");
    return 2;
  }
  countdata.count = 0;
  countdata.patterncounts = patterncounts;
  if ((finder = hs_finder_initialize(when_found, &countdata)) == NULL) {
    fprintf(stderr, "Error in hs_finder_initialize()\n");
    return 3;
  }
  //process command line parameters
  {
    int i = 0;
    char* param;
    int paramerror = 0;
    while (!paramerror && ++i < argc) {
      if (argv[i][0] == '-') {
        param = NULL;
        switch (tolower(argv[i][1])) {
          case '?' :
          case 'h' :
            if (argv[i][2])
              paramerror++;
            else
              show_help();
            return 0;
          case 'c' :
            if (argv[i][2])
              paramerror++;
            else
              flags &= ~HS_FLAG_CASELESS;
            break;
          case 'i' :
            if (argv[i][2])
              paramerror++;
            else
              flags |= HS_FLAG_CASELESS;
            break;
          case 'f' :
            if (argv[i][2])
              param = argv[i] + 2;
            else if (i + 1 < argc && argv[i + 1])
              param = argv[++i];
            if (!param)
              paramerror++;
            else
              srcfile = param;
            break;
          case 't' :
            if (argv[i][2])
              param = argv[i] + 2;
            else if (i + 1 < argc && argv[i + 1])
              param = argv[++i];
            if (!param)
              paramerror++;
            else
              srctext = param;
            break;
          case 'n' :
            if (argv[i][2])
              paramerror++;
            else
              hs_finder_add_instance(finder, when_found, &countdata);
            break;
          case 'p' :
            if (argv[i][2])
              param = argv[i] + 2;
            else if (i + 1 < argc && argv[i + 1])
              param = argv[++i];
            if (!param)
              paramerror++;
            else {
              patterncounts[patterns] = 0;
              hs_finder_add_expr(finder, param, HS_FLAG_SOM_LEFTMOST | HS_FLAG_DOTALL | flags, patterns++);
            }
            break;
          default :
            paramerror++;
            break;
        }
      } else {
        patterncounts[patterns] = 0;
        hs_finder_add_expr(finder, param, HS_FLAG_SOM_LEFTMOST | HS_FLAG_DOTALL | flags, patterns++);
      }
    }
    if (paramerror || argc <= 1) {
      if (paramerror)
        fprintf(stderr, "Invalid command line parameters\n");
      show_help();
      return 1;
    }
  }
  //prepare finder for searching
  if (hs_finder_open(finder, hs_finder_output_to_null, NULL) != HS_SUCCESS) {
    fprintf(stderr, "Error in hs_finder_open()\n");
    hs_finder_cleanup(finder);
    return 4;
  }
  //process search data
  if (srctext) {
    //process supplied text
    if (hs_finder_process(finder, srctext, strlen(srctext)) != HS_SUCCESS) {
      fprintf(stderr, "Error in hs_finder_open()\n");
    }
  } else {
    //process file (or standard input)
    FILE* src;
    char buf[READBUFFERSIZE];
    size_t buflen;
    if (!srcfile) {
      src = stdin;
    } else {
      if ((src = fopen(srcfile, "rb")) == NULL) {
        fprintf(stderr, "Error opening file: %s\n", srcfile);
        hs_finder_cleanup(finder);
        return 5;
      }
    }
    while ((buflen = fread(buf, 1, READBUFFERSIZE, src)) > 0) {
      if (hs_finder_process(finder, buf, buflen) != HS_SUCCESS) {
        fprintf(stderr, "Error in hs_finder_open()\n");
      }
    }
    fclose(src);
  }
  hs_finder_close(finder);
  //show results
  printf("%lu matches found\n", (unsigned long)countdata.count);
  {
    size_t i;
    for (i = 0; i < patterns; i++)
      printf("pattern %lu found %lu times\n", (unsigned long)i + 1, (unsigned long)patterncounts[i]);
  }
  //clean up
  free(patterncounts);
  hs_finder_cleanup(finder);
  return 0;
}
