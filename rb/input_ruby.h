#ifndef SIT_INPUT_RUBY_H_INCLUDED
#define SIT_INPUT_RUBY_H_INCLUDED

#include "ruby.h"

VALUE 
rbc_input_consume(VALUE self, VALUE rstr);

VALUE
rbc_input_new(VALUE class, VALUE rengine, VALUE rbuf_size, VALUE outbuf);

VALUE
rbc_input_end_stream(VALUE self);

#endif