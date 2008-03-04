/*
  ctst - (c) 2007 Nicolas Lehuen
  This is released under the LGPL - see http://www.gnu.org/licenses/lgpl.html

  $Id: $

  Main test file.
*/
#include <stdio.h>

#include "include/ctst.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

void load_test(ctst_ctst* ctst) {
  int i=0,r,l=0,c=100000;
  char dest[1024];

  printf("Load test pass 0...");
  for(i=0;i<c;i++) {
    l = sprintf(dest,"%d",i%317);
    assert(ctst_get(ctst,dest,0,l)==0);
  }
  printf("OK\n");

  printf("Load test pass 1...");
  for(i=0;i<c;i++) {
    l = sprintf(dest,"%d",i%317);
    ctst_set(ctst,dest,0,l,i%317);
    assert(ctst_get(ctst,dest,0,l)==i%317);
  }
  printf(" OK\n");

  printf("Load test pass 2...");
  for(i=0;i<c;i++) {
    l = sprintf(dest,"%d",i%317);
    assert(ctst_get(ctst,dest,0,l)==i%317);
  }
  printf("OK\n");

  printf("Load test pass 3...");
  srand(1);
  for(i=0;i<c;i++) {
    r = rand();
    l = sprintf(dest,"%d",r%1000);
    ctst_set(ctst,dest,0,l,r%1000);
    assert(ctst_get(ctst,dest,0,l)==r%1000);
  }
  printf("OK\n");

  printf("Load test pass 4...");
  for(i=0;i<c;i++) {
    r = rand();
    l = sprintf(dest,"%d",r%1000);
    assert(ctst_get(ctst,dest,0,l)==r%1000);
  }
  printf("OK\n");
}

ctst_data printer_visitor(void* context, char *key_bytes, size_t key_length, ctst_data data, size_t distance) {
  printf("\"%*s\" : %d (length=%d,distance=%d)\n",key_length,key_bytes,data,key_length,distance);
  return 0;
}


int main(int argc, char** argv) {
  ctst_ctst* ctst = ctst_alloc();
  ctst_data data;

  data = ctst_set(ctst,"The answer to your question is",0,30,42);
  printf("%*s %d (was %d)\n",30,"The answer to your question is",ctst_get(ctst,"The answer to your question is",0,30),data);
  
  data = ctst_set(ctst,"Hello, world!",0,13,57);
  printf("%*s %d (was %d)\n",13,"Hello, world!",ctst_get(ctst,"Hello, world!",0,13),data);

  data = ctst_set(ctst,"Hello, world!",0,13,62);
  printf("%*s %d (was %d)\n",13,"Hello, world!",ctst_get(ctst,"Hello, world!",0,13),data);

  data = ctst_set(ctst,"The answer to your question is",0,30,11);
  printf("%*s %d (was %d)\n",30,"The answer to your question is",ctst_get(ctst,"The answer to your question is",0,30),data);

  data = ctst_set(ctst,"There is something rotten in the kingdom of Denmark",0,51,1122);

  printf("DUMP OF THE TREE\n");
  ctst_visit_all(ctst, &printer_visitor, 0);
  printf("END DUMP OF THE TREE\n");

  printf("DUMP OF THE TREE, only The*\n");
  ctst_visit_all_from_key(ctst, &printer_visitor, 0, "The", 0, 3);
  printf("END DUMP OF THE TREE\n");

  printf("DUMP OF THE TREE, only There*\n");
  ctst_visit_all_from_key(ctst, &printer_visitor, 0, "--There", 2, 5);
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
  
  return 0;
}
