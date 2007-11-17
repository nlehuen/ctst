#include "ruby.h"
#include "include/ctst.h"
#include <stdlib.h>

static VALUE rtst_new(VALUE self) {
  ctst_ctst *ctst = ctst_alloc();

  VALUE obj = Data_Wrap_Struct(
    self, /* target object */
    NULL, /* mark function */
    ctst_free, /* free function */
    ctst  /* structure instance */
  );

  return obj;
}

static VALUE rtst_set(VALUE self, VALUE key, VALUE value) {
  ctst_ctst *ctst;

  Data_Get_Struct(
    self,
    ctst_ctst,
    ctst
  );
  
  ctst_data result = ctst_set(ctst, RSTRING(key)->ptr, 0, RSTRING(key)->len, value);
  
  if(result==0) {
    return Qnil;
  }
  else {
    return result;
  }
}

static VALUE rtst_get(VALUE self, VALUE key) {
  ctst_ctst *ctst;

  Data_Get_Struct(
    self,
    ctst_ctst,
    ctst
  );
  
  ctst_data result = ctst_get(ctst, RSTRING(key)->ptr, 0, RSTRING(key)->len);
  if(result==0) {
    return Qnil;
  }
  else {
    return result;
  }
}

/*
 * The initialization method for this module
 */ 
void Init_rtst() {
	VALUE RTST = rb_define_class("RTST",rb_cObject);
	rb_define_singleton_method(RTST, "new", rtst_new, 0);
	rb_define_method(RTST, "get", rtst_get, 1);
	rb_define_method(RTST, "set", rtst_set, 2);
}

