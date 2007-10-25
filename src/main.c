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
  ctst_ctst* ctst = ctst_alloc(storage);

  ctst_node_ref node = ctst_storage_node_alloc(storage,0,0,0,0,"Hello, world!",0,13);
  
  printf("%d : %*s\n",ctst_storage_get_bytes_length(storage,node),ctst_storage_get_bytes_length(storage,node),ctst_storage_load_bytes(storage,node));
  
  node = ctst_storage_set_data(storage,node,42);
  node = ctst_storage_set_bytes(storage,node,"The answer to your question is",0,30);
  printf("%*s %d\n",ctst_storage_get_bytes_length(storage,node),ctst_storage_load_bytes(storage,node),ctst_storage_get_data(storage,node));
  
  ctst_storage_node_free(storage,node);
  
  ctst_set(ctst,"The answer to your question is",0,30,42);
  printf("%*s %d\n",30,"The answer to your question is",ctst_get(ctst,"The answer to your question is",0,30));
  
  ctst_set(ctst,"Hello, world!",0,13,57);
  printf("%*s %d\n",13,"Hello, world!",ctst_get(ctst,"Hello, world!",0,13));

  ctst_set(ctst,"Hello, world!",0,13,62);
  printf("%*s %d\n",13,"Hello, world!",ctst_get(ctst,"Hello, world!",0,13));

  ctst_set(ctst,"The answer to your question is",0,30,42);
  printf("%*s %d\n",30,"The answer to your question is",ctst_get(ctst,"The answer to your question is",0,30));

  printf("Number of entries: %d\nTotal length of keys: %d\n",
    ctst_get_size(ctst),
    ctst_get_total_key_length(ctst)
  );
  
  ctst_free(ctst);  
  ctst_storage_free(storage);
  
  return 0;
}
