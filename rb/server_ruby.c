#include "sit_ruby.h"

VALUE
rbc_server_new(VALUE class, VALUE rengine) {  
  Engine *engine;
  Data_Get_Struct(rengine, Engine, engine);
  Server *server = server_new(engine);
	VALUE tdata = Data_Wrap_Struct(class, markall, NULL, server);
	rb_obj_call_init(tdata, 0, NULL);
  return tdata;
}

VALUE 
rbc_server_start(VALUE self, VALUE rport) {
  Server *server;
  Data_Get_Struct(self, Server, server);
  int port = NUM2INT(rport);
  
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	
  server_start(server, &addr);
  return Qnil;
}
