#include "include/ctst.h"

#ifdef CTST_STORAGE_TYPE_MALLOC_STORAGE
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

const size_t ctst_max_bytes_per_node = 1024; 

struct struct_ctst_storage {
	size_t node_count;
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
  storage->node_count = 0;
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
  result->bytes = (char*)malloc(bytes_length);
  memcpy(result->bytes,bytes+bytes_index,bytes_length);

  storage->node_count++;  

  return result;
}

void ctst_storage_node_free(ctst_storage* storage, ctst_node_ref node) {
  if(node->bytes_length>0) {
    free(node->bytes);
  }
  free(node);

  storage->node_count--;
}

/* Statistics about the storage */

size_t ctst_storage_node_count(ctst_storage* storage) {
  return storage->node_count;
}

size_t ctst_storage_memory_usage(ctst_storage* storage) {
  return sizeof(ctst_storage)+storage->node_count*sizeof(ctst_node);
}

/* Node attribute reading */

ctst_data ctst_storage_get_data(ctst_storage* storage, ctst_node_ref node) {
  return node->data;
}

ctst_node_ref ctst_storage_get_next(ctst_storage* storage, ctst_node_ref node) {
  return node->next;
}

ctst_node_ref ctst_storage_get_left(ctst_storage* storage, ctst_node_ref node) {
  return node->left;
}

ctst_node_ref ctst_storage_get_right(ctst_storage* storage, ctst_node_ref node) {
  return node->right;
}

size_t ctst_storage_get_bytes_length(ctst_storage* storage, ctst_node_ref node) {
  return node->bytes_length;
}

char ctst_storage_get_byte(ctst_storage* storage, ctst_node_ref node,size_t byte_index) {
  return node->bytes[byte_index];
}

char* ctst_storage_load_bytes(ctst_storage* storage, ctst_node_ref node) {
  return node->bytes;
}

void ctst_storage_unload_bytes(ctst_storage* storage, ctst_node_ref node, char* bytes) {
}

/* Node attribute writing */

void ctst_storage_set_data(ctst_storage* storage, ctst_balance_info* balance_info) {
  ctst_data old_data = balance_info->node->data;
  balance_info->node->data = balance_info->data;
  balance_info->data = old_data;
}

ctst_node_ref ctst_storage_set_next(ctst_storage* storage, ctst_node_ref node, ctst_node_ref next) {
  node->next = next;
  return node;
}

ctst_node_ref ctst_storage_set_left(ctst_storage* storage, ctst_node_ref node, ctst_node_ref left) {
  node->left = left;
  return node;
}

ctst_node_ref ctst_storage_set_right(ctst_storage* storage, ctst_node_ref node, ctst_node_ref right) {
  node->right = right;
  return node;
}

ctst_node_ref ctst_storage_set_bytes(ctst_storage* storage, ctst_node_ref node, char* bytes, size_t bytes_index, size_t bytes_length) {
  if(node->bytes_length>0) {
    free(node->bytes);
  }
  node->bytes_length = bytes_length;
  if(bytes_length>0) {
    node->bytes = (char*)malloc(bytes_length);
    memcpy(node->bytes,bytes+bytes_index,bytes_length);
  }
  return node;
}

/* Special node operations */

ctst_two_node_refs ctst_storage_split_node(ctst_storage* storage, ctst_node_ref node, size_t node_index) {
  ctst_two_node_refs result;
  
  result.ref1 = node;
  result.ref2 = ctst_storage_node_alloc(storage, node->data, node->next, node->left, node->right, node->bytes, node_index+1, node->bytes_length-node_index-1);
  
  node->data = 0;
  node->next = result.ref2;
  node->left = 0;
  node->right = 0; 
  node->bytes_length = node_index+1;
  node->bytes = realloc(node->bytes,node->bytes_length);
  
  return result;
}

ctst_node_ref ctst_storage_join_nodes(ctst_storage* storage, ctst_node_ref node) {
  if(node!=0 && node->data==0) {
    ctst_node_ref next = node->next;
    if(next!=0) {
      size_t new_bytes_length = node->bytes_length + next->bytes_length;
      if(node->left==0 && node->right==0 && new_bytes_length <= ctst_max_bytes_per_node) {
        /* At this point, we've got a node that should be merged with its
           next node. Normally, this can only be caused by the removal of
           a key from the tree. */
      
        /* We concatenate the bytes from the two nodes */
        char* bytes = node->bytes;
        bytes = realloc(bytes,new_bytes_length);
        memcpy(bytes+node->bytes_length,next->bytes,next->bytes_length);      
        
        /* We'll free the bytes from the next node with this node */
        node->bytes = next->bytes;
        ctst_storage_node_free(storage,node);
 
        /* The remaining node will be the next node */
        next->bytes = bytes;
        next->bytes_length = new_bytes_length;
        
        return next;
      }
    }
    else {
      /* We don't have a next node ; as we don't have data in the node,
         we may be in a one-way branch, in which case we can perform a merge. */
      ctst_node_ref branch = 0;
      ctst_node_ref left = node->left;
      ctst_node_ref right = node->right;

      /* Let's find out if we are in a one-way branch */      
      if(left==0) {
        if(right==0) {
          /* This node is totally useless, we free it. */
          ctst_storage_node_free(storage,node);
          return 0;
        }
        else {
          branch = right;
        }
      }
      else {
        if(right==0) {
          branch = left;
        }
        else {
          /* Nope, no one-way branch, this node must stay */
        }
      }
      
      if(branch!=0) {
        /* We'll rebuild the bytes in the branch node with all the bytes
           from the current node except the last one, which was the reason
           for the branch. */
        size_t new_bytes_length = node->bytes_length + branch->bytes_length - 1;
        char* bytes = node->bytes;
  
        bytes = realloc(bytes,new_bytes_length);
        memcpy(bytes+node->bytes_length-1,branch->bytes,branch->bytes_length);      

        /* We'll free the bytes from the branch node with this node */
        node->bytes = branch->bytes;
        ctst_storage_node_free(storage,node);
        
        /* The remaining node will be the branch node */
        branch->bytes = bytes;
        branch->bytes_length = new_bytes_length;
        
        return branch;
      }
    }
  }
  return node;
}
#endif
