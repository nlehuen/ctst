/*
  ctst - (c) 2007 Nicolas Lehuen
  This is released under the LGPL - see http://www.gnu.org/licenses/lgpl.html

  $Id: $

  This file implements a stack structure that is used by visitors and iterators.
*/
#include "include/ctst.h"
#include "include/ctst_stack.h"
#include <string.h>

#define CTST_STACK_INITIAL_LIMIT 16

/* Allocation and deallocation */

ctst_stack* ctst_stack_alloc() {
  ctst_stack* stack = (ctst_stack*)malloc(sizeof(ctst_stack));

  /* Initialize ctst_node_ref stack */
  stack->node_limit = CTST_STACK_INITIAL_LIMIT;
  stack->node_bottom = (ctst_node_ref*)malloc(stack->node_limit*sizeof(ctst_node_ref));
  stack->node_top = stack->node_bottom - 1;

  /* Initialize length stack */
  stack->bytes_length = CTST_STACK_INITIAL_LIMIT*CTST_STACK_INITIAL_LIMIT;
  stack->bytes = (char*)malloc(stack->bytes_length);
  stack->bytes_length_limit = CTST_STACK_INITIAL_LIMIT;
  stack->bytes_length_bottom = (size_t*)malloc(stack->bytes_length_limit*sizeof(size_t));
  stack->bytes_length_top = stack->bytes_length_bottom - 1;

  return stack;
}

void ctst_stack_free(ctst_stack* stack) {
  free(stack->bytes);
  free(stack->bytes_length_bottom);
  free(stack->node_bottom);
}

/* Manipulation functions */

size_t ctst_stack_size(ctst_stack* stack) {
  return stack->bytes_length_top - stack->bytes_length_bottom + 1;
}

void ctst_stack_push(ctst_stack* stack, ctst_node_ref node, char* bytes, size_t bytes_index, size_t bytes_length) {
  size_t bytes_start;
  size_t bytes_end;
  size_t new_top_index;
  
  /* First handle the node reference */
  new_top_index = stack->node_top - stack->node_bottom + 1;
  if(new_top_index == stack->node_limit) {
    /* Grow ctst_node_ref stack */
    stack->node_limit = stack->node_limit + (stack->node_limit>>1);
    stack->node_bottom = (ctst_node_ref*)realloc(stack->node_bottom,stack->node_limit*sizeof(ctst_node_ref));
  }
  stack->node_top = stack->node_bottom + new_top_index;
  *(stack->node_top) = node;

  /* Find the position in the buffer where bytes will be copied */
  if(stack->bytes_length_top >= stack->bytes_length_bottom) {
    bytes_start = *(stack->bytes_length_top);
  }
  else {
    bytes_start = 0;
  }

  /* Determine the future size of the buffer and grow it
     if needed. */
  bytes_end = bytes_start + bytes_length;

  if(bytes_end + 1 > stack->bytes_length) { 
    stack->bytes_length = bytes_end + (stack->bytes_length>>1);
    stack->bytes = (char*)realloc(stack->bytes,stack->bytes_length);
  }

  /* Copy the bytes into the buffer */
  if(bytes_length>0) memcpy(stack->bytes+bytes_start,bytes+bytes_index,bytes_length);

  /* 1 extra byte for the terminating 0. It should not be needed
     as length are always given but this simplifies the use of printf. */
  *(stack->bytes+bytes_end) = '\0';

  /* Push the length of those bytes onto the bytes length stack */
  new_top_index = stack->bytes_length_top - stack->bytes_length_bottom + 1;
  if(new_top_index == stack->bytes_length_limit) {
    /* Grow the bytes length stack if required */
    stack->bytes_length_limit = stack->bytes_length_limit + (stack->bytes_length_limit>>1);
    stack->bytes_length_bottom = (size_t*)realloc(stack->bytes_length_bottom,stack->bytes_length_limit*sizeof(size_t));
  }
  stack->bytes_length_top = stack->bytes_length_bottom + new_top_index;
  *(stack->bytes_length_top) = bytes_end;
}

size_t ctst_stack_peek(ctst_stack* stack, ctst_node_ref* node, char** bytes, size_t* bytes_index, size_t* bytes_length) {
  size_t size = stack->bytes_length_top - stack->bytes_length_bottom + 1;
  if(size>0) {
    *bytes = stack->bytes;

    *node = *(stack->node_top) ;

    if(size>1) {
      *bytes_index = *(stack->bytes_length_top - 1);
      *bytes_length = *(stack->bytes_length_top) - *bytes_index;
    }
    else {
      /* size == 1 => bytes_length_top == bytes_length_bottom */
      *bytes_index = 0;
      *bytes_length = *(stack->bytes_length_top);
    }
  }
  else {
    *node = 0;
    *bytes = 0;
    *bytes_index = 0;
    *bytes_length = 0;            
  }

  return size;
}

size_t ctst_stack_pop(ctst_stack* stack, ctst_node_ref* node, char** bytes, size_t* bytes_index, size_t* bytes_length) {
  size_t size = stack->bytes_length_top - stack->bytes_length_bottom + 1;

  if(size>0) {
    size_t size = stack->node_top - stack->node_bottom + 1;
    if(size>0) {
      *node = *(stack->node_top--);
    }

    *bytes = stack->bytes;

    if(size>1) {
      *bytes_index = *(stack->bytes_length_top - 1);
      *bytes_length = *(stack->bytes_length_top) - *bytes_index;
    }
    else {
      /* size == 1 => bytes_length_top == bytes_length_bottom */
      *bytes_index = 0;
      *bytes_length = *(stack->bytes_length_top);
    }

    --(stack->bytes_length_top);
  }
  else {
    *node = 0;
    *bytes = 0;
    *bytes_index = 0;
    *bytes_length = 0;            
  }

  return size;
}

