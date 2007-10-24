/*
  ctst - (c) 2007 Nicolas Lehuen
  This is released under the LGPL - see http://www.gnu.org/licenses/lgpl.html

  $Id: $

  Main test file.
*/
#include "stdio.h"

#include "include/ctst.h"

int main(int argc, char** argv) {
  printf("Hello, world !\n");

  ctst_storage* storage = ctst_storage_alloc();
  printf("%d\n",(int)storage);
  ctst_storage_free(storage);
  
  return 0;
}
