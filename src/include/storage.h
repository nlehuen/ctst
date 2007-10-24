#ifndef __CTST_STORAGE_H__
#define __CTST_STORAGE_H__
/*
  ctst - (c) 2007 Nicolas Lehuen
  This is released under the LGPL - see http://www.gnu.org/licenses/lgpl.html

  $Id: $

  This file defines the storage interface.
*/

extern int ctst_max_bytes_per_node;
typedef struct struct_ctst_storage ctst_storage;


typedef struct struct_ctst_tworefs {
  ctst_node_ref ref1;
  ctst_node_ref ref2;
} ctst_tworefs;

/* Storage allocation / deallocation */
ctst_storage* ctst_storage_alloc();
void ctst_storage_free(ctst_storage* storage);

/* Node allocation / deallocation */
ctst_node_ref ctst_storage_node_alloc(ctst_storage* storage, ctst_data data, ctst_node_ref next, ctst_node_ref left, ctst_node_ref right, char* bytes, size_t bytes_index, size_t bytes_length);
void ctst_storage_node_free(ctst_storage* storage, ctst_node_ref node);

/* Node attribute reading */
ctst_data ctst_storage_get_data(ctst_storage* storage, ctst_node_ref node);
ctst_node_ref ctst_storage_get_next(ctst_storage* storage, ctst_node_ref node);
ctst_node_ref ctst_storage_get_left(ctst_storage* storage, ctst_node_ref node);
ctst_node_ref ctst_storage_get_right(ctst_storage* storage, ctst_node_ref node);
size_t ctst_storage_get_bytes_length(ctst_storage* storage, ctst_node_ref node);
char ctst_storage_get_byte(ctst_storage* storage, ctst_node_ref node,size_t byte_index);
char* ctst_storage_load_bytes(ctst_storage* storage, ctst_node_ref node);
void ctst_storage_unload_bytes(ctst_storage* storage, ctst_node_ref node, char* bytes);

/* Node attribute writing */
ctst_node_ref ctst_storage_set_data(ctst_storage* storage, ctst_node_ref node, ctst_data data);
ctst_node_ref ctst_storage_set_next(ctst_storage* storage, ctst_node_ref node, ctst_node_ref next);
ctst_node_ref ctst_storage_set_left(ctst_storage* storage, ctst_node_ref node, ctst_node_ref left);
ctst_node_ref ctst_storage_set_right(ctst_storage* storage, ctst_node_ref node, ctst_node_ref right);
ctst_node_ref ctst_storage_set_bytes(ctst_storage* storage, ctst_node_ref node, char* bytes, size_t bytes_index, size_t bytes_length);

/* Special node operations */
ctst_tworefs ctst_storage_swap_bytes(ctst_storage* storage, ctst_node_ref node1, ctst_node_ref node2);
void ctst_storage_swap_last_byte(ctst_storage* storage, ctst_node_ref node1, ctst_node_ref node2);
ctst_tworefs ctst_storage_split_node(ctst_storage* storage, ctst_node_ref node1, ctst_node_ref node2);

#endif
