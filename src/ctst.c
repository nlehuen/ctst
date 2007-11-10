/*
  ctst - (c) 2007 Nicolas Lehuen
  This is released under the LGPL - see http://www.gnu.org/licenses/lgpl.html

  $Id: $

  This file contains the implementation of the ctst.
*/
#include "include/ctst.h"
#include "include/ctst_stack.h"

struct struct_ctst_ctst {
  ctst_storage* storage;
  ctst_node_ref root;
  size_t size;
  size_t total_key_length;
};

/* Private functions of this module */
void _ctst_recursive_set(ctst_ctst* ctst, char* bytes, size_t bytes_index, size_t bytes_length,ctst_balance_info* balance_info, size_t local_index);
void _ctst_recursive_remove(ctst_ctst* ctst, char* bytes, size_t bytes_index, size_t bytes_length,ctst_balance_info* balance_info, size_t local_index);
ctst_node_ref _ctst_new_node(ctst_ctst* ctst, char* bytes, size_t bytes_index, size_t bytes_length,ctst_data data, size_t local_index);
void _ctst_compute_balance(ctst_ctst* ctst, ctst_balance_info* balance_info);
void _ctst_balance_node(ctst_ctst* ctst, ctst_balance_info* balance_info);

/* ctst allocation / deallocation */

ctst_ctst* ctst_alloc(ctst_storage* storage) {
  ctst_ctst* ctst = (ctst_ctst*)malloc(sizeof(ctst_ctst));
  ctst->storage = storage;
  ctst->root = 0;
  ctst->size = 0;
  ctst->total_key_length = 0;
  return ctst;
}

void ctst_free(ctst_ctst* ctst) {
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
        node = ctst_storage_get_next(ctst->storage,node);
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
    balance_info->did_balance=0;
    balance_info->height=1;
    balance_info->balance=0;
    balance_info->left_balance=0;
    balance_info->right_balance=0;
  }
  else {
    /* We load the bytes from the node into local memory */
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
      ctst_balance_info right_balance_info;
    
      if(node_index<node_bytes_length-1) {
        /* Since there is a mismatch before the last byte of the node, we
           need to split the node */
        ctst_two_node_refs splitted = ctst_storage_split_node(ctst->storage,balance_info->node,node_index);
        
        /* Maybe the next node can be joined, following the split */
        ctst_node_ref joined = ctst_storage_join_nodes(ctst->storage,splitted.ref2);
        balance_info->node = splitted.ref1;
        if(splitted.ref2 != joined) {
          balance_info->node = ctst_storage_set_next(ctst->storage,balance_info->node,joined); 
        }
      }
      
      left_balance_info.node = ctst_storage_get_left(ctst->storage,balance_info->node);
      right_balance_info.node = ctst_storage_get_right(ctst->storage,balance_info->node);

      if(diff>0) {
        /* We need to grow the left branch */

        left_balance_info.data = balance_info->data;
        _ctst_recursive_set(ctst,bytes,bytes_index,bytes_length,&left_balance_info,local_index);
        balance_info->node = ctst_storage_set_left(ctst->storage, balance_info->node, left_balance_info.node);
        balance_info->data = left_balance_info.data;
        balance_info->did_balance = left_balance_info.did_balance;
        
        _ctst_compute_balance(ctst,&right_balance_info); 
      }
      else {
        /* We need to grow the right branch */

        _ctst_compute_balance(ctst,&left_balance_info); 

        right_balance_info.data = balance_info->data;
        _ctst_recursive_set(ctst,bytes,bytes_index,bytes_length,&right_balance_info,local_index);
        balance_info->node = ctst_storage_set_right(ctst->storage, balance_info->node, right_balance_info.node);
        balance_info->data=right_balance_info.data;
        balance_info->did_balance = right_balance_info.did_balance;
      }
      
      /* Now we can compute the balance for this node. */        
      if(ctst_storage_get_bytes_length(ctst->storage,balance_info->node)>1) {
        balance_info->height = 1;
        balance_info->balance = 0;
      }
      else {
        balance_info->balance = left_balance_info.height - right_balance_info.height;
        if(balance_info->balance>=0) {
          balance_info->height = left_balance_info.height + 1;
        }
        else {
          balance_info->height = right_balance_info.height + 1;
        }
      }
      balance_info->left_balance = left_balance_info.balance;
      balance_info->right_balance = right_balance_info.balance;
      
      if(balance_info->did_balance==0) {
        _ctst_balance_node(ctst,balance_info);
      }
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
        ctst_node_ref previous_next = ctst_storage_get_next(ctst->storage,balance_info->node); 
        ctst_balance_info next_info;
        next_info.node = previous_next;
        next_info.data = balance_info->data;
        _ctst_recursive_set(ctst,bytes,bytes_index,bytes_length,&next_info,local_index);
        balance_info->data = next_info.data;
        if(previous_next!=next_info.node) {
          balance_info->node = ctst_storage_set_next(ctst->storage,balance_info->node,next_info.node); 
        }
      }

      _ctst_compute_balance(ctst, balance_info);
    }
    else {
      /* We reached the end of the key, but not the end of the bytes
       for this node. Therefore, we need to split this node. */
      ctst_two_node_refs splitted = ctst_storage_split_node(ctst->storage,balance_info->node,local_index - 1);

      /* Maybe the next node can be joined, following the split */
      ctst_node_ref joined = ctst_storage_join_nodes(ctst->storage,splitted.ref2);
      balance_info->node = splitted.ref1;
      if(splitted.ref2 != joined) {
        balance_info->node = ctst_storage_set_next(ctst->storage,balance_info->node,joined); 
      }

      ctst_storage_set_data(ctst->storage,balance_info);
      balance_info->height = 1;
      balance_info->did_balance = 1;
      balance_info->left_balance = 0;
      balance_info->right_balance = 0;
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
      ctst_balance_info right_balance_info;
    
      if(node_index<node_bytes_length-1) {
        /* Since there is a mismatch before the last byte of the node, we
           bail out, the key isn't in the tree. */
        return;
      }
      
      left_balance_info.node = ctst_storage_get_left(ctst->storage,balance_info->node);
      right_balance_info.node = ctst_storage_get_right(ctst->storage,balance_info->node);

      if(diff>0) {
        /* We need to follow the left branch */

        left_balance_info.data = balance_info->data;
        _ctst_recursive_remove(ctst,bytes,bytes_index,bytes_length,&left_balance_info,local_index);
        balance_info->node = ctst_storage_set_left(ctst->storage, balance_info->node, left_balance_info.node);
        balance_info->data = left_balance_info.data;
        balance_info->did_balance = left_balance_info.did_balance;
        
        _ctst_compute_balance(ctst,&right_balance_info); 
      }
      else {
        /* We need to follow the right branch */

        _ctst_compute_balance(ctst,&left_balance_info); 

        right_balance_info.data = balance_info->data;
        _ctst_recursive_remove(ctst,bytes,bytes_index,bytes_length,&right_balance_info,local_index);
        balance_info->node = ctst_storage_set_right(ctst->storage, balance_info->node, right_balance_info.node);
        balance_info->data=right_balance_info.data;
        balance_info->did_balance = right_balance_info.did_balance;
      }
      
      /* There we check whether we can get rid of this node */
      balance_info->node = ctst_storage_join_nodes(ctst->storage,balance_info->node);

      /* Now we can compute the balance for this node. */        
      if(ctst_storage_get_bytes_length(ctst->storage,balance_info->node)>1) {
        balance_info->height = 1;
        balance_info->balance = 0;
      }
      else {
        balance_info->balance = left_balance_info.height - right_balance_info.height;
        if(balance_info->balance>=0) {
          balance_info->height = left_balance_info.height + 1;
        }
        else {
          balance_info->height = right_balance_info.height + 1;
        }
      }
      balance_info->left_balance = left_balance_info.balance;
      balance_info->right_balance = right_balance_info.balance;
      
      if(balance_info->did_balance==0) {
        _ctst_balance_node(ctst,balance_info);
      }
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
        ctst_node_ref previous_next = ctst_storage_get_next(ctst->storage,balance_info->node); 
        ctst_balance_info next_info;
        next_info.node = previous_next;
        next_info.data = balance_info->data;
        _ctst_recursive_remove(ctst,bytes,bytes_index,bytes_length,&next_info,local_index);
        balance_info->data = next_info.data;
        if(previous_next!=next_info.node) {
          balance_info->node = ctst_storage_set_next(ctst->storage,balance_info->node,next_info.node); 
        }
      }

      /* There we check whether we can get rid of this node */
      balance_info->node = ctst_storage_join_nodes(ctst->storage,balance_info->node);

      _ctst_compute_balance(ctst, balance_info);
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
 
    return ctst_storage_node_alloc(ctst->storage,0,next,0,0,bytes,bytes_index+local_index,local_size);    
  }
  else {
    ctst->size++;
    ctst->total_key_length+=bytes_length;
    return ctst_storage_node_alloc(ctst->storage,data,0,0,0,bytes,bytes_index+local_index,local_size);
  }
}

void _ctst_compute_balance(ctst_ctst* ctst, ctst_balance_info* result) {
  if(result->node==0) {
    result->did_balance=0;
    result->height=0;
    result->balance=0;
    result->left_balance=0;
    result->right_balance=0;
  }
  else {
    if(ctst_storage_get_bytes_length(ctst->storage,result->node)>1) {
      result->did_balance=0;
      result->height=1;
      result->balance=0;
      result->left_balance=0;
      result->right_balance=0;
    }
    else {
      ctst_balance_info left;
      ctst_balance_info right;

      left.node = ctst_storage_get_left(ctst->storage, result->node);
      right.node = ctst_storage_get_right(ctst->storage, result->node);
      
      _ctst_compute_balance(ctst, &left);
      _ctst_compute_balance(ctst, &right);

      result->did_balance=0;
      result->balance = left.height - right.height;
      if(result->balance>=0) {
        result->height=left.height+1;
      }
      else {
        result->height=right.height+1;
      }
      result->left_balance=left.balance;
      result->right_balance=right.balance;
    }
  }
}

void _ctst_balance_node(ctst_ctst* ctst,ctst_balance_info* balance_info) {
  /* TODO : implement this */
}

/* Visitor pattern */

ctst_data ctst_visit_all(ctst_ctst* ctst, ctst_visitor_function visitor, void* context) {
  ctst_node_ref node, next_node;
  ctst_stack* stack;
  char* bytes;
  size_t bytes_index, bytes_length;
  ctst_data data;

  stack = ctst_stack_alloc();
  node = ctst->root;
  while(node!=0) {
    /* current : push all the bytes of the node except the last one */
    ctst_storage_load_bytes(ctst->storage, node, &bytes, &bytes_length);
    ctst_stack_push(stack, 0, bytes, 0, bytes_length-1);

    /* right */
    next_node = ctst_storage_get_right(ctst->storage, node);
    if(next_node!=0) {
      ctst_stack_push(stack, next_node, bytes, 0, 0);
    }

    /* left */
    next_node = ctst_storage_get_left(ctst->storage, node);
    if(next_node!=0) {
      ctst_stack_push(stack, next_node, bytes, 0, 0);
    }

    /* current : push the last byte of the node */
    ctst_stack_push(stack, 0, bytes, bytes_length-1, 1);
    ctst_storage_unload_bytes(ctst->storage, node, bytes);

    data = ctst_storage_get_data(ctst->storage, node);
    if(data!=0) {
      /* We call ctst_stack_peek to get the entire key. We pass
         a reference to next_node because it will be erased by the
         zero we just pushed in. */
      ctst_stack_peek(stack, &next_node, &bytes, &bytes_index, &bytes_length);
      data = visitor(context, bytes, bytes_index+bytes_length, data, 0);
      if(data != 0) {
        /* If the visitor function returned some data, we stop the visit */
        ctst_stack_free(stack);
        return data;
      }
    }

    /* next */
    next_node = ctst_storage_get_next(ctst->storage, node);
    if(next_node!=0) {
      /* No need to push anything since it would be popped immediatly */
      node = next_node;
    }
    else {
      while(ctst_stack_pop(stack, &node, &bytes, &bytes_index, &bytes_length)>0) {
        if(node!=0) break;
      }
    }
  }

  ctst_stack_free(stack);
  return 0;
}
