#ifndef INCLUDED_search_data_buffer_BUFFER_H
#define INCLUDED_search_data_buffer_BUFFER_H

#include <stdlib.h>
#include <stdio.h>

/* C library for buffering and flushing data to disk while it is being searched */

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t (search_data_buffer_output_fn) (void* callbackdata, const char* data, size_t datalen);

//data structure
struct search_data_buffer_struct;

//initialize
struct search_data_buffer_struct* initialize_search_data_buffer ();

//clean up
void deinitialize_search_data_buffer (struct search_data_buffer_struct* searchdata);

//reset
void reset_search_data_buffer (struct search_data_buffer_struct* searchdata);

//add data
void search_data_buffer_add (struct search_data_buffer_struct* searchdata, const char* data, size_t datalen);

//flush data to stream
size_t search_data_buffer_flush (struct search_data_buffer_struct* searchdata, size_t flushpos, FILE* dst);

//flush data using function
size_t search_data_buffer_flush_fn (struct search_data_buffer_struct* searchdata, size_t flushpos, search_data_buffer_output_fn flushfn, void* callbackdata);

//flush remaining data to stream
size_t search_data_buffer_flush_remaining (struct search_data_buffer_struct* searchdata, FILE* dst);

//flush remaining data using function
size_t search_data_buffer_flush_remaining_fn (struct search_data_buffer_struct* searchdata, search_data_buffer_output_fn flushfn, void* callbackdata);

//get number of bytes flushed
size_t search_data_buffer_get_pos (struct search_data_buffer_struct* searchdata);

//get number of bytes in buffer (not flushed yet)
size_t search_data_buffer_get_len (struct search_data_buffer_struct* searchdata);

//get pointer to buffer
const char* search_data_buffer_get_at_pos (struct search_data_buffer_struct* searchdata, size_t pos);

#ifdef __cplusplus
}
#endif

#endif //INCLUDED_search_data_buffer_BUFFER_H
