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
  ctst_data     data;
  char*         bytes;
  size_t        bytes_length;
  char* next_bytes;
  ctst_node_ref* next_nodes;
  size_t next_length;
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

ctst_node_ref ctst_storage_node_alloc(ctst_storage* storage, ctst_data data, char* bytes, size_t bytes_index, size_t bytes_length, char next_byte, ctst_node_ref next_node) {
  ctst_node_ref result=(ctst_node_ref)malloc(sizeof(ctst_node));
  
  result->data = data;
  result->bytes_length = bytes_length;
  if(bytes_length>0) {
    result->bytes = (char*)malloc(sizeof(char)*bytes_length);
    memcpy(result->bytes,bytes+bytes_index,sizeof(char)*bytes_length);
  }
  if(next_node) {
    result->next_length = 1;
    result->next_bytes = (char*)malloc(sizeof(char));
    *(result->next_bytes) = next_byte;
    result->next_nodes = (ctst_node_ref*)malloc(sizeof(ctst_node_ref));
    *(result->next_nodes) = next_node;
  } else {
    result->next_length = 0;
  }
  storage->node_count++;  

  return result;
}

void ctst_storage_node_free(ctst_storage* storage, ctst_node_ref node) {
  if(node->bytes_length>0) {
    free(node->bytes);
  }
  if(node->next_length>0) {
    size_t i;
    for(i=0;i<node->next_length;i++) {
      ctst_storage_node_free(storage, node->next_nodes[i]);
    }
    free(node->next_bytes);
    free(node->next_nodes);
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

ctst_node_ref ctst_storage_get_next(ctst_storage* storage, ctst_node_ref node, char next_byte) {
  if(node->next_length==0) {
    return 0;
  } else {
    size_t low = 0;
    size_t high = node->next_length;
    size_t mid;
    
    while(low < high) {
      mid = low + ((high - low) >> 1);
      if(node->next_bytes[mid] < next_byte) {
        low = mid + 1;  
      }
      else {
        high = mid;
      }
    }
    
    if(low<node->next_length && node->next_bytes[low]==next_byte) {
      return node->next_nodes[low];
    }
    else {
      return 0;
    }
  }
}

size_t ctst_storage_get_bytes_length(ctst_storage* storage, ctst_node_ref node) {
  return node->bytes_length;
}

char ctst_storage_get_byte(ctst_storage* storage, ctst_node_ref node,size_t byte_index) {
  return node->bytes[byte_index];
}

void ctst_storage_load_bytes(ctst_storage* storage, ctst_node_ref node, char** bytes, size_t* bytes_length) {
  *bytes = node->bytes;
  *bytes_length = node->bytes_length;  
}

void ctst_storage_unload_bytes(ctst_storage* storage, ctst_node_ref node, char* bytes) {
}

/* Node attribute writing */

void ctst_storage_set_data(ctst_storage* storage, ctst_balance_info* balance_info) {
  ctst_data old_data = balance_info->node->data;
  balance_info->node->data = balance_info->data;
  balance_info->data = old_data;
}

void ctst_storage_set_next(ctst_storage* storage, ctst_node_ref* node, char next_byte, ctst_node_ref next_node) {
  ctst_node_ref node2 = *node;
  if(node2->next_length==0) {
    if(next_node!=0) {
      node2->next_bytes = (char*)malloc(sizeof(char));
      *(node2->next_bytes) = next_byte;
      node2->next_nodes = (ctst_node_ref*)malloc(sizeof(ctst_node_ref));
      *(node2->next_nodes) = next_node;
      node2->next_length = 1;
    }
  } else {
    size_t low = 0;
    size_t high = node2->next_length;
    size_t mid;
    
    while(low < high) {
      mid = low + ((high - low) >> 1);
      if(node2->next_bytes[mid] < next_byte) {
        low = mid + 1;  
      }
      else {
        high = mid;
      }
    }
    
    if(low<node2->next_length && node2->next_bytes[low]==next_byte) {
      ctst_node_ref old_node = node2->next_nodes[low];
      if(old_node!=next_node) {
        if(next_node!=0) {
          node2->next_nodes[low] = next_node;
        } else {
          memcpy(node2->next_bytes+low,node2->next_bytes+low+1,sizeof(char)*(node2->next_length-low-1));
          memcpy(node2->next_nodes+low,node2->next_nodes+low+1,sizeof(ctst_node_ref)*(node2->next_length-low-1));
          node2->next_length--;
        }
      }
    }
    else {
      if(next_node!=0) {
        if(low<node2->next_length) {
          node2->next_length++;
          node2->next_bytes=(char*)realloc(node2->next_bytes,sizeof(char)*node2->next_length);
          memcpy(node2->next_bytes+low+1,node2->next_bytes+low,sizeof(char)*(node2->next_length-low-1));
          node2->next_nodes=(ctst_node_ref*)realloc(node2->next_nodes,sizeof(ctst_node_ref)*node2->next_length);
          memcpy(node2->next_nodes+low+1,node2->next_nodes+low,sizeof(ctst_node_ref)*(node2->next_length-low-1));
        } else {
          node2->next_length++;
          node2->next_bytes=(char*)realloc(node2->next_bytes,sizeof(char)*node2->next_length);
          node2->next_nodes=(ctst_node_ref*)realloc(node2->next_nodes,sizeof(ctst_node_ref)*node2->next_length);
        }

        node2->next_bytes[low] = next_byte;
        node2->next_nodes[low] = next_node;
      }
    }
  }
}

void ctst_storage_set_bytes(ctst_storage* storage, ctst_node_ref* node, char* bytes, size_t bytes_index, size_t bytes_length) {
  ctst_node_ref node2 = *node;
  if(node2->bytes_length>0) {
    if(bytes_length>0) {
      node2->bytes = (char*)realloc(node2->bytes,sizeof(char)*bytes_length);
      memcpy(node2->bytes,bytes+bytes_index,sizeof(char)*bytes_length);
    }
    else {
      free(node2->bytes);
    }
  } else {
    if(bytes_length>0) {
      node2->bytes = (char*)malloc(sizeof(char)*bytes_length);
      memcpy(node2->bytes,bytes+bytes_index,sizeof(char)*bytes_length);
    }
  }
  node2->bytes_length = bytes_length;
}

/* Special node operations */

ctst_two_node_refs ctst_storage_split_node(ctst_storage* storage, ctst_node_ref node, size_t split_index) {
  ctst_two_node_refs result;
  char* old_bytes = node->bytes;
  
  result.ref1 = ctst_storage_node_alloc(storage, 0, old_bytes, 0, split_index, node->bytes[split_index],node);
  result.ref2 = node;
  
  node->bytes_length = node->bytes_length - 1 - split_index;
  if(node->bytes_length>0) {
    node->bytes = (char*)malloc(sizeof(char)*node->bytes_length);
    memcpy(node->bytes,old_bytes+split_index+1,sizeof(char)*node->bytes_length);
  }

  free(old_bytes);

  return result;
}

ctst_node_ref ctst_storage_join_nodes(ctst_storage* storage, ctst_node_ref node) {
  /* TODO */
  return node;
}

#endif
