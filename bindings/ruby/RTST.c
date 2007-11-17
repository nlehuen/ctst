// Include the Ruby headers and goodies
#include "ruby.h"
#include <stdlib.h>

typedef struct struct_rtst_ctst {
  char* text;
} rtst_ctst;

static VALUE rtst_new(VALUE self) {
  rtst_ctst *ctst;

  VALUE obj = Data_Make_Struct(
    self, // target object
    rtst_ctst, // structure type
    NULL, // mark function
    free, // free function
    ctst  // structure instance
  );

  ctst->text = "Hello, world";

  return obj;
}

static VALUE rtst_to_s(VALUE self) {
  rtst_ctst *ctst;

  Data_Get_Struct(
    self,
    rtst_ctst,
    ctst
  );
  
  return rb_str_new2(ctst->text);
}

// The initialization method for this module
void Init_rtst() {
	VALUE RTST = rb_define_class("RTST",rb_cObject);
	rb_define_singleton_method(RTST, "new", rtst_new, 0);
	rb_define_method(RTST, "to_s", rtst_to_s, 0);
}

