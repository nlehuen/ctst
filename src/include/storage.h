/*
  ctst - (c) 2007 Nicolas Lehuen
  This is released under the LGPL - see http://www.gnu.org/licenses/lgpl.html

  $Id: $

  This file defines the storage interface.
*/
#ifndef __CTST_STORAGE_H__
#define __CTST_STORAGE_H__

/* Memory allocation */
ctst_node_ref ctst_storage_alloc(ctst_data data, ctst_node_ref next, ctst_node_ref left, ctst_node_ref right, unsigned char* bytes, int bytes_index, int bytes_length);
void ctst_storage_free(ctst_node_ref node);

#endif
