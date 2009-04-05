/*
  ctst - (c) 2007 Nicolas Lehuen
  This is released under the LGPL - see http://www.gnu.org/licenses/lgpl.html

  $Id: $

  This file contains the implementation of the ctst.
*/
#include "ctst.h"

#include <string.h>

/* Private functions of this module */
void _ctst_recursive_set(ctst_ctst* ctst, char* bytes, size_t bytes_index, size_t bytes_length,ctst_balance_info* balance_info, size_t local_index);
void _ctst_recursive_remove(ctst_ctst* ctst, char* bytes, size_t bytes_index, size_t bytes_length,ctst_balance_info* balance_info, size_t local_index);
ctst_node_ref _ctst_new_node(ctst_ctst* ctst, char* bytes, size_t bytes_index, size_t bytes_length,ctst_data data, size_t local_index);
void _ctst_compute_balance(ctst_ctst* ctst, ctst_balance_info* balance_info);
void _ctst_balance_node(ctst_ctst* ctst, ctst_balance_info* balance_info);

/* ctst allocation / deallocation */

ctst_ctst* ctst_alloc() {
  ctst_storage* storage = ctst_storage_alloc();
  ctst_ctst* ctst = (ctst_ctst*)malloc(sizeof(ctst_ctst));
  ctst->storage = storage;
  ctst->root = 0;
  ctst->size = 0;
  ctst->total_key_length = 0;
  return ctst;
}

void ctst_free(ctst_ctst* ctst) {
  ctst_storage_free(ctst->storage);
  free(ctst);
}

/* A few statistics about the ctst */

size_t ctst_get_size(ctst_ctst* ctst) {
  return ctst->size;
}

size_t ctst_get_total_key_length(ctst_ctst* ctst) {
  return ctst->total_key_length;
}

size_t ctst_get_node_count(ctst_ctst* ctst) {
  return ctst_storage_node_count(ctst->storage);
}

size_t ctst_get_memory_usage(ctst_ctst* ctst) {
  return sizeof(ctst_ctst)+ctst_storage_memory_usage(ctst->storage);
}

float ctst_get_ratio(ctst_ctst* ctst) {
  return 1.0f*ctst_get_memory_usage(ctst)/(ctst->total_key_length+ctst->size*sizeof(ctst_data));
}

/* Basic accessors : get and set */

ctst_data ctst_get(ctst_ctst* ctst, char* bytes, size_t bytes_index, size_t bytes_length) {
  ctst_node_ref node = ctst->root;
  size_t index = 0;
  
  while(node!=0) {
    char* node_bytes;
    size_t node_bytes_length;
    size_t local_index=0;
    int diff=0;

    /* We load the bytes from the node into local memory */
    ctst_storage_load_bytes(ctst->storage,node,&node_bytes,&node_bytes_length);

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

    /* We unload the bytes from local memory */
    ctst_storage_unload_bytes(ctst->storage,node,node_bytes);
    
    if (diff != 0) {
      /* We got a mismatch. We stopped before the last byte of the node.
         A match is impossible, otherwise a split
         would have occured during the insertion. */
      return 0;
    } else if (local_index == node_bytes_length) {
      /* We matched all the bytes of the node */

      if (index == bytes_length) {
        /* We also matched all the bytes of the key, so
           we've got our result ! */
        return ctst_storage_get_data(ctst->storage,node);
      } else {
        /* The key is not over yet, so we try to advance
           within the tree. */
        node = ctst_storage_get_next(ctst->storage,node,bytes[bytes_index]);
        local_index=0;
        bytes_index++;
        index++;
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
  ctst_balance_info result;
  result.node = ctst->root;
  result.data = data;
  _ctst_recursive_set(ctst,bytes,bytes_index,bytes_length,&result,0);
  ctst->root = result.node;
  return result.data;
}

void _ctst_recursive_set(ctst_ctst* ctst, char* bytes, size_t bytes_index, size_t bytes_length, ctst_balance_info* balance_info, size_t local_index) {
  if(balance_info->node==0) {
    balance_info->node=_ctst_new_node(ctst,bytes,bytes_index,bytes_length,balance_info->data,local_index);
    balance_info->data=0;
  }
  else {
    /* We load the bytes from the node into local memory */
    char* node_bytes;
    char last_byte;
    size_t node_bytes_length;
    size_t node_index = 0;
    int diff = 0;

    /* We load the bytes from the node into local memory */
    ctst_storage_load_bytes(ctst->storage,balance_info->node,&node_bytes,&node_bytes_length);

    /* We keep advancing within the node while the bytes match */
    last_byte = bytes[bytes_index+local_index];
    while (node_index < node_bytes_length && local_index < bytes_length) {
      last_byte = bytes[bytes_index+local_index];
      diff = last_byte - node_bytes[node_index];
      if (diff == 0) {
          local_index++;
          node_index++;
      } else {
          break;
      }
    } 

    if(diff!=0) {
      ctst_balance_info next_balance_info;

      /* we need to split the node */
      ctst_two_node_refs splitted = ctst_storage_split_node(ctst->storage,balance_info->node,node_index);
      
      /* Maybe the next node can be joined, following the split */
      ctst_node_ref joined = ctst_storage_join_nodes(ctst->storage,splitted.ref2);
      if(splitted.ref2 != joined) {
        ctst_storage_set_next(ctst->storage,&(splitted.ref1),last_byte,joined); 
      }

      balance_info->node = splitted.ref1;

      next_balance_info.node = ctst_storage_get_next(ctst->storage, balance_info->node, last_byte); 
      next_balance_info.data = balance_info->data;

      _ctst_recursive_set(ctst,bytes,bytes_index,bytes_length,&next_balance_info,local_index+1);

      ctst_storage_set_next(ctst->storage, &(balance_info->node), last_byte, next_balance_info.node);        
      balance_info->data = next_balance_info.data;
    }
    else if (node_index == node_bytes_length) {
      /* We reached the end of the bytes of the node, without differences */
      
      if(local_index == bytes_length) {
        /* We also reached the end of the key, therefore we are in the right
           place to insert the data ! */
        ctst_storage_set_data(ctst->storage, balance_info);
      }
      else {
        /* We haven't finished the key, so we go to the next node */
        ctst_node_ref previous_next = ctst_storage_get_next(ctst->storage,balance_info->node,bytes[bytes_index+local_index]); 
        ctst_balance_info next_info;
        next_info.node = previous_next;
        next_info.data = balance_info->data;
        _ctst_recursive_set(ctst,bytes,bytes_index,bytes_length,&next_info,local_index+1);
        balance_info->data = next_info.data;
        if(previous_next!=next_info.node) {
          ctst_storage_set_next(ctst->storage,&(balance_info->node),bytes[bytes_index+local_index],next_info.node); 
        }
     }
    }
    else {
      /* We reached the end of the key, but not the end of the bytes
       for this node. Therefore, we need to split this node. */
      ctst_two_node_refs splitted = ctst_storage_split_node(ctst->storage,balance_info->node,node_index);

      /* Maybe the next node can be joined, following the split */
      ctst_node_ref joined = ctst_storage_join_nodes(ctst->storage,splitted.ref2);
      if(splitted.ref2 != joined) {
        ctst_storage_set_next(ctst->storage,&(splitted.ref1),last_byte,joined); 
      }

      balance_info->node = splitted.ref1;
      ctst_storage_set_data(ctst->storage,balance_info);
    }
  }
}

ctst_data ctst_remove(ctst_ctst* ctst, char* bytes, size_t bytes_index, size_t bytes_length) {
  ctst_balance_info result;
  result.node = ctst->root;
  result.data = 0;
  _ctst_recursive_remove(ctst,bytes,bytes_index,bytes_length,&result,0);
  ctst->root = result.node;
  return result.data;
}

void _ctst_recursive_remove(ctst_ctst* ctst, char* bytes, size_t bytes_index, size_t bytes_length, ctst_balance_info* balance_info, size_t local_index) {
  if(balance_info->node==0) {
    return;
  }
  else {
    char* node_bytes;
    size_t node_bytes_length;
    size_t node_index = 0;
    int diff = 0;
    
    /* We load the bytes from the node into local memory */
    ctst_storage_load_bytes(ctst->storage,balance_info->node,&node_bytes,&node_bytes_length);

    /* We keep advancing within the node while the bytes match */
    while (node_index < node_bytes_length && local_index < bytes_length) {
      diff = bytes[bytes_index+local_index] - node_bytes[node_index];
      if (diff == 0) {
          local_index++;
          node_index++;
      } else {
          break;
      }
    } 

    if(diff!=0) {
      ctst_balance_info left_balance_info;
    
      if(node_index<node_bytes_length-1) {
        /* Since there is a mismatch before the last byte of the node, we
           bail out, the key isn't in the tree. */
        return;
      }
      
      left_balance_info.node = ctst_storage_get_next(ctst->storage, balance_info->node, bytes[bytes_index+local_index]);
      left_balance_info.data = balance_info->data;
      _ctst_recursive_remove(ctst,bytes,bytes_index,bytes_length,&left_balance_info,local_index+1);
      ctst_storage_set_next(ctst->storage, &(balance_info->node), bytes[bytes_index+local_index], left_balance_info.node);
      balance_info->data = left_balance_info.data;

      /* There we check whether we can get rid of this node */
      balance_info->node = ctst_storage_join_nodes(ctst->storage,balance_info->node);
    }
    else if (node_index == node_bytes_length) {
      /* We reached the end of the bytes of the node, without differences */
      
      if(local_index == bytes_length) {
        /* We also reached the end of the key, therefore we are in the right
           place to remove the data ! */
        ctst_storage_set_data(ctst->storage, balance_info);
        ctst->size--;
        ctst->total_key_length-=bytes_length;
      }
      else {
        /* We haven't finished the key, so we go to the next node */
        ctst_node_ref previous_next = ctst_storage_get_next(ctst->storage,balance_info->node, bytes[bytes_index+local_index]); 
        ctst_balance_info next_info;
        next_info.node = previous_next;
        next_info.data = balance_info->data;
        _ctst_recursive_remove(ctst,bytes,bytes_index,bytes_length,&next_info,local_index+1);
        balance_info->data = next_info.data;
        if(previous_next!=next_info.node) {
          ctst_storage_set_next(ctst->storage,&(balance_info->node),bytes[bytes_index+local_index],next_info.node); 
        }
      }
    }
    else {
      /* We reached the end of the key, but not the end of the bytes
       for this node. Therefore, the key is not in the tree. */
      return;
    }
  }
}

ctst_node_ref _ctst_new_node(ctst_ctst* ctst, char* bytes, size_t bytes_index, size_t bytes_length,ctst_data data, size_t local_index) {
  size_t local_size = bytes_length - local_index;
  if(local_size>ctst_max_bytes_per_node) {
    ctst_node_ref next;

    local_size = ctst_max_bytes_per_node;
    next = _ctst_new_node(ctst,bytes,bytes_index,bytes_length,data,local_index+local_size);
 
    return ctst_storage_node_alloc(ctst->storage,0,bytes,bytes_index+local_index,local_size,0,0);    
  }
  else {
    ctst->size++;
    ctst->total_key_length+=bytes_length;
    return ctst_storage_node_alloc(ctst->storage,data,bytes,bytes_index+local_index,local_size,0,0);
  }
}

/* Visitor pattern */

ctst_data ctst_visit_all(ctst_ctst* ctst, ctst_visitor_function visitor, void* context) {
  ctst_data result = 0;

  if(ctst->root) {
    size_t bytes_limit,node_bytes_length;
    char* bytes;
    char* node_bytes;

    /* We load the bytes from the node into local memory */
    ctst_storage_load_bytes(ctst->storage,ctst->root,&node_bytes,&node_bytes_length);

    bytes_limit = node_bytes_length*3/2 + 16;
    bytes = (char*)malloc(sizeof(char)*bytes_limit);
    memcpy(bytes,node_bytes,node_bytes_length);

    /* We unload the bytes from local memory */
    ctst_storage_unload_bytes(ctst->storage,ctst->root,node_bytes);

    result = ctst_storage_visit_all(ctst->storage, visitor, context, ctst->root, &bytes, node_bytes_length, &bytes_limit);

    free(bytes);
  }

  return result;
}

ctst_data ctst_visit_all_from_key(ctst_ctst* ctst, ctst_visitor_function visitor, void* context, char* bytes, size_t bytes_index, size_t bytes_length) {
  ctst_node_ref node = ctst->root;
  size_t index = 0;
  
  while(node!=0) {
    char* node_bytes;
    size_t node_bytes_length;
    size_t local_index=0;
    int diff=0;

    /* We load the bytes from the node into local memory */
    ctst_storage_load_bytes(ctst->storage,node,&node_bytes,&node_bytes_length);

    /* We keep advancing within the node while the bytes match */
    while (local_index < node_bytes_length && index < bytes_length) {
      diff = bytes[bytes_index+index] - node_bytes[local_index];
      if (diff == 0) {
          local_index++;
          index++;
      } else {
          break;
      }
    } 

    /* We unload the bytes from local memory */
    ctst_storage_unload_bytes(ctst->storage,node,node_bytes);
    
    if (diff != 0) {
      /* We got a mismatch. We stopped before the last byte of the node.
         A match is impossible, otherwise a split
         would have occured during the insertion. */
        return 0;
    } else if (local_index == node_bytes_length) {
      /* We matched all the bytes of the node */

      if (index == bytes_length) {
        /* We also matched all the bytes of the key, so
           we've got our result ! */
        ctst_data result;

        size_t bytes_limit = bytes_length*3/2;
        char* bytes_buffer = (char*)malloc(sizeof(char)*bytes_limit);
        memcpy(bytes_buffer,bytes+bytes_index,bytes_length);

        result = ctst_storage_visit_all(ctst->storage, visitor, context, node, &bytes_buffer, bytes_length, &bytes_limit);

        free(bytes_buffer);

        return result;
      } else {
        /* The key is not over yet, so we try to advance
           within the tree. */
        node = ctst_storage_get_next(ctst->storage,node,bytes[bytes_index+index]);
        local_index=0;
        index++;
      }
    } else {
      /* We've got a match but the key ended before the node. */
      ctst_data result;

      size_t bytes_limit = (bytes_length + node_bytes_length - local_index)*3/2 + 1;
      char* bytes_buffer = (char*)(char*)malloc(sizeof(char)*bytes_limit);
      memcpy(bytes_buffer,bytes+bytes_index,bytes_length);
      memcpy(bytes_buffer+bytes_length,node_bytes+local_index,node_bytes_length-local_index); 

      result = ctst_storage_visit_all(ctst->storage, visitor, context, node, &bytes_buffer, bytes_length + node_bytes_length - local_index, &bytes_limit);

      free(bytes_buffer);

      return result;
    }
  }
  
  /* We reached an empty branch, therefore we don't have any result */
  return 0;
}
