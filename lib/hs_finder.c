#include "hs_finder.h"
#include "search_data_buffer.h"
#include "hyperscan_expr_list.h"
#include <stdlib.h>
#include <string.h>

#ifndef HS_MAX_BUFFER_SIZE
#define HS_MAX_BUFFER_SIZE 1024
#endif

DLL_EXPORT_HS_FINDER void hs_finder_get_version (int* pmajor, int* pminor, int* pmicro)
{
  if (pmajor)
    *pmajor = HS_FINDER_VERSION_MAJOR;
  if (pminor)
    *pminor = HS_FINDER_VERSION_MINOR;
  if (pmicro)
    *pmicro = HS_FINDER_VERSION_MICRO;
}

DLL_EXPORT_HS_FINDER const char* hs_finder_get_version_string ()
{
  return HS_FINDER_VERSION_STRING;
}

struct hs_finder {
  struct search_data_buffer_struct* searchdatabuffer;
  struct hyperscan_expr_list_struct* hyperscanexprlist;
  match_event_handler matchfn;
  void* matchcallbackdata;
  search_data_buffer_output_fn* outputfn;
  void* outputcallbackdata;
  hs_database_t* database;
  hs_scratch_t* scratch;
  hs_stream_t* stream;
  struct hs_finder* next;
  struct hs_finder* last;
};

DLL_EXPORT_HS_FINDER struct hs_finder* hs_finder_initialize (hs_finder_match_fn matchfn, void* callbackdata)
{
  struct hs_finder* result;
  if ((result = (struct hs_finder*)malloc(sizeof(struct hs_finder))) != NULL) {
    result->searchdatabuffer = initialize_search_data_buffer();
    result->hyperscanexprlist = initialize_hyperscan_data();
    result->matchfn = (match_event_handler)matchfn;
    result->matchcallbackdata = callbackdata;
    result->outputfn = NULL;
    result->outputcallbackdata = NULL;
    result->database = NULL;
    result->scratch = NULL;
    result->stream = NULL;
    result->next = NULL;
    result->last = result;
  }
  return result;
}

DLL_EXPORT_HS_FINDER void hs_finder_cleanup (struct hs_finder* finder)
{
  struct hs_finder* current;
  struct hs_finder* next;
  current = finder;
  while (current) {
    next = current->next;
    if (current->searchdatabuffer)
      deinitialize_search_data_buffer(current->searchdatabuffer);
    if (current->hyperscanexprlist)
      deinitialize_hyperscan_data(current->hyperscanexprlist);
    if (current->scratch)
      hs_free_scratch(current->scratch);
    if (current->database)
      hs_free_database(current->database);
    free(current);
    current = next;
  }
}

DLL_EXPORT_HS_FINDER void hs_finder_add_expr (struct hs_finder* finder, char* expr, unsigned int flags, unsigned int id)
{
  if (expr)
    hyperscan_expr_list_add(finder->last->hyperscanexprlist, strdup(expr), flags, id);
}

DLL_EXPORT_HS_FINDER void hs_finder_add_instance (struct hs_finder* finder, hs_finder_match_fn matchfn, void* callbackdata)
{
  if (hyperscan_expr_list_count(finder->last->hyperscanexprlist) > 0) {
    finder->last->next = hs_finder_initialize(matchfn, callbackdata);
    finder->last = finder->last->next;
  }
}

DLL_EXPORT_HS_FINDER size_t hs_finder_output_to_stream (void* callbackdata, const char* data, size_t datalen)
{
  return fwrite(data, 1, datalen, (FILE*)callbackdata);
}

DLL_EXPORT_HS_FINDER size_t hs_finder_output_to_null (void* callbackdata, const char* data, size_t datalen)
{
  return datalen;
}

DLL_EXPORT_HS_FINDER hs_error_t hs_finder_open (struct hs_finder* finder, search_data_buffer_output_fn outputfn, void* callbackdata)
{
  hs_error_t status;
  hs_compile_error_t *compile_err;
  struct hs_finder* current = finder;
  while (current) {
    //set output function (daisy chain with next if not last in chain, otherwise set final output function)
    if (current->next) {
      current->outputfn = (hs_finder_output_fn)hs_finder_process;
      current->outputcallbackdata = current->next;
    } else {
      current->outputfn = (search_data_buffer_output_fn*)(outputfn ? outputfn : hs_finder_output_to_stream);
      current->outputcallbackdata = callbackdata;
    }
    //reset output buffer
    reset_search_data_buffer(current->searchdatabuffer);
    //compile expressions
    if ((status = hs_compile_multi(hyperscan_expr_list_get_expressions(current->hyperscanexprlist), hyperscan_expr_list_get_flags(current->hyperscanexprlist), hyperscan_expr_list_get_ids(current->hyperscanexprlist), hyperscan_expr_list_count(current->hyperscanexprlist), HS_MODE_STREAM | HS_MODE_SOM_HORIZON_SMALL, NULL, &current->database, &compile_err)) != HS_SUCCESS) {
      fprintf(stderr, "ERROR: Unable to compile patterns: %s\n", compile_err->message);
      hs_free_compile_error(compile_err);
    }
    //allocate scratch space (can be reused for multiple calls to hs_scan)
    if (status == HS_SUCCESS && current->scratch == NULL) {
      /*current->scratch = NULL;*/
      if ((status = hs_alloc_scratch(current->database, &current->scratch)) != HS_SUCCESS) {
        fprintf(stderr, "ERROR: Unable to allocate scratch space. Exiting.\n");
        hs_free_database(current->database);
        current->database = NULL;
      }
    }
    //open stream
    if (status == HS_SUCCESS) {
      if ((status = hs_open_stream(current->database, HS_FLAG_SOM_LEFTMOST | HS_FLAG_DOTALL, &current->stream)) != HS_SUCCESS) {
        fprintf(stderr, "ERROR %i: Unable to open scan input stream. Exiting.\n", (int)status);
        hs_free_scratch(current->scratch);
        current->scratch = NULL;
        hs_free_database(current->database);
        current->database = NULL;
      }
    }
    current = current->next;
  }
  return status;
}

DLL_EXPORT_HS_FINDER hs_error_t hs_finder_process (struct hs_finder* finder, const char* data, size_t datalen)
{
  hs_error_t status;
  size_t buflen;
#ifdef HS_MAX_BUFFER_SIZE
  //flush buffer in case it gets too large
  if ((buflen = search_data_buffer_get_len(finder->searchdatabuffer)) + datalen > HS_MAX_BUFFER_SIZE)
    search_data_buffer_flush_fn(finder->searchdatabuffer, search_data_buffer_get_pos(finder->searchdatabuffer) + buflen + datalen - HS_MAX_BUFFER_SIZE, finder->outputfn, finder->outputcallbackdata);
#endif
  //add new data to buffer
  search_data_buffer_add(finder->searchdatabuffer, data, datalen);
  //scan new data
  if ((status = hs_scan_stream(finder->stream, data, datalen, 0, finder->scratch, finder->matchfn, finder)) != HS_SUCCESS) {
    fprintf(stderr, "ERROR %i in hs_scan_stream()\n", (int)status);
  }
  return status;
}

DLL_EXPORT_HS_FINDER hs_error_t hs_finder_close (struct hs_finder* finder)
{
  hs_error_t status;
  struct hs_finder* current = finder;
  while (current) {
    search_data_buffer_flush_remaining_fn(current->searchdatabuffer, current->outputfn, current->outputcallbackdata);
    if ((status = hs_close_stream(current->stream, current->scratch, current->matchfn, finder)) != HS_SUCCESS) {
      fprintf(stderr, "ERROR %i in hs_close_stream()\n", (int)status);
    }
    if (current->scratch) {
      hs_free_scratch(current->scratch);
      current->scratch = NULL;
    }
    if (current->database) {
      hs_free_database(current->database);
      current->database = NULL;
    }
    current = current->next;
  }
  return status;
}

DLL_EXPORT_HS_FINDER size_t hs_finder_get_pos (struct hs_finder* finder)
{
  return search_data_buffer_get_pos(finder->searchdatabuffer);
}

DLL_EXPORT_HS_FINDER void* hs_finder_get_callbackdata (struct hs_finder* finder)
{
  return finder->matchcallbackdata;
}

DLL_EXPORT_HS_FINDER size_t hs_finder_flush (struct hs_finder* finder, size_t flushpos)
{
  return search_data_buffer_flush_fn(finder->searchdatabuffer, flushpos, finder->outputfn, finder->outputcallbackdata);
}

DLL_EXPORT_HS_FINDER size_t hs_finder_skip (struct hs_finder* finder, size_t flushpos)
{
  return search_data_buffer_flush_fn(finder->searchdatabuffer, flushpos, NULL, NULL);
}

DLL_EXPORT_HS_FINDER size_t hs_finder_output (struct hs_finder* finder, const char* data, size_t datalen)
{
  return (*finder->outputfn)(finder->outputcallbackdata, data, datalen);
}

