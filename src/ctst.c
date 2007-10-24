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
  ctst_node_ref node = ctst->root;
  size_t index = 0;
  
  while(node!=0) {
    /* We load the data from the node into local memory */
    char* node_bytes = ctst_storage_get_bytes(ctst->storage,node);
    int node_bytes_length = ctst_storage_get_bytes_length(ctst->storage,node);

    int local_index=0;
    int diff=0;
    
    /* We keep advancing within the node while the bytes match */
    while (local_index < node_bytes_length && index < bytes_length) {
      diff = bytes[bytes_index] - node_bytes[local_index];
      if (diff == 0) {
          local_index++;
          bytes_index++;
          index++;
      } else {
          break;
      }
    } 
    
    if (diff != 0) {
      /* We got a mismatch. */

      if (local_index < node_bytes_length - 1) {
        /* We stopped before the last byte of the node.
           A match is impossible, otherwise a split
           would have occured during the insertion. */
          return 0;
      } else if (diff > 0) {
        /* Mismatch on the last byte, we go to the left */
        node = ctst_storage_get_left(ctst->storage,node);
      } else {
        /* Mismatch on the last byte, we go to the left */
        node = ctst_storage_get_right(ctst->storage,node);
      }
    } else if (local_index == node_bytes_length) {
      /* We matched all the bytes of the node */

      if (index == bytes_length) {
        /* We also matched all the bytes of the key, so
           we've got our result ! */
        return ctst_storage_get_data(ctst->storage,node);
      } else {
        /* The key is not over yet, so we try to advance
           within the tree. */
        node = ctst_storage_get_right(ctst->storage,node);
      }
    } else {
      /* We've got a match but the key ended before the node.
         Therefore we don't have any result (otherwise we would have splitted
         the node during the insertion. */
      return 0;
    }
  }
  
  /* We reached an empty branch, therefore we don't have any result */
  return 0;
}

ctst_data ctst_set(ctst_ctst* ctst, char* bytes, size_t bytes_index, size_t bytes_length,ctst_data data) {
  return 0;
}
