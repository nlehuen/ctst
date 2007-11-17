// Include the Ruby headers and goodies
#include "ruby.h"
#include <stdlib.h>

static VALUE method_initialize(VALUE self) {
  return self;
}

// The initialization method for this module
void Init_rtst() {
	VALUE RTST = rb_define_class("RTST",rb_cObject);
	rb_define_method(RTST, "initialize", method_initialize, 0);
}

