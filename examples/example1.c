#include "hs_finder.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int when_found (unsigned int id, unsigned long long from, unsigned long long to, unsigned int flags, struct hs_finder* finder)
{
  if (hs_finder_get_pos(finder) > from) {
    //multiple match found
    printf("Secondary match %lu from position %lu to position %lu\n", (unsigned long)id, (unsigned long)from, (unsigned long)to);
  } else {
    char c = (char)id;
    hs_finder_flush(finder, from);
    hs_finder_output(finder, &c, 1);
    hs_finder_flush(finder, to);
    hs_finder_output(finder, &c, 1);
  }
  return 0;
}

#define DATA \
  "CFLAGS=-I/C/Temp/include\n" \
  "LDFLAGS=-L/C/Temp -l/C/Temp/libtmp.a -l/C/Temp/libtmp.dll.a\n" \
  "WORKDIR=/C/Temp\n" \
  "EOF\n"
#define DATALEN strlen(DATA)
#define BLOCK_SIZE 8

int main (int argc, char *argv[])
{
  hs_error_t status;
  size_t pos;
  struct hs_finder* finder;
  finder = hs_finder_initialize(when_found, NULL);
  hs_finder_add_expr(finder, "-L/C/Temp", HS_FLAG_SOM_LEFTMOST | HS_FLAG_DOTALL, (int)'^');
  hs_finder_add_expr(finder, "-l/C/Temp/lib[^ ]*\\.a", HS_FLAG_SOM_LEFTMOST | HS_FLAG_DOTALL, (int)'~');
  hs_finder_add_instance(finder, when_found, NULL);
  hs_finder_add_expr(finder, "/C/Temp", HS_FLAG_SOM_LEFTMOST | HS_FLAG_DOTALL | HS_FLAG_CASELESS, (int)'*');
  hs_finder_open(finder, hs_finder_output_to_stream, stdout);
  pos = 0;
  while (pos < DATALEN) {
    size_t len = (pos + BLOCK_SIZE <= DATALEN ? BLOCK_SIZE : DATALEN - pos);
    if ((status = hs_finder_process(finder, DATA + pos, len)) != HS_SUCCESS) {
      fprintf(stderr, "ERROR %i in hs_scan_stream()\n", (int)status);
      break;
    }
    pos += BLOCK_SIZE;
  }
  hs_finder_close(finder);
/**/
  hs_finder_add_expr(finder, "DIR", HS_FLAG_SOM_LEFTMOST | HS_FLAG_DOTALL, (int)'.');
  hs_finder_open(finder, hs_finder_output_to_stream, stdout);
  pos = 0;
  while (pos < DATALEN) {
    size_t len = (pos + BLOCK_SIZE <= DATALEN ? BLOCK_SIZE : DATALEN - pos);
    if ((status = hs_finder_process(finder, DATA + pos, len)) != HS_SUCCESS) {
      fprintf(stderr, "ERROR %i in hs_scan_stream()\n", (int)status);
      break;
    }
    pos += BLOCK_SIZE;
  }
  hs_finder_close(finder);
/**/
  hs_finder_cleanup(finder);
  return 0;
}
