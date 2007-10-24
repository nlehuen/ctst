/*
  ctst - (c) 2007 Nicolas Lehuen
  This is released under the LGPL - see http://www.gnu.org/licenses/lgpl.html

  $Id: $

  This file contains the implementation of the ctst.
*/
#include "include/ctst.h"

struct struct_ctst_ctst {
  ctst_storage* storage;
  ctst_node_ref root;
};

ctst_ctst* ctst_alloc(ctst_storage* storage) {
  ctst_ctst* ctst = (ctst_ctst*)malloc(sizeof(ctst_ctst));
  ctst->storage = storage;
  ctst->root = 0;
  return ctst;   
}

void ctst_free(ctst_ctst* ctst) {
  free(ctst);
}

ctst_data ctst_get(ctst_ctst* ctst, char* bytes, size_t bytes_index, size_t bytes_length) {
  return 0;
}

ctst_data ctst_set(ctst_ctst* ctst, char* bytes, size_t bytes_index, size_t bytes_length,ctst_data data) {
  return 0;
}
