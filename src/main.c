/*
  ctst - (c) 2007 Nicolas Lehuen
  This is released under the LGPL - see http://www.gnu.org/licenses/lgpl.html

  $Id: $

  Main test file.
*/
#include <stdio.h>

#include "include/ctst.h"

int main(int argc, char** argv) {
  ctst_storage* storage = ctst_storage_alloc();
  ctst_node_ref node = ctst_storage_node_alloc(storage,0,0,0,0,"Hello, world !\n",0,15);
  printf("%*s\n",ctst_storage_get_bytes_length(storage,node),ctst_storage_get_bytes(storage,node));
  ctst_storage_node_free(storage,node);
  ctst_storage_free(storage);
  
  return 0;
}
