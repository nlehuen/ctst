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
#include "malloc.h"

struct struct_ctst_storage {
  unsigned int n_blocks;
  char** blocks;
};

struct struct_ctst_node {
  ctst_node_ref next;
  ctst_node_ref left;
  ctst_node_ref right;
  ctst_data     data;
  char*         bytes;
  size_t        bytes_length;
};

ctst_storage* ctst_storage_alloc() {
  ctst_storage* storage=(ctst_storage*)malloc(sizeof(ctst_storage));
  return storage;
}

void ctst_storage_free(ctst_storage* storage) {
  free(storage);
}

#endif
