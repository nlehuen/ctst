#include "ctst.h"

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
#include <assert.h>

const size_t ctst_max_bytes_per_node = 255; 

struct struct_ctst_storage {
	size_t node_count;
};

struct struct_ctst_node {
  ctst_data      data;
  char*          bytes;
  size_t         bytes_length;
  char*          next_bytes;
  ctst_node_ref* next_nodes;
  size_t         next_length;
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
    
    // dichotomy search
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
    
    // dichotomy search
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
      // We found next_byte in the next_bytes array
      // Let's look at the old node   
      ctst_node_ref old_node = node2->next_nodes[low];

      if(old_node!=next_node) {
        // We need to do something
        
        if(next_node!=0) {
          // Set the new node value
          node2->next_nodes[low] = next_node;
        } else {
          // Deletion of the link
          memmove(node2->next_bytes+low,node2->next_bytes+low+1,sizeof(char)*(node2->next_length-low-1));
          memmove(node2->next_nodes+low,node2->next_nodes+low+1,sizeof(ctst_node_ref)*(node2->next_length-low-1));
          node2->next_length--;
        
          // Free the old node
          // TODO : shouldn't we return the old node and
          // let the calling code free it ?
          if(old_node != 0) {
            ctst_storage_node_free(storage, old_node);
          }
        }
      }
    }
    else {
      // We haven't found next_byte in the array so we need to insert it
      if(next_node!=0) {
        // We need to insert a new link
        // The low index is the insertion point
        
        // We resize the link data arrays
        size_t length = node2->next_length;
        node2->next_length = length+1;
        node2->next_bytes=(char*)realloc(node2->next_bytes,sizeof(char)*(length+1));
        node2->next_nodes=(ctst_node_ref*)realloc(node2->next_nodes,sizeof(ctst_node_ref)*(length+1));
        
        if(low<length) {
          // If the insertion point is not at the end, we move all link data above the insertion point
          memmove(node2->next_bytes+low+1,node2->next_bytes+low,sizeof(char)*(length-low));
          memmove(node2->next_nodes+low+1,node2->next_nodes+low,sizeof(ctst_node_ref)*(length-low));
        }

        // We store the link data
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
  
  result.ref1 = ctst_storage_node_alloc(storage, 0, old_bytes, 0, split_index, node->bytes[split_index], node);
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

ctst_data ctst_storage_visit_all(ctst_storage* storage, ctst_visitor_function visitor, void* context, ctst_node_ref node, char** bytes, size_t bytes_length, size_t* bytes_limit) {
  size_t i;
  ctst_data result;

  if(node->data) {
    (*bytes)[bytes_length] = 0;
    result = visitor(context, *bytes, bytes_length, node->data, 0);
    if(result) {
      return result;
    }
  }

  for(i=0;i<node->next_length;i++) {
    char next_byte = node->next_bytes[i];
    ctst_node_ref next_node = node->next_nodes[i];

    if(bytes_length + next_node->bytes_length + 2 >= *bytes_limit) {
      *bytes_limit = (bytes_length + next_node->bytes_length)*3/2 + 2;
      *bytes = (char*)realloc(*bytes,sizeof(char)*(*bytes_limit));
    }

    (*bytes)[bytes_length] = next_byte;
    if(next_node->bytes_length>0) {
      assert(bytes_length + 1 + next_node->bytes_length < *bytes_limit);
      memcpy(*bytes+bytes_length+1,next_node->bytes,next_node->bytes_length);
    }

    result = ctst_storage_visit_all(storage, visitor, context, next_node, bytes, bytes_length + 1 + next_node->bytes_length, bytes_limit);
    if(result) {
      return result;
    }
  }
  
  return 0;
}

void ctst_storage_debug_node(ctst_storage* storage, ctst_node_ref node, FILE* output, int start) {
	if(start) {
		fprintf(output,"digraph tst {\ngraph [overlap=false];\n");
	}

	if(node) {
		int type=0;
		if(node->data) type+=4;
		if(node->bytes_length) type+=2;
		if(node->next_length) type+=1;
		
		fprintf(output, "N%lx [shape=record, label=\"{ %lx\\ntype=%i ",(unsigned long)node, (unsigned long)node, type);
		
		if(node->data) fprintf(output, " | data=%lx", (unsigned long)node->data);
		
		if(node->bytes_length > 0) {
			// Build a NULL terminated string
		    char cstr[10240];
		    memcpy(cstr, node->bytes, sizeof(char)*node->bytes_length);
		    cstr[node->bytes_length] = '\0';
		 	fprintf(output, " | \\\"%s\\\"", cstr);
		}

		if(node->next_length > 0) {		
			fprintf(output, " | {");
			int i,l;
			for(i=0,l=node->next_length;i<l;i++) {
				if(i>0) fprintf(output, " | ");
				fprintf(output, "<P%i> %c",i,node->next_bytes[i]);
			}
			fprintf(output, "}");
		}
				
		fprintf(output, "}\"];\n");
		
		int i,l;
		for(i=0,l=node->next_length;i<l;i++) {
			ctst_storage_debug_node(storage, node->next_nodes[i], output, 0);		
			fprintf(output, "N%lx:P%i -> N%lx;\n",(unsigned long)node,i,(unsigned long)node->next_nodes[i]);
		}
	}
		
	if(start) {
		fprintf(output,"}\n");
	}
}
#endif
