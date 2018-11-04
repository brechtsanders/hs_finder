#include "search_data_buffer.h"
#include <stdlib.h>
#include <string.h>

struct search_data_buffer_struct {
  char* data;
  size_t datalen;
  size_t dataalloclen;
  size_t diskpos;
};

struct search_data_buffer_struct* initialize_search_data_buffer ()
{
  struct search_data_buffer_struct* result;
  if ((result = (struct search_data_buffer_struct*)malloc(sizeof(struct search_data_buffer_struct))) != NULL) {
    result->data = NULL;
    result->datalen = 0;
    result->dataalloclen = 0;
    result->diskpos = 0;
  }
  return result;
};

void deinitialize_search_data_buffer (struct search_data_buffer_struct* searchdata)
{
  if (searchdata) {
    if (searchdata->data)
      free(searchdata->data);
    free(searchdata);
  }
};

void reset_search_data_buffer (struct search_data_buffer_struct* searchdata)
{
  if (searchdata->data)
    free(searchdata->data);
  searchdata->data = NULL;
  searchdata->datalen = 0;
  searchdata->dataalloclen = 0;
  searchdata->diskpos = 0;
}

void search_data_buffer_add (struct search_data_buffer_struct* searchdata, const char* data, size_t datalen)
{
  if (searchdata->datalen + datalen > searchdata->dataalloclen) {
    searchdata->dataalloclen = searchdata->datalen + datalen;
    searchdata->data = (char*)realloc(searchdata->data, searchdata->dataalloclen);
  }
  memcpy(searchdata->data + searchdata->datalen, data, datalen);
  searchdata->datalen += datalen;
}

size_t search_data_buffer_flush (struct search_data_buffer_struct* searchdata, size_t flushpos, FILE* dst)
{
  size_t result;
  if (flushpos <= searchdata->diskpos)
    return 0;
  if (flushpos > searchdata->diskpos + searchdata->datalen)
    flushpos = searchdata->diskpos + searchdata->datalen;
  if (dst)
    result = fwrite(searchdata->data, 1, flushpos - searchdata->diskpos, dst);
  else
    result = flushpos - searchdata->diskpos;
  memmove(searchdata->data, searchdata->data + flushpos - searchdata->diskpos, searchdata->datalen -= flushpos - searchdata->diskpos);
  searchdata->diskpos = flushpos;
  return result;
}

size_t search_data_buffer_flush_fn (struct search_data_buffer_struct* searchdata, size_t flushpos, search_data_buffer_output_fn flushfn, void* callbackdata)
{
  size_t result;
  if (flushpos <= searchdata->diskpos)
    return 0;
  if (flushpos > searchdata->diskpos + searchdata->datalen)
    flushpos = searchdata->diskpos + searchdata->datalen;
  if (flushfn)
    result = (*flushfn)(callbackdata, searchdata->data, flushpos - searchdata->diskpos);
  else
    result = flushpos - searchdata->diskpos;
  memmove(searchdata->data, searchdata->data + flushpos - searchdata->diskpos, searchdata->datalen -= flushpos - searchdata->diskpos);
  searchdata->diskpos = flushpos;
  return result;
}

size_t search_data_buffer_flush_remaining (struct search_data_buffer_struct* searchdata, FILE* dst)
{
  size_t result;
  if (dst)
    result = fwrite(searchdata->data, 1, searchdata->datalen, dst);
  else
    result = searchdata->datalen;
  searchdata->diskpos += searchdata->datalen;
  searchdata->datalen = 0;
  return result;
}

size_t search_data_buffer_flush_remaining_fn (struct search_data_buffer_struct* searchdata, search_data_buffer_output_fn flushfn, void* callbackdata)
{
  size_t result;
  if (flushfn)
    result = (*flushfn)(callbackdata, searchdata->data, searchdata->datalen);
  else
    result = searchdata->datalen;
  searchdata->diskpos += searchdata->datalen;
  searchdata->datalen = 0;
  return result;
}

size_t search_data_buffer_get_pos (struct search_data_buffer_struct* searchdata)
{
  return searchdata->diskpos;
}

size_t search_data_buffer_get_len (struct search_data_buffer_struct* searchdata)
{
  return searchdata->datalen;
}

const char* search_data_buffer_get_at_pos (struct search_data_buffer_struct* searchdata, size_t pos)
{
  if (pos < searchdata->diskpos || pos >= searchdata->diskpos + searchdata->datalen)
    return NULL;
  return searchdata->data + (pos - searchdata->diskpos);
}

