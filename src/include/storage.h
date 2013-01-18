#ifndef __CTST_STORAGE_H__
#define __CTST_STORAGE_H__
/*
  ctst - (c) 2007 Nicolas Lehuen
  This is released under the LGPL - see http://www.gnu.org/licenses/lgpl.html

  $Id: $

  This file defines the storage interface.
*/

#include <stdio.h>

/* This is the maximum number of bytes that can be stored
   in a single node. The exact value depends on the storage
   implementation. */
extern const size_t ctst_max_bytes_per_node;

/* This is the structure storing data about the storage. There
   again, its contents depends on the implementation. */
typedef struct struct_ctst_storage ctst_storage;

typedef struct struct_ctst_two_node_refs {
  ctst_node_ref ref1;
  ctst_node_ref ref2;
} ctst_two_node_refs;

typedef struct struct_ctst_balance_info {
  ctst_node_ref node;
  ctst_data data;
} ctst_balance_info;

/* Storage allocation / deallocation */
ctst_storage* ctst_storage_alloc();
void ctst_storage_free(ctst_storage* storage);

/* Node allocation / deallocation */
ctst_node_ref ctst_storage_node_alloc(ctst_storage* storage, ctst_data data, char* bytes, size_t bytes_index, size_t bytes_length, char next_byte, ctst_node_ref next_node);
void ctst_storage_node_free(ctst_storage* storage, ctst_node_ref node);

/* Statistics about the storage */
size_t ctst_storage_node_count(ctst_storage* storage);
size_t ctst_storage_memory_usage(ctst_storage* storage);

/* Node attribute reading */
ctst_data ctst_storage_get_data(ctst_storage* storage, ctst_node_ref node);
ctst_node_ref ctst_storage_get_next(ctst_storage* storage, ctst_node_ref node, char byte);
size_t ctst_storage_get_bytes_length(ctst_storage* storage, ctst_node_ref node);
char ctst_storage_get_byte(ctst_storage* storage, ctst_node_ref node, size_t byte_index);
void ctst_storage_load_bytes(ctst_storage* storage, ctst_node_ref node, char** bytes, size_t* bytes_length);
void ctst_storage_unload_bytes(ctst_storage* storage, ctst_node_ref node, char* bytes);

/* Node attribute writing */
void ctst_storage_set_data(ctst_storage* storage, ctst_balance_info* balance_info);
void ctst_storage_set_next(ctst_storage* storage, ctst_node_ref* node, char byte, ctst_node_ref next);
void ctst_storage_set_bytes(ctst_storage* storage, ctst_node_ref* node, char* bytes, size_t bytes_index, size_t bytes_length);

/* Special node operations */
ctst_two_node_refs ctst_storage_split_node(ctst_storage* storage, ctst_node_ref node, size_t split_index);
ctst_node_ref ctst_storage_join_nodes(ctst_storage* storage, ctst_node_ref node);

/* Visitor support */
typedef ctst_data(*ctst_visitor_function)(void* context, char *key_bytes, size_t key_length, ctst_data data, size_t distance);
ctst_data ctst_storage_visit_all(ctst_storage* ctst, ctst_visitor_function visitor, void* context, ctst_node_ref node, char** bytes, size_t bytes_length, size_t *bytes_limit);

/* Debug functions */
void ctst_storage_debug_node(ctst_storage* storage, ctst_node_ref node, FILE* output, int start);

#endif
