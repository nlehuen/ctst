#include "ruby.h"
#include "ctst.h"
#include <stdlib.h>

/* Allocation / garbage collection functions */

static ctst_data rtst_mark_visitor(void* context, char *key_bytes, size_t key_length, ctst_data data, size_t distance) {
  rb_gc_mark((VALUE)data);
  return 0;
}

static void rtst_mark(ctst_ctst* ctst) {
  ctst_visit_all(ctst, rtst_mark_visitor, NULL);
}

static VALUE rtst_new(VALUE self) {
  ctst_ctst *ctst = ctst_alloc();

  VALUE obj = Data_Wrap_Struct(
    self, /* target object */
    rtst_mark, /* mark function */
    ctst_free, /* free function */
    ctst  /* structure instance */
  );

  return obj;
}

/* A few statistics about the ctst */

static VALUE rtst_size(VALUE self) {
  ctst_ctst *ctst;
  Data_Get_Struct(
    self,
    ctst_ctst,
    ctst
  );
  
  return INT2NUM(ctst_get_size(ctst));
}

static VALUE rtst_total_key_length(VALUE self) {
  ctst_ctst *ctst;
  Data_Get_Struct(
    self,
    ctst_ctst,
    ctst
  );
  
  return INT2NUM(ctst_get_total_key_length(ctst));
}

static VALUE rtst_node_count(VALUE self) {
  ctst_ctst *ctst;
  Data_Get_Struct(
    self,
    ctst_ctst,
    ctst
  );
  
  return INT2NUM(ctst_get_node_count(ctst));
}

static VALUE rtst_memory_usage(VALUE self) {
  ctst_ctst *ctst;
  Data_Get_Struct(
    self,
    ctst_ctst,
    ctst
  );
  
  return INT2NUM(ctst_get_memory_usage(ctst));
}

static VALUE rtst_ratio(VALUE self) {
  ctst_ctst *ctst;
  Data_Get_Struct(
    self,
    ctst_ctst,
    ctst
  );
  
  return rb_float_new(ctst_get_ratio(ctst));
}

/* Basic accessors : get, set and remove */

static VALUE rtst_get(VALUE self, VALUE key) {
  ctst_ctst *ctst;
  Data_Get_Struct(
    self,
    ctst_ctst,
    ctst
  );
  
  VALUE result = (VALUE)ctst_get(ctst, RSTRING(key)->ptr, 0, RSTRING(key)->len);
  if(result==0) {
    return Qnil;
  }
  else {
    return result;
  }
}

static VALUE rtst_set(VALUE self, VALUE key, VALUE value) {
  ctst_ctst *ctst;

  Data_Get_Struct(
    self,
    ctst_ctst,
    ctst
  );
  
  VALUE result = (VALUE)ctst_set(ctst, RSTRING(key)->ptr, 0, RSTRING(key)->len, (ctst_data)value);

  if(result==0) {
    return Qnil;
  }
  else {
    return result;
  }
}

static VALUE rtst_remove(VALUE self, VALUE key) {
  ctst_ctst *ctst;
  Data_Get_Struct(
    self,
    ctst_ctst,
    ctst
  );
  
  VALUE result = (VALUE)ctst_remove(ctst, RSTRING(key)->ptr, 0, RSTRING(key)->len);
  
  if(result==0) {
    return Qnil;
  }
  else {
    return result;
  }
}

/* Visitor pattern : we'll map this pattern to an iterator pattern, the caller
   passing a block. */
static ctst_data rtst_visitor(void* context, char *key_bytes, size_t key_length, ctst_data data, size_t distance) {
  if(rb_block_given_p()) {
    VALUE key = rb_str_new(key_bytes,key_length);
    VALUE params = rb_ary_new3(3,key,(VALUE)data,INT2NUM(distance));
    VALUE result = rb_yield(params);
    if(result==Qnil) { 
      return 0;
    }
    else {
      return (ctst_data)result;
    }
  }
  else {
    rb_warn("No block given to RTST.each");
    return Qfalse;
  }
}

static VALUE rtst_each(VALUE self) {
  ctst_ctst *ctst;
  Data_Get_Struct(
    self,
    ctst_ctst,
    ctst
  );

  VALUE result = (VALUE)ctst_visit_all(ctst, rtst_visitor, 0);
  if(result == 0) {
    return Qnil;
  } else {
    return result;
  }
}


/*
 * The initialization method for this module
 */ 
void Init_rtst() {
	VALUE RTST = rb_define_class("RTST",rb_cObject);

  /* Allocation / garbage collection functions */
	rb_define_singleton_method(RTST, "new", rtst_new, 0);

  /* A few statistics about the ctst */
	rb_define_method(RTST, "size", rtst_size, 0);
	rb_define_method(RTST, "total_key_length", rtst_total_key_length, 0);
	rb_define_method(RTST, "node_count", rtst_node_count, 0);
	rb_define_method(RTST, "memory_usage", rtst_memory_usage, 0);
	rb_define_method(RTST, "ratio", rtst_ratio, 0);

  /* Basic accessors : get, set and remove */
	rb_define_method(RTST, "get", rtst_get, 1);
	rb_define_method(RTST, "set", rtst_set, 2);
	rb_define_method(RTST, "remove", rtst_remove, 1);
	
	/* Visitor pattern */
	rb_define_method(RTST, "each", rtst_each, 0);
}

