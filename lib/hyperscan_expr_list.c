#include "hyperscan_expr_list.h"
#include <stdlib.h>
#include <string.h>

struct hyperscan_expr_list_struct {
  size_t entries;
  char** expressions;
  unsigned int* flags;
  unsigned int* ids;
};

struct hyperscan_expr_list_struct* initialize_hyperscan_data ()
{
  struct hyperscan_expr_list_struct* result;
  if ((result = (struct hyperscan_expr_list_struct*)malloc(sizeof(struct hyperscan_expr_list_struct))) != NULL) {
    result->entries = 0;
    result->expressions = NULL;
    result->flags = NULL;
    result->ids = NULL;
  }
  return result;
};

void deinitialize_hyperscan_data (struct hyperscan_expr_list_struct* searchdata)
{
  if (searchdata) {
    size_t i;
    if (searchdata->expressions) {
      for (i = 0; i < searchdata->entries; i++)
        free(searchdata->expressions[i]);
      free(searchdata->expressions);
    }
    if (searchdata->flags) {
      free(searchdata->flags);
    }
    if (searchdata->ids) {
      free(searchdata->ids);
    }
    free(searchdata);
  }
}

void hyperscan_expr_list_add (struct hyperscan_expr_list_struct* searchdata, char* expr, unsigned int flags, unsigned int id)
{
  size_t i = searchdata->entries++;
  if ((searchdata->expressions = (char**)realloc(searchdata->expressions, searchdata->entries * sizeof(char*))) != NULL)
    searchdata->expressions[i] = expr;
  if ((searchdata->flags = (unsigned int*)realloc(searchdata->flags, searchdata->entries * sizeof(unsigned int))) != NULL)
    searchdata->flags[i] = flags;
  if ((searchdata->ids = (unsigned int*)realloc(searchdata->ids, searchdata->entries * sizeof(unsigned int))) != NULL)
    searchdata->ids[i] = id;
}

size_t hyperscan_expr_list_count (struct hyperscan_expr_list_struct* searchdata)
{
  return searchdata->entries;
}

const char* const* hyperscan_expr_list_get_expressions (struct hyperscan_expr_list_struct* searchdata)
{
  return (const char* const*)searchdata->expressions;
}

const unsigned int* hyperscan_expr_list_get_flags (struct hyperscan_expr_list_struct* searchdata)
{
  return searchdata->flags;
}

const unsigned int* hyperscan_expr_list_get_ids (struct hyperscan_expr_list_struct* searchdata)
{
  return searchdata->ids;
}

