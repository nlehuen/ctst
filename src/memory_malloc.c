#include "include/ctst.h"

#ifdef CTST_STORAGE_TYPE_MEMORY_MALLOC
/*
  ctst - (c) 2007 Nicolas Lehuen
  This is released under the LGPL - see http://www.gnu.org/licenses/lgpl.html

  $Id: $

  This file implements a storage based on malloc/free.
  
  This is NOT an efficient implementation. its only purpose
  is to be a performance reference against which better implementation
  will be measured.
*/
#include <string.h>

struct struct_ctst_storage {
};

struct struct_ctst_node {
  ctst_node_ref next;
  ctst_node_ref left;
  ctst_node_ref right;
  ctst_data     data;
  char*         bytes;
  size_t        bytes_length;
};

/* Storage allocation / deallocation */

ctst_storage* ctst_storage_alloc() {
  ctst_storage* storage=(ctst_storage*)malloc(sizeof(ctst_storage));
  return storage;
}

void ctst_storage_free(ctst_storage* storage) {
  free(storage);
}

/* Node allocation / deallocation */

ctst_node_ref ctst_storage_node_alloc(ctst_storage* storage, ctst_data data, ctst_node_ref next, ctst_node_ref left, ctst_node_ref right, char* bytes, size_t bytes_index, size_t bytes_length) {
  ctst_node_ref result=(ctst_node_ref)malloc(sizeof(ctst_node));
  
  result->data = data;
  result->next = next;
  result->left = left;
  result->right = right;
  result->bytes_length = bytes_length;
  result->bytes = malloc(bytes_length);
  memcpy(result->bytes,bytes+bytes_index,bytes_length);
  
  return result;
}

void ctst_storage_node_free(ctst_storage* storage, ctst_node_ref node) {
  free(node->bytes);
  free(node);
}

/* Node attribute reading */

inline size_t ctst_storage_get_bytes_length(ctst_storage* storage, ctst_node_ref node) {
  return node->bytes_length;
}

inline char* ctst_storage_get_bytes(ctst_storage* storage, ctst_node_ref node) {
  return node->bytes;
}

#endif
