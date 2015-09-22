#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/errno.h>


#include <mruby.h>
#include <mruby/array.h>
#include <mruby/irep.h>
#include <mruby/compile.h>
#include <mruby/string.h>
#include <mruby/error.h>


#include "detective.h"
#include "investigation.h"
#include "hterm.h"

# define TEMP_FAILURE_RETRY(expression) \
  (__extension__\
    ({ long int __result;\
       do __result = (long int) (expression); \
       while (__result == -1L && errno == EINTR); \
       __result; }))


static void if_exception_error_and_exit(mrb_state* mrb, char *msg) {
  // check for exception, only one can exist at any point in time
  if (mrb->exc) {
    fprintf(stderr, "%s", msg);
    mrb_print_error(mrb);
    exit(2);
  }
}


static void eval_static_libs(mrb_state* mrb, ...) {
  va_list argp;
  va_start(argp, mrb);

  int end_of_static_libs = 0;
  uint8_t const *p;

  while(!end_of_static_libs) {
    p = va_arg(argp, uint8_t const*);
    if (NULL == p) {
      end_of_static_libs = 1;
    } else {
      mrb_load_irep(mrb, p);
      if_exception_error_and_exit(mrb, "Exception in bundled ruby\n");
    }
  }

  va_end(argp);
}


static mrb_value business(mrb_state* mrb, mrb_value obj)
{
  mrb_value ret;
  mrb_value block;
  mrb_get_args(mrb, "&", &block);

  ret = mrb_yield_argv(mrb, block, 0, NULL);

  if_exception_error_and_exit(mrb, "Exception in business yield\n");

  char *tasks = mrb_str_to_cstr(mrb, ret);
  fprintf(stdout, "%s", tasks);

  return mrb_nil_value();
}


static void feed_http(mrb_state *mrb, int SocketFD, char *request_token) {
  if (-1 == listen(SocketFD, 10)) {
    perror("listen failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  int ConnectFD = accept(SocketFD, NULL, NULL);

  if (0 > ConnectFD) {
    perror("accept failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  char c[1];
  mrb_value ret;
  mrb_value in;

  fd_set set;
  struct timeval timeout;
  /* Initialize the file descriptor set. */
  FD_ZERO (&set);
  FD_SET (ConnectFD, &set);

  /* Initialize the timeout data structure. */
  timeout.tv_sec = 30;
  timeout.tv_usec = 0;

  /* select returns 0 if timeout, 1 if input available, -1 if error. */
  int rs = 1;

  size_t bytes_read = 0;
  int nn = 0;

  while(rs == 1) {
    rs = TEMP_FAILURE_RETRY(select(FD_SETSIZE,
                                   &set, NULL, NULL,
                                   &timeout));

    bytes_read = recv(ConnectFD, c, 1, 0);
    if (bytes_read == -1) {
      perror("reading from client");
    }

    in = mrb_str_new_cstr(mrb, c);
    ret = mrb_funcall(mrb, mrb_obj_value(mrb->object_class), "stack_byte", 1, in);
    if_exception_error_and_exit(mrb, "stack_byte");

    nn = mrb_bool(ret);

    if (nn) {
      break;
    }
  }

  in = mrb_str_new_cstr(mrb, request_token);
  ret = mrb_funcall(mrb, mrb_obj_value(mrb->object_class), "response_for_token", 1, in);
  if_exception_error_and_exit(mrb, "response_for_token");

  char *http_response = mrb_str_to_cstr(mrb, ret);
  int going = strlen(http_response);
  int sent = send(ConnectFD, http_response, going, 0);

  printf("sent: %d %d\n", going, sent);

  if (-1 == shutdown(ConnectFD, SHUT_RDWR)) {
    perror("shutdown failed");
    close(ConnectFD);
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  close(ConnectFD);
}


int main(int argc, char** argv) {
  FILE *f = 0;
  f = fopen("Detectivefile", "r");
  if (0 == f) {
    return 1;
  }

  mrb_state *mrb;
  mrb_value ret;

  // initialize mruby
  if (!(mrb = mrb_open())) {
    fprintf(stderr,"%s: could not initialize mruby\n",argv[0]);
    return -1;
  }

  mrb_value args = mrb_ary_new(mrb);
  int i;

  // convert argv into mruby strings
  for (i=1; i<argc; i++) {
     mrb_ary_push(mrb, args, mrb_str_new_cstr(mrb,argv[i]));
  }

  mrb_define_global_const(mrb, "ARGV", args);

  mrb_define_method(mrb, mrb->object_class, "business", business, MRB_ARGS_BLOCK());

  eval_static_libs(mrb, detective, investigation, hterm, NULL);

  mrbc_context *detective_file = mrbc_context_new(mrb);
  mrbc_filename(mrb, detective_file, "Detectivefile");
  ret = mrb_load_file_cxt(mrb, f, detective_file);
  mrbc_context_free(mrb, detective_file);
  // cleanup our opened Detectivefile
  fclose(f);
  if_exception_error_and_exit(mrb, "Exception in Detectivefile\n");


  /*
  ///////////////////////
  ret = mrb_funcall(mrb, mrb_obj_value(mrb->object_class), "index", 0);
  if_exception_error_and_exit(mrb, "index.html");
  char *index = mrb_str_to_cstr(mrb, ret);

  ///////////////////////
  ret = mrb_funcall(mrb, mrb_obj_value(mrb->object_class), "hterm", 0);
  if_exception_error_and_exit(mrb, "index.html");
  char *hterm2 = mrb_str_to_cstr(mrb, ret);
  ///////////////////////
  */

  // network shell
  struct sockaddr_in sa;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  int iSetOption = 1;
  setsockopt(SocketFD, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));

  if (-1 == SocketFD) {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  memset(&sa, 0, sizeof sa);

  sa.sin_family = AF_INET;
  sa.sin_port = htons(11000);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);

  if (-1 == bind(SocketFD,(struct sockaddr *)&sa, sizeof sa)) {
    perror("bind failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  feed_http(mrb, SocketFD, "");
  feed_http(mrb, SocketFD, "hterm");

  // cleanup mruby
  mrb_close(mrb);

  return 0;
}
