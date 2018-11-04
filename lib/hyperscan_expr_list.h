#ifndef INCLUDED_HYPERSCAN_EXPR_LIST_H
#define INCLUDED_HYPERSCAN_EXPR_LIST_H

#include <stdlib.h>
#include <stdio.h>

/* C library defining a data type to define multiple search patterns for use with hyperscan */

#ifdef __cplusplus
extern "C" {
#endif

//data structure
struct hyperscan_expr_list_struct;

//initialize
struct hyperscan_expr_list_struct* initialize_hyperscan_data ();

//clean up
void deinitialize_hyperscan_data (struct hyperscan_expr_list_struct* searchdata);

//add data
void hyperscan_expr_list_add (struct hyperscan_expr_list_struct* searchdata, char* expr, unsigned int flags, unsigned int id);

//get number of entries
size_t hyperscan_expr_list_count (struct hyperscan_expr_list_struct* searchdata);

//get pointer to list of expressions
const char* const* hyperscan_expr_list_get_expressions (struct hyperscan_expr_list_struct* searchdata);

//get pointer to list of flags
const unsigned int* hyperscan_expr_list_get_flags (struct hyperscan_expr_list_struct* searchdata);

//get pointer to list of ids
const unsigned int* hyperscan_expr_list_get_ids (struct hyperscan_expr_list_struct* searchdata);

#ifdef __cplusplus
}
#endif

#endif //INCLUDED_HYPERSCAN_EXPR_LIST_H
