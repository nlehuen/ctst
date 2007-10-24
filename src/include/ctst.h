#ifndef __CTST_CTST_H__
#define __CTST_CTST_H__
/*
  ctst - (c) 2007 Nicolas Lehuen
  This is released under the LGPL - see http://www.gnu.org/licenses/lgpl.html

  $Id: $

  This file contains general definitions
*/

typedef unsigned int size_t;

#define CTST_STORAGE_TYPE_MEMORY_MALLOC

#ifdef CTST_STORAGE_TYPE_MEMORY_MALLOC
  #include "memory_malloc.h"
#endif

#ifdef CTST_STORAGE_TYPE_MEMORY_SMART
  #include "memory_smart.h"
#endif

#include "storage.h"

#endif
