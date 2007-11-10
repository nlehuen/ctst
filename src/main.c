/*
  ctst - (c) 2007 Nicolas Lehuen
  This is released under the LGPL - see http://www.gnu.org/licenses/lgpl.html

  $Id: $

  Main test file.
*/
#include <stdio.h>

#include "include/ctst.h"
#include "include/ctst_stack.h"
#include <assert.h>
#include <string.h>

void load_test(ctst_ctst* ctst) {
  int i=0,l=0,c=110;
  char dest[1024];

  printf("Load test pass 1...");
  for(i=0;i<c;i++) {
    l = sprintf(dest,"%d",i);
    if(ctst_set(ctst,dest,0,l,i)!=0) {
      printf("Erreur 1 !");
      exit(1);
    }
  }
  printf("OK\n");

  printf("Load test pass 2...");
  for(i=0;i<c;i++) {
    l = sprintf(dest,"%d",i);
    if(ctst_set(ctst,dest,0,l,i*2)!=i) {
      printf("KO !\n");
      exit(1);
    }
  }
  printf("OK\n");

  printf("Load test pass 3...");
  for(i=0;i<c;i++) {
    l = sprintf(dest,"%d",i);
    if(ctst_set(ctst,dest,0,l,i)!=i*2) {
      printf("KO !\n");
      exit(1);
    }
  }
  printf("OK\n");
}

ctst_data printer_visitor(void* context, char *key_bytes, size_t key_length, ctst_data data, size_t distance) {
  printf("\"%*s\" : %d (length=%d,distance=%d)\n",key_length,key_bytes,data,key_length,distance);
  return 0;
}


int main(int argc, char** argv) {
  ctst_storage* storage = ctst_storage_alloc();
  ctst_ctst* ctst = ctst_alloc(storage);
  ctst_data data;

  data = ctst_set(ctst,"The answer to your question is",0,30,42);
  printf("%*s %d (was %d)\n",30,"The answer to your question is",ctst_get(ctst,"The answer to your question is",0,30),data);
  
  data = ctst_set(ctst,"Hello, world!",0,13,57);
  printf("%*s %d (was %d)\n",13,"Hello, world!",ctst_get(ctst,"Hello, world!",0,13),data);

  data = ctst_set(ctst,"Hello, world!",0,13,62);
  printf("%*s %d (was %d)\n",13,"Hello, world!",ctst_get(ctst,"Hello, world!",0,13),data);

  data = ctst_set(ctst,"The answer to your question is",0,30,11);
  printf("%*s %d (was %d)\n",30,"The answer to your question is",ctst_get(ctst,"The answer to your question is",0,30),data);

  printf("DUMP OF THE TREE\n");
  ctst_visit_all(ctst, &printer_visitor, 0);
  printf("END DUMP OF THE TREE\n");

  printf("Number of entries: %d\nTotal length of keys: %d\nTotal node count: %d\nMemory usage: %d\nRatio: %f\n",
    ctst_get_size(ctst),
    ctst_get_total_key_length(ctst),
    ctst_get_node_count(ctst),
    ctst_get_memory_usage(ctst),
    ctst_get_ratio(ctst)
  );

  printf("Removing a key\n");
  data = ctst_remove(ctst,"Hello, world!",0,13);
  printf("%*s %d (was %d)\n",13,"Hello, world!",ctst_get(ctst,"Hello, world!",0,13),data);

  printf("Number of entries: %d\nTotal length of keys: %d\nTotal node count: %d\nMemory usage: %d\nRatio: %f\n",
    ctst_get_size(ctst),
    ctst_get_total_key_length(ctst),
    ctst_get_node_count(ctst),
    ctst_get_memory_usage(ctst),
    ctst_get_ratio(ctst)
  );

  load_test(ctst);

  printf("DUMP OF THE TREE\n");
  ctst_visit_all(ctst, &printer_visitor, 0);
  printf("END DUMP OF THE TREE\n");
  
  printf("Number of entries: %d\nTotal length of keys: %d\nTotal node count: %d\nMemory usage: %d\nRatio: %f\n",
    ctst_get_size(ctst),
    ctst_get_total_key_length(ctst),
    ctst_get_node_count(ctst),
    ctst_get_memory_usage(ctst),
    ctst_get_ratio(ctst)
  );

  ctst_free(ctst);  
  ctst_storage_free(storage);
  
  return 0;
}
